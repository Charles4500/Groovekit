#include "Deck.h"

#include <algorithm>
#include <stdexcept>

#include "Mp3File.h"
#include "WavFile.h"

namespace dj::core::audio {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool hasExtension(const std::string& path, const std::string& ext) {
    if (path.size() < ext.size()) {
        return false;
    }
    return toLower(path.substr(path.size() - ext.size())) == ext;
}

}  // namespace

void Deck::load(const std::string& path) {
    // Not real-time safe: replacing track_ while the audio thread reads it
    // in renderInto() is only safe here because callers load before calling
    // play(). Swapping tracks on a live/playing deck needs a proper
    // lock-free handoff and is out of scope for this slice.
    std::shared_ptr<const AudioSource> loaded;
    if (hasExtension(path, ".wav")) {
        loaded = std::make_shared<WavFile>(WavFile::loadFromFile(path));
    } else if (hasExtension(path, ".mp3")) {
        loaded = std::make_shared<Mp3File>(Mp3File::loadFromFile(path));
    } else {
        throw std::runtime_error("unrecognized audio file extension: " + path);
    }

    positionFrames_.store(0, std::memory_order_relaxed);
    track_ = std::move(loaded);
}

void Deck::play() {
    playing_.store(true, std::memory_order_relaxed);
}

void Deck::pause() {
    playing_.store(false, std::memory_order_relaxed);
}

void Deck::seek(double seconds) {
    if (!track_) {
        return;
    }
    seconds = std::max(seconds, 0.0);
    long long targetFrame =
        static_cast<long long>(seconds * static_cast<double>(track_->sampleRate()));
    long long maxFrame = static_cast<long long>(track_->frameCount());
    targetFrame = std::clamp(targetFrame, 0LL, maxFrame);
    pendingSeekFrames_.store(targetFrame, std::memory_order_relaxed);
}

void Deck::setVolume(float volume) {
    volume_.store(std::clamp(volume, 0.0f, 1.0f), std::memory_order_relaxed);
}

double Deck::positionSeconds() const {
    if (!track_ || track_->sampleRate() == 0) {
        return 0.0;
    }
    return static_cast<double>(positionFrames_.load(std::memory_order_relaxed)) /
           static_cast<double>(track_->sampleRate());
}

unsigned int Deck::sampleRate() const {
    return track_ ? track_->sampleRate() : 0;
}

unsigned int Deck::channels() const {
    return track_ ? track_->channels() : 0;
}

void Deck::renderInto(float* out, size_t frameCount, unsigned int outChannels) {
    if (!track_) {
        return;
    }

    // Apply any pending seek at the block boundary, regardless of play
    // state, so seeking a paused deck is reflected as soon as the next
    // callback runs rather than only once playback resumes.
    long long pendingSeek = pendingSeekFrames_.exchange(-1, std::memory_order_relaxed);
    if (pendingSeek >= 0) {
        positionFrames_.store(static_cast<size_t>(pendingSeek), std::memory_order_relaxed);
    }

    if (!playing_.load(std::memory_order_relaxed)) {
        return;
    }

    const auto& samples = track_->samples();
    const unsigned int trackChannels = track_->channels();
    const size_t totalFrames = track_->frameCount();
    const float gain = volume_.load(std::memory_order_relaxed);
    size_t pos = positionFrames_.load(std::memory_order_relaxed);

    for (size_t frame = 0; frame < frameCount; ++frame) {
        if (pos >= totalFrames) {
            playing_.store(false, std::memory_order_relaxed);
            break;
        }
        for (unsigned int ch = 0; ch < outChannels; ++ch) {
            // Mono tracks fan out to all output channels; for multi-channel
            // tracks with a differing channel count, extra output channels
            // get silence (no downmix logic yet — deferred).
            unsigned int srcCh = (trackChannels == 1) ? 0 : ch;
            if (srcCh < trackChannels) {
                out[frame * outChannels + ch] +=
                    samples[pos * trackChannels + srcCh] * gain;
            }
        }
        ++pos;
    }

    positionFrames_.store(pos, std::memory_order_relaxed);
}

}  // namespace dj::core::audio
