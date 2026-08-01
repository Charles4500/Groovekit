// Plain-assert unit test for Deck (no test framework dependency yet, per
// dependency discipline rule 16). Writes a small synthetic WAV to a temp
// file, then exercises Deck's play/pause/seek/volume/end-of-track behavior
// via renderInto() into an in-memory buffer — no audio hardware needed.

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "Deck.h"

using dj::core::audio::Deck;

namespace {

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void writeU32(std::ofstream& f, uint32_t v) {
    uint8_t b[4] = {static_cast<uint8_t>(v), static_cast<uint8_t>(v >> 8),
                    static_cast<uint8_t>(v >> 16), static_cast<uint8_t>(v >> 24)};
    f.write(reinterpret_cast<char*>(b), 4);
}

void writeU16(std::ofstream& f, uint16_t v) {
    uint8_t b[2] = {static_cast<uint8_t>(v), static_cast<uint8_t>(v >> 8)};
    f.write(reinterpret_cast<char*>(b), 2);
}

// Writes a mono, 16-bit PCM WAV with `frameCount` frames at `sampleRate`,
// where sample i (as int16) == i * 100, so decoded float samples are
// exactly predictable: (i * 100) / 32768.0f.
std::string writeSyntheticWav(unsigned int sampleRate, unsigned int frameCount) {
    std::string path =
        (std::filesystem::temp_directory_path() / "dj_deck_test.wav").string();
    std::ofstream f(path, std::ios::binary);

    uint32_t dataSize = frameCount * 2;  // 16-bit mono
    uint32_t riffSize = 36 + dataSize;

    f.write("RIFF", 4);
    writeU32(f, riffSize);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    writeU32(f, 16);
    writeU16(f, 1);              // PCM
    writeU16(f, 1);              // mono
    writeU32(f, sampleRate);
    writeU32(f, sampleRate * 2);  // byte rate
    writeU16(f, 2);              // block align
    writeU16(f, 16);             // bits per sample
    f.write("data", 4);
    writeU32(f, dataSize);

    for (unsigned int i = 0; i < frameCount; ++i) {
        writeU16(f, static_cast<uint16_t>(static_cast<int16_t>(i * 100)));
    }

    return path;
}

float expectedSample(unsigned int i) {
    return static_cast<float>(static_cast<int16_t>(i * 100)) / 32768.0f;
}

bool nearlyEqual(float a, float b) { return std::fabs(a - b) < 1e-6f; }

}  // namespace

int main() {
    constexpr unsigned int kSampleRate = 1000;  // simplifies seconds<->frames math
    constexpr unsigned int kFrameCount = 100;
    std::string path = writeSyntheticWav(kSampleRate, kFrameCount);

    Deck deck;
    deck.load(path);
    check(deck.sampleRate() == kSampleRate, "sampleRate reflects loaded file");
    check(deck.channels() == 1, "channels reflects loaded file");
    check(!deck.isPlaying(), "deck is not playing immediately after load");
    check(nearlyEqual(static_cast<float>(deck.positionSeconds()), 0.0f),
          "position starts at 0");

    // Not playing: renderInto should not advance position or write samples.
    {
        std::vector<float> buf(10, 0.0f);
        deck.renderInto(buf.data(), 10, 1);
        check(nearlyEqual(static_cast<float>(deck.positionSeconds()), 0.0f),
              "renderInto while not playing does not advance position");
        for (float s : buf) {
            check(s == 0.0f, "renderInto while not playing writes silence");
        }
    }

    // Play: first 10 frames should match samples[0..9].
    deck.play();
    {
        std::vector<float> buf(10, 0.0f);
        deck.renderInto(buf.data(), 10, 1);
        for (unsigned int i = 0; i < 10; ++i) {
            check(nearlyEqual(buf[i], expectedSample(i)),
                  "playing frame " + std::to_string(i) + " matches source sample");
        }
        check(nearlyEqual(static_cast<float>(deck.positionSeconds()), 0.01f),
              "position advanced by 10 frames at 1000Hz = 0.01s");
    }

    // Pause: further renderInto calls must not advance position or write samples.
    deck.pause();
    {
        std::vector<float> buf(10, 0.0f);
        deck.renderInto(buf.data(), 10, 1);
        check(nearlyEqual(static_cast<float>(deck.positionSeconds()), 0.01f),
              "pause halts position advancement");
        for (float s : buf) {
            check(s == 0.0f, "paused deck writes silence");
        }
    }

    // Seek while paused: takes effect on next renderInto even without play().
    deck.seek(0.05);  // frame 50
    {
        std::vector<float> buf(5, 0.0f);
        deck.renderInto(buf.data(), 5, 1);
        check(nearlyEqual(static_cast<float>(deck.positionSeconds()), 0.05f),
              "seek applied at next renderInto call even while paused");
        for (float s : buf) {
            check(s == 0.0f, "still paused: no samples written despite seek");
        }
    }

    // Resume: next frames should match samples[50..54].
    deck.play();
    {
        std::vector<float> buf(5, 0.0f);
        deck.renderInto(buf.data(), 5, 1);
        for (unsigned int i = 0; i < 5; ++i) {
            check(nearlyEqual(buf[i], expectedSample(50 + i)),
                  "post-seek playback resumes at correct sample index");
        }
    }

    // Volume: scales subsequent samples.
    deck.setVolume(0.5f);
    check(nearlyEqual(deck.volume(), 0.5f), "volume getter reflects setVolume");
    {
        std::vector<float> buf(5, 0.0f);
        deck.renderInto(buf.data(), 5, 1);
        for (unsigned int i = 0; i < 5; ++i) {
            check(nearlyEqual(buf[i], expectedSample(55 + i) * 0.5f),
                  "volume scales sample amplitude");
        }
    }

    // End of track: seek near the end, render past it, expect isPlaying() to
    // clear and only in-range frames to be written.
    deck.setVolume(1.0f);
    deck.seek(0.098);  // frame 98, 2 frames remain
    {
        std::vector<float> buf(10, 0.0f);
        deck.renderInto(buf.data(), 10, 1);
        check(!deck.isPlaying(), "isPlaying() clears at end of track");
        check(nearlyEqual(buf[0], expectedSample(98)), "last-but-one sample rendered");
        check(nearlyEqual(buf[1], expectedSample(99)), "last sample rendered");
        for (unsigned int i = 2; i < 10; ++i) {
            check(buf[i] == 0.0f, "frames past end of track are left untouched/silent");
        }
    }

    std::filesystem::remove(path);

    if (failures == 0) {
        std::cout << "deck_test: all checks passed\n";
        return 0;
    }
    std::cerr << "deck_test: " << failures << " check(s) failed\n";
    return 1;
}
