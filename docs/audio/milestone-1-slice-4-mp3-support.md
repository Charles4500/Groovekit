# Milestone 1 — Slice 4: MP3 Playback

Per rule 43/46. Builds on slices 2–3 (WAV load/play/pause/seek/volume).

## Problem
`Deck` is currently hard-coded to `WavFile`. Need to add MP3 support (per
ADR-002: dr_mp3) without duplicating all of Deck's playback/seek/volume
logic for a second file type.

## User behavior
Not user-facing yet. `Deck::load()` now accepts `.mp3` files as well as
`.wav`, selected by file extension. Playback/pause/seek/volume behave
identically regardless of source format.

## Technical approach
- Introduce `core/audio/AudioSource` — a small abstract interface
  (`sampleRate()`, `channels()`, `frameCount()`, `samples()`) that both
  `WavFile` and the new `Mp3File` implement. This is a deliberate core
  boundary (rule 34 explicitly allows/encourages intentional design for
  audio/format boundaries, unlike speculative abstractions elsewhere).
- `Deck::track_` changes from `shared_ptr<const WavFile>` to
  `shared_ptr<const AudioSource>` — the only change needed in `Deck` itself;
  all playback/render/seek/volume logic is untouched since it only ever
  used the interface surface already.
- `Mp3File` wraps `drmp3_open_file_and_read_pcm_frames_f32` — decodes the
  entire file into memory up front, same eager-decode strategy as `WavFile`
  (consistent behavior, and dr_mp3's own docs indicate this all-in-one API
  is the straightforward path for decode-once-play-from-memory use).
- dr_mp3 is a single header (`dr_mp3.h`), fetched via CMake `FetchContent`
  (URL download of the pinned file, not a git clone — it's header-only with
  no build system of its own).
- `Deck::load()` picks `WavFile` or `Mp3File` based on the file extension
  (`.wav` vs `.mp3`, case-insensitive). Unrecognized extensions throw,
  consistent with "never guess" — we don't sniff/guess format from content
  in this slice.

## Alternatives considered
- Templating `Deck` on the source type: rejected — adds complexity for no
  benefit since we always want runtime polymorphism (a deck can load either
  format across its lifetime).
- Sniffing file content instead of trusting the extension: more robust
  long-term, but adds complexity not needed yet; deferred, noted as a risk.

## Risks
- Trusting the file extension to pick a decoder is a known-weak heuristic —
  a mislabeled file will fail (loudly, via exception) rather than being
  silently misdecoded, which is the safer failure mode but still a gap
  worth revisiting before this reaches real users (rule 30).
- dr_mp3, being a single-header hobbyist-maintained decoder, is less
  battle-tested against adversarial/malformed MP3 input than libmpg123
  (already flagged in ADR-002) — not fuzz-tested in this slice.
- No gapless playback handling (MP3 encoder delay/padding) — first slice
  proves basic decode+playback only; gapless accuracy is deferred.

## Testing
- New: extend `deck_test`-style verification with an `Mp3File` test using a
  real MP3 generated via `ffmpeg` from a synthetic WAV (deterministic tone),
  decoded and spot-checked for correct channel count/sample rate and
  non-empty, non-silent output. Exact sample-for-sample equality isn't
  meaningful for lossy MP3 (unlike the WAV test's exact-match checks).
- Manual: `play_wav` tool (renamed conceptually but kept as-is; loads by
  extension) tested against a real MP3 file end-to-end through
  `AudioEngine`.

## Files
- `CMakeLists.txt` (root) — FetchContent for `dr_mp3.h`.
- `core/audio/AudioSource.h` — new interface.
- `core/audio/WavFile.h` — implement `AudioSource`.
- `core/audio/Mp3File.h` / `.cpp` — new.
- `core/audio/Deck.h` / `.cpp` — switch to `AudioSource`, extension dispatch.
- `tests/core_audio/mp3file_test.cpp` — new.
