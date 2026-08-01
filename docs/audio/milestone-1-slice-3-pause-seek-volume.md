# Milestone 1 — Slice 3: Pause, Seek, Volume

Per rule 43/46. Builds on slice 2 (load + play). Completes the
"Track → Load → Play → Pause → Seek" progression, plus volume (listed
alongside play/pause/seek in the Milestone 1 scope).

## Problem
A DJ needs to pause a playing deck, jump to an arbitrary position (cueing),
and control per-deck output level — all without glitching the audio thread.

## User behavior
Not user-facing yet. Diagnostic-level: a deck can be paused/resumed, seeked
to a given time, and have its volume adjusted, all while another deck
continues playing unaffected.

## Technical approach
- **Pause:** `playing_` atomic flag already exists (slice 2); `pause()` is
  simply the mirror of `play()` — no new synchronization needed.
- **Seek:** seeking from a non-audio thread while the audio thread is mid-
  callback for the same deck is a genuine race if we naively write
  `positionFrames_` directly — the callback's own end-of-block position
  update could clobber a seek that landed mid-block, or vice versa.
  Resolved with a lock-free pending-seek handoff: `seek()` stores the
  target frame into a separate `pendingSeekFrames_` atomic (sentinel `-1`
  = none pending). `renderInto()` checks and applies it once, atomically,
  at the *start* of each callback before rendering — so a seek always
  takes effect at a block boundary, never mid-block.
- **Volume:** a `std::atomic<float>` gain in `[0, 1]`, applied as a
  multiply per sample in `renderInto()`. No smoothing/ramping yet (see
  Risks — instantaneous gain changes can click).

## Alternatives considered
- Locking (mutex) around position/seek instead of lock-free atomics:
  rejected — a mutex on the audio thread risks priority inversion and
  blocking (rule 5 explicitly forbids expensive locks in the audio path).
- Applying seek immediately/directly to `positionFrames_` from the calling
  thread: rejected due to the race described above.

## Risks
- Instantaneous volume changes can produce audible clicks/zipper noise —
  proper implementations ramp gain over a few milliseconds. Deferred; flagged
  here so it isn't mistaken for finished work (rule 6, rule 48 Definition of
  Done).
- Seeking mid-playback while `load()` is replacing `track_` on the same
  deck remains unsupported (documented already in slice 2) — still true.
- No fade/click suppression when pausing either — an abrupt stop can pop.
  Deferred, same reasoning as volume ramping.

## Testing
- New: `tests/core_audio/deck_test.cpp` — a plain-assert unit test (no test
  framework dependency added yet, per rule 16) that writes a small synthetic
  WAV to a temp file, loads it on a `Deck`, and verifies via `renderInto()`
  into an in-memory buffer (no real audio device needed):
  - playing advances position frame-accurately,
  - pause halts advancement,
  - seek relocates position at the next render call, applied atomically,
  - volume scales sample amplitude as expected,
  - end-of-track correctly clears `isPlaying()`.
- Registered with CTest (`enable_testing()` already present in root
  CMakeLists) so it runs as part of the build, giving Milestone 1 its first
  real regression test (rule 29).

## Files
- `core/audio/Deck.h` / `.cpp` — pause/seek/volume.
- `tests/core_audio/CMakeLists.txt`, `tests/core_audio/deck_test.cpp` — new.
- `tests/CMakeLists.txt` — wire up the new test directory.
