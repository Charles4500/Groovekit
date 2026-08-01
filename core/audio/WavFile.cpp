#include "WavFile.h"

#include <cstring>
#include <fstream>

namespace dj::core::audio {

namespace {

constexpr uint16_t kFormatPcm = 1;
constexpr uint16_t kFormatIeeeFloat = 3;

uint32_t readU32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint16_t readU16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

}  // namespace

WavFile WavFile::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw WavFileError("could not open file: " + path);
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());

    if (data.size() < 12 || std::memcmp(data.data(), "RIFF", 4) != 0 ||
        std::memcmp(data.data() + 8, "WAVE", 4) != 0) {
        throw WavFileError("not a RIFF/WAVE file: " + path);
    }

    WavFile wav;
    uint16_t formatTag = 0;
    uint16_t bitsPerSample = 0;
    bool haveFmt = false;
    const uint8_t* dataChunk = nullptr;
    uint32_t dataChunkSize = 0;

    size_t offset = 12;
    while (offset + 8 <= data.size()) {
        char chunkId[5] = {0};
        std::memcpy(chunkId, data.data() + offset, 4);
        uint32_t chunkSize = readU32(data.data() + offset + 4);
        size_t chunkDataStart = offset + 8;

        if (chunkDataStart + chunkSize > data.size()) {
            throw WavFileError("truncated chunk in WAV file: " + path);
        }

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) {
                throw WavFileError("fmt chunk too small: " + path);
            }
            const uint8_t* fmt = data.data() + chunkDataStart;
            formatTag = readU16(fmt);
            wav.channels_ = readU16(fmt + 2);
            wav.sampleRate_ = readU32(fmt + 4);
            bitsPerSample = readU16(fmt + 14);
            haveFmt = true;
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            dataChunk = data.data() + chunkDataStart;
            dataChunkSize = chunkSize;
        }

        // Chunks are word-aligned; skip a padding byte if size is odd.
        offset = chunkDataStart + chunkSize + (chunkSize % 2);
    }

    if (!haveFmt) {
        throw WavFileError("missing fmt chunk: " + path);
    }
    if (dataChunk == nullptr) {
        throw WavFileError("missing data chunk: " + path);
    }
    if (wav.channels_ == 0) {
        throw WavFileError("invalid channel count (0): " + path);
    }

    if (formatTag == kFormatPcm && bitsPerSample == 16) {
        size_t sampleCount = dataChunkSize / 2;
        wav.samples_.resize(sampleCount);
        for (size_t i = 0; i < sampleCount; ++i) {
            int16_t raw = static_cast<int16_t>(readU16(dataChunk + i * 2));
            wav.samples_[i] = static_cast<float>(raw) / 32768.0f;
        }
    } else if (formatTag == kFormatIeeeFloat && bitsPerSample == 32) {
        size_t sampleCount = dataChunkSize / 4;
        wav.samples_.resize(sampleCount);
        for (size_t i = 0; i < sampleCount; ++i) {
            uint32_t bits = readU32(dataChunk + i * 4);
            float sample;
            std::memcpy(&sample, &bits, sizeof(sample));
            wav.samples_[i] = sample;
        }
    } else {
        throw WavFileError("unsupported WAV format (tag=" + std::to_string(formatTag) +
                            ", bits=" + std::to_string(bitsPerSample) + "): " + path);
    }

    return wav;
}

}  // namespace dj::core::audio
