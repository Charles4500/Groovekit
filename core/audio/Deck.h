#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "WavFile.h"

namespace dj::core::audio {

// A single playback unit. load() does file I/O and must never be called from
// the audio thread. play()/renderInto() are real-time safe: no allocation,
// no locking, no blocking (rule 5).
class Deck {
public:
    // Loads a file from disk. Not real-time safe — call from a non-audio
    // thread only.
    void load(const std::string& path);

    // Real-time safe. Starts playback from the current position.
    void play();

    bool isPlaying() const { return playing_.load(std::memory_order_relaxed); }
    unsigned int sampleRate() const;
    unsigned int channels() const;

    // Real-time safe. Adds this deck's next `frameCount` frames into `out`
    // (interleaved, `outChannels` channels), advancing position. Does not
    // clear `out` first — caller is responsible for zeroing the buffer.
    // Stops automatically (sets isPlaying() to false) at end of track.
    void renderInto(float* out, size_t frameCount, unsigned int outChannels);

private:
    std::shared_ptr<const WavFile> track_;
    std::atomic<size_t> positionFrames_{0};
    std::atomic<bool> playing_{false};
};

}  // namespace dj::core::audio
