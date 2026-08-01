#pragma once

#include <RtAudio.h>

#include <array>
#include <stdexcept>

#include "Deck.h"

namespace dj::core::audio {

class AudioEngineError : public std::runtime_error {
public:
    explicit AudioEngineError(const std::string& message) : std::runtime_error(message) {}
};

// Owns the RtAudio output stream and two decks, mixed by direct summation.
// Operates at a fixed sample rate/channel count (see ADR-001: 44.1kHz
// internal); loaded tracks must match or open() will fail. Resampling and
// per-deck gain/clipping protection are deferred (see Milestone 1 slice 2
// design doc).
class AudioEngine {
public:
    static constexpr unsigned int kSampleRate = 44100;
    static constexpr unsigned int kChannels = 2;
    static constexpr unsigned int kDeckCount = 2;

    // Opens the default output device's stream. Not real-time safe — call
    // during setup only.
    void open();
    void start();
    void stop();
    ~AudioEngine();

    Deck& deck(unsigned int index) { return decks_.at(index); }

private:
    static int audioCallback(void* outputBuffer, void* inputBuffer,
                              unsigned int nFrames, double streamTime,
                              RtAudioStreamStatus status, void* userData);

    RtAudio rtAudio_;
    std::array<Deck, kDeckCount> decks_;
    bool streamOpen_ = false;
};

}  // namespace dj::core::audio
