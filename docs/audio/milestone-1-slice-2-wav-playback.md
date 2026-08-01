# Milestone 1 — Slice 2: WAV Load + Play (Two Decks)

Per rule 43 (before coding) and rule 46 (small iterations). Builds on slice 1
(device discovery). This slice is intentionally narrow: **load and play only**.
Pause, seek, and volume are explicitly deferred to slice 3, per the
"Track → Load → Play → Pause → Seek" progression in rule 46.

## Problem
Need the smallest working path from "WAV file on disk" to "audible sound out
the default device," on two independent decks, without touching the audio
thread's real-time constraints (rule 5).

## User behavior
Not user-facing yet. A second diagnostic tool loads a WAV file onto a deck
and plays it to completion. Two decks can be loaded and started independently
to prove they don't interfere with each other.

## Technical approach
- `core/audio/WavFile` — parses a PCM WAV file (RIFF/fmt/data chunks) fully
  into memory as interleaved `float` samples, exposing sample rate and
  channel count. WAV is a simple, fully-specified format — no third-party
  decoder needed (keeps rule 16 dependency discipline: don't add a dependency
  to avoid a small amount of well-understood code). Supports 16-bit PCM and
  32-bit IEEE float source data, converted to `float` on load.
- `core/audio/Deck` — owns a loaded `WavFile`'s buffer and a playback
  position. `load()` happens off the audio thread (file I/O, rule 5).
  `play()` only flips an atomic "playing" flag — no I/O or allocation.
  A `renderInto(float* out, size_t frames)` method is the only thing the
  audio callback calls: it copies samples into the output buffer, advancing
  position atomically, doing no locking or allocation.
- `core/audio/AudioEngine` — owns the RtAudio stream and two `Deck`
  instances. The audio callback mixes both decks by simple sample-wise
  addition into the output buffer (no gain staging or clipping protection
  yet — explicitly deferred, see Risks).
- Device/sample rate: opens the stream at the loaded file's native sample
  rate for this slice (no resampling yet — see Risks/deferred work).

## Alternatives considered
- Writing a WAV parser vs. pulling in a header-only library (e.g. dr_wav):
  chose to hand-roll given WAV's simplicity, consistent with not adding
  dependencies to avoid small amounts of code (rule 16). Revisit if edge
  cases (e.g. exotic chunk layouts, ADPCM) prove costly to support manually.
- Mixing decks by direct summation vs. building a proper mixer bus now:
  summation chosen to keep this slice minimal; a real mixer (gain, EQ,
  crossfader) is Milestone 4 scope, not Milestone 1.

## Risks
- No resampling: if a loaded file's sample rate isn't supported by the
  output device, stream opening will fail. Deferred — flagged as a known
  gap, not silently ignored (rule 6 lists resampling as a required
  consideration before this reaches real users).
- No clipping protection when two decks sum: summing two full-scale signals
  can exceed [-1, 1] and clip. Acceptable for this internal diagnostic slice
  only; must be addressed before this reaches any real playback UI (rule 6).
- Hand-rolled WAV parser only handles PCM16/Float32 — malformed or exotic
  WAV files are not yet defended against (rule 30, malformed audio files);
  parsing errors will be reported, not silently ignored, but robustness
  testing against malformed input is deferred to a later hardening pass.

## Testing
- Manual: generate a known test WAV (sine wave, via `sox`), load it on deck 1,
  play, confirm audible output matches expectations (duration, pitch).
- Manual: load different WAV files on deck 1 and deck 2 simultaneously,
  confirm both audible and mixed.
- No automated test yet for audio output itself (requires audio hardware);
  `WavFile` parsing logic (header parsing, sample conversion) is a candidate
  for a unit test in a follow-up slice since it has no I/O side effects once
  given a byte buffer.

## Files
- `core/audio/WavFile.h` / `.cpp` — WAV parsing.
- `core/audio/Deck.h` / `.cpp` — single-deck playback state.
- `core/audio/AudioEngine.h` / `.cpp` — RtAudio stream + two-deck mixing.
- `app/tools/play_wav.cpp` — diagnostic tool: load two files, play, wait.
