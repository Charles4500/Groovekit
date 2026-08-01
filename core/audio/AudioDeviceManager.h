#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dj::core::audio {

struct AudioDeviceInfo {
    unsigned int id = 0;
    std::string name;
    unsigned int outputChannels = 0;
    unsigned int inputChannels = 0;
    std::vector<unsigned int> sampleRates;
    unsigned int preferredSampleRate = 0;
    bool isDefaultOutput = false;
};

// Wraps RtAudio device enumeration behind our own interface so the rest of
// the engine never depends on the RtAudio API directly.
class AudioDeviceManager {
public:
    // Queries the system for currently available devices. Devices can be
    // (dis)connected between calls, so callers should re-query rather than
    // cache results long-term.
    std::vector<AudioDeviceInfo> listOutputDevices() const;
};

}  // namespace dj::core::audio
