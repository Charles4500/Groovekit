#include "Deck.h"

#include <algorithm>

namespace dj::core::audio {

void Deck::load(const std::string& path) {
    // Not real-time safe: replacing track_ while the audio thread reads it
    // in renderInto() is only safe here because callers load before calling
    // play(). Swapping tracks on a live/playing deck needs a proper
    // lock-free handoff and is out of scope for this slice.
    auto loaded = std::make_shared<WavFile>(WavFile::loadFromFile(path));
    positionFrames_.store(0, std::memory_order_relaxed);
    track_ = std::move(loaded);
}

void Deck::play() {
    playing_.store(true, std::memory_order_relaxed);
}

unsigned int Deck::sampleRate() const {
    return track_ ? track_->sampleRate() : 0;
}

unsigned int Deck::channels() const {
    return track_ ? track_->channels() : 0;
}

void Deck::renderInto(float* out, size_t frameCount, unsigned int outChannels) {
    if (!playing_.load(std::memory_order_relaxed) || !track_) {
        return;
    }

    const auto& samples = track_->samples();
    const unsigned int trackChannels = track_->channels();
    const size_t totalFrames = track_->frameCount();
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
                    samples[pos * trackChannels + srcCh];
            }
        }
        ++pos;
    }

    positionFrames_.store(pos, std::memory_order_relaxed);
}

}  // namespace dj::core::audio
