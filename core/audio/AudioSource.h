#pragma once

#include <cstddef>
#include <vector>

namespace dj::core::audio {

// Common interface for a fully-decoded, in-memory audio source (WAV, MP3,
// ...). Deck only ever talks to this interface, never a concrete format.
class AudioSource {
public:
    virtual ~AudioSource() = default;

    virtual unsigned int sampleRate() const = 0;
    virtual unsigned int channels() const = 0;
    virtual const std::vector<float>& samples() const = 0;  // interleaved

    size_t frameCount() const {
        unsigned int ch = channels();
        return ch == 0 ? 0 : samples().size() / ch;
    }
};

}  // namespace dj::core::audio
