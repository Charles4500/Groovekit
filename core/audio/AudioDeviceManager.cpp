#include "AudioDeviceManager.h"

#include <RtAudio.h>

namespace dj::core::audio {

std::vector<AudioDeviceInfo> AudioDeviceManager::listOutputDevices() const {
    RtAudio rtAudio;
    std::vector<AudioDeviceInfo> devices;

    for (unsigned int id : rtAudio.getDeviceIds()) {
        RtAudio::DeviceInfo info = rtAudio.getDeviceInfo(id);
        if (info.outputChannels == 0) {
            continue;  // input-only device, not relevant for playback
        }

        AudioDeviceInfo device;
        device.id = info.ID;
        device.name = info.name;
        device.outputChannels = info.outputChannels;
        device.inputChannels = info.inputChannels;
        device.sampleRates = info.sampleRates;
        device.preferredSampleRate = info.preferredSampleRate;
        device.isDefaultOutput = info.isDefaultOutput;
        devices.push_back(std::move(device));
    }

    return devices;
}

}  // namespace dj::core::audio
