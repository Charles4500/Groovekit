#include "AudioEngine.h"

#include <cstring>

namespace dj::core::audio {

void AudioEngine::open() {
    if (rtAudio_.getDeviceIds().empty()) {
        throw AudioEngineError("no audio devices available");
    }

    RtAudio::StreamParameters outputParams;
    outputParams.deviceId = rtAudio_.getDefaultOutputDevice();
    outputParams.nChannels = kChannels;

    unsigned int bufferFrames = 256;
    RtAudioErrorType err = rtAudio_.openStream(
        &outputParams, nullptr, RTAUDIO_FLOAT32, kSampleRate, &bufferFrames,
        &AudioEngine::audioCallback, this);

    if (err != RTAUDIO_NO_ERROR) {
        throw AudioEngineError("failed to open audio stream: " + rtAudio_.getErrorText());
    }
    streamOpen_ = true;
}

void AudioEngine::start() {
    if (!streamOpen_) {
        throw AudioEngineError("start() called before open()");
    }
    if (rtAudio_.startStream() != RTAUDIO_NO_ERROR) {
        throw AudioEngineError("failed to start audio stream: " + rtAudio_.getErrorText());
    }
}

void AudioEngine::stop() {
    if (streamOpen_ && rtAudio_.isStreamRunning()) {
        rtAudio_.stopStream();
    }
}

AudioEngine::~AudioEngine() {
    stop();
    if (streamOpen_) {
        rtAudio_.closeStream();
    }
}

int AudioEngine::audioCallback(void* outputBuffer, void* /*inputBuffer*/,
                                unsigned int nFrames, double /*streamTime*/,
                                RtAudioStreamStatus /*status*/, void* userData) {
    auto* engine = static_cast<AudioEngine*>(userData);
    auto* out = static_cast<float*>(outputBuffer);

    std::memset(out, 0, sizeof(float) * nFrames * kChannels);

    for (auto& deck : engine->decks_) {
        deck.renderInto(out, nFrames, kChannels);
    }

    return 0;
}

}  // namespace dj::core::audio
