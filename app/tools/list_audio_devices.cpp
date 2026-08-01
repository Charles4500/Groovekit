#include <iostream>

#include "AudioDeviceManager.h"

// Manual diagnostic tool for Milestone 1 slice 1: verifies RtAudio device
// enumeration is wired up correctly. Not part of the shipped application.
int main() {
    dj::core::audio::AudioDeviceManager manager;
    auto devices = manager.listOutputDevices();

    if (devices.empty()) {
        std::cout << "No output devices found.\n";
        return 0;
    }

    for (const auto& device : devices) {
        std::cout << "[" << device.id << "] " << device.name
                  << (device.isDefaultOutput ? " (default)" : "") << "\n"
                  << "    output channels: " << device.outputChannels << "\n"
                  << "    preferred sample rate: " << device.preferredSampleRate << " Hz\n"
                  << "    supported sample rates:";
        for (unsigned int rate : device.sampleRates) {
            std::cout << " " << rate;
        }
        std::cout << "\n";
    }

    return 0;
}
