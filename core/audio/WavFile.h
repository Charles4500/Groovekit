#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "AudioSource.h"

namespace dj::core::audio {

class WavFileError : public std::runtime_error {
public:
    explicit WavFileError(const std::string& message) : std::runtime_error(message) {}
};

// Parses a PCM WAV file (16-bit integer or 32-bit IEEE float) fully into
// memory as interleaved float samples in [-1, 1]. Throws WavFileError on
// malformed or unsupported files.
class WavFile : public AudioSource {
public:
    static WavFile loadFromFile(const std::string& path);

    unsigned int sampleRate() const override { return sampleRate_; }
    unsigned int channels() const override { return channels_; }
    const std::vector<float>& samples() const override { return samples_; }  // interleaved

private:
    unsigned int sampleRate_ = 0;
    unsigned int channels_ = 0;
    std::vector<float> samples_;
};

}  // namespace dj::core::audio
