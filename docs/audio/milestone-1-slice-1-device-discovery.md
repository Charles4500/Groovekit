# Milestone 1 — Slice 1: Audio Device Discovery

Per rule 43 (before coding) and rule 46 (small iterations) — this is the first vertical slice of
Milestone 1, before output/playback/pause/seek.

## Problem
The engine needs to enumerate available audio output devices on Linux/macOS/Windows via RtAudio,
as the foundation every later playback feature depends on.

## User behavior
Not user-facing yet. This slice produces a `core/audio` component and a small diagnostic
executable that lists detected output devices and their capabilities (sample rates, channel
counts) — used to manually verify RtAudio integration on each platform.

## Technical approach
- Add RtAudio as a dependency via CMake `FetchContent`, pinned to a specific tagged release
  (not a floating branch) for build reproducibility across platforms (rule 53).
- `core/audio/AudioDeviceManager` wraps `RtAudio` device enumeration behind our own interface,
  so the rest of the engine never depends on the RtAudio API directly (rule 34 — core boundaries
  like audio should be intentionally designed/isolated).
- No playback yet — enumeration only.

## Alternatives considered
- Vendoring RtAudio source directly in-repo: rejected for now — FetchContent with a pinned tag
  gives the same reproducibility without committing third-party source into our history.
- System package manager (apt/brew/vcpkg) dependency: rejected as the sole path — package
  availability/version differs across Linux distros, macOS, and Windows, undermining rule 7
  (cross-platform first-class). FetchContent keeps the build self-contained.

## Risks
- RtAudio's CMake integration quality varies by version — must verify it configures cleanly on
  this machine (Linux) before assuming it will elsewhere; macOS/Windows verification is not
  possible in this environment and is an open item.
- Device enumeration behavior on headless/CI environments (no real audio device) is unverified.

## Testing
- Manual: build and run the diagnostic executable, confirm it lists at least one device on this
  Linux machine, matching what `pactl list short sinks` / `aplay -l` reports.
- No automated test yet — device enumeration depends on real hardware/drivers; this slice is a
  connectivity check, not logic to unit test.

## Files
- `CMakeLists.txt` (root) — add `FetchContent` for RtAudio.
- `core/audio/CMakeLists.txt` — new library target.
- `core/audio/AudioDeviceManager.h` / `.cpp` — device enumeration wrapper.
- `app/tools/list_audio_devices.cpp` (or similar) — diagnostic executable.
