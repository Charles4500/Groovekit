#include "Mp3File.h"

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

namespace dj::core::audio {

Mp3File Mp3File::loadFromFile(const std::string& path) {
    drmp3_config config;
    drmp3_uint64 totalFrameCount = 0;

    float* decoded = drmp3_open_file_and_read_pcm_frames_f32(
        path.c_str(), &config, &totalFrameCount, nullptr);

    if (decoded == nullptr) {
        throw Mp3FileError("failed to decode MP3 file: " + path);
    }

    Mp3File mp3;
    mp3.sampleRate_ = config.sampleRate;
    mp3.channels_ = config.channels;
    mp3.samples_.assign(decoded, decoded + totalFrameCount * config.channels);

    drmp3_free(decoded, nullptr);

    if (mp3.channels_ == 0 || mp3.sampleRate_ == 0) {
        throw Mp3FileError("decoded MP3 has invalid format (channels/sampleRate = 0): " + path);
    }

    return mp3;
}

}  // namespace dj::core::audio
