#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "AudioSource.h"

namespace dj::core::audio {

class Mp3FileError : public std::runtime_error {
public:
    explicit Mp3FileError(const std::string& message) : std::runtime_error(message) {}
};

// Decodes an MP3 file fully into memory via dr_mp3 (see ADR-002), exposed
// as interleaved float samples in [-1, 1]. Throws Mp3FileError on
// malformed/unreadable files.
class Mp3File : public AudioSource {
public:
    static Mp3File loadFromFile(const std::string& path);

    unsigned int sampleRate() const override { return sampleRate_; }
    unsigned int channels() const override { return channels_; }
    const std::vector<float>& samples() const override { return samples_; }

private:
    unsigned int sampleRate_ = 0;
    unsigned int channels_ = 0;
    std::vector<float> samples_;
};

}  // namespace dj::core::audio
