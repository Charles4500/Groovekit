#include <chrono>
#include <iostream>
#include <thread>

#include "AudioEngine.h"

// Manual diagnostic tool for Milestone 1 slice 2: loads up to two WAV files
// onto two decks and plays them. Usage:
//   play_wav <deck0.wav> [deck1.wav]
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: play_wav <deck0.wav> [deck1.wav]\n";
        return 1;
    }

    dj::core::audio::AudioEngine engine;

    try {
        engine.deck(0).load(argv[1]);
        if (engine.deck(0).sampleRate() != dj::core::audio::AudioEngine::kSampleRate) {
            std::cerr << "deck0: file sample rate " << engine.deck(0).sampleRate()
                      << " != engine rate " << dj::core::audio::AudioEngine::kSampleRate
                      << " (no resampling yet — see Milestone 1 slice 2 risks)\n";
            return 1;
        }

        bool haveDeck1 = argc >= 3;
        if (haveDeck1) {
            engine.deck(1).load(argv[2]);
            if (engine.deck(1).sampleRate() != dj::core::audio::AudioEngine::kSampleRate) {
                std::cerr << "deck1: file sample rate " << engine.deck(1).sampleRate()
                          << " != engine rate " << dj::core::audio::AudioEngine::kSampleRate
                          << " (no resampling yet)\n";
                return 1;
            }
        }

        engine.open();
        engine.start();
        engine.deck(0).play();
        if (haveDeck1) {
            engine.deck(1).play();
        }

        while (engine.deck(0).isPlaying() || (haveDeck1 && engine.deck(1).isPlaying())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        engine.stop();
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
