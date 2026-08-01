# ADR-001: Core Technology Stack

## Status
Accepted — 2026-08-01

## Problem
Need a language, UI framework, audio I/O backend, and build system that support a professional, cross-platform (Linux/macOS/Windows) real-time DJ application, per ENGINEERING_STANDARDS.md rules 5–8.

## Context
- Audio engine must be real-time safe: no allocations, blocking I/O, or GC pauses on the audio thread.
- Application must run natively on Linux, macOS, and Windows without a "fix one platform later" approach.
- Core DJ logic must be separable from and testable without the UI.

## Options Considered

### Language
- **C++** — native performance, direct control over memory/threads, mature real-time audio ecosystem. Higher development overhead, manual memory/lifetime management.
- **Rust** — native performance with memory safety guarantees, growing audio ecosystem (cpal, cubeb). Smaller pool of mature DJ-adjacent libraries, younger ecosystem for some audio codecs/DSP.
- **C++ core + web UI shell (Tauri/Electron)** — familiar UI dev, but adds an IPC boundary between UI and real-time engine, more moving parts.

### UI Framework
- **Qt** — mature native cross-platform C++ toolkit, good performance, wide OS support, custom widget support for waveform/deck UI.
- **JUCE** — audio-first framework bundling UI and audio primitives; strong fit for audio apps, but couples UI framework choice to audio framework choice.
- **Web-based shell** — fast UI iteration but crosses a process/IPC boundary to reach the real-time engine, adding latency/complexity risk (rule 5).

### Audio I/O Backend
- **RtAudio** — lightweight, direct callback-based buffer control, MIT-licensed, established track record in audio tools.
- **PortAudio** — very mature, BSD-style license, more C-style API.
- **JUCE audio module (standalone)** — capable but pulls in a heavier framework; licensing (GPL/commercial dual-license) needs verification before adoption.

## Decision
- **Language:** C++ (core engine, platform layer, and application logic).
- **UI:** Qt.
- **Audio I/O backend:** RtAudio.
- **Build system:** CMake, with `core/`, `platform/{linux,macos,windows}/`, `app/` (UI), and `tests/` as top-level separations, per ENGINEERING_STANDARDS.md rule 7–8.

## Why
- C++ gives direct, deterministic control over the audio thread with no runtime GC — required by ENGINEERING_STANDARDS.md rule 5 (audio engine real-time constraints).
- Qt is a mature, native, cross-platform toolkit with a large history of use in professional audio tools, and keeps UI cleanly separable from `core/` (rule 8).
- RtAudio is small, well-understood, and focused purely on audio I/O — avoids pulling in a large framework (rule 16, dependency discipline) while giving the low-level buffer/callback control the audio engine needs.
- CMake is the de facto standard cross-platform C++ build system, with first-class support for Linux/macOS/Windows toolchains.

## Trade-offs
- C++ requires more manual discipline (RAII, ownership, no automatic memory safety) than Rust — mitigated by code review discipline and tests (rules 26–29), not tooling.
- Qt has licensing implications (LGPL vs. commercial) that must be verified per rule 17 before any commercial distribution — **not yet verified, tracked as open item below**.
- RtAudio/PortAudio licenses (MIT / BSD-style respectively) are commonly cited as permissive but must be confirmed against current official sources before relying on them — **not yet verified**.

## Open Items (must verify before relying on this ADR for distribution)
1. Confirm current Qt licensing terms (LGPLv3/GPLv3/commercial) against qt.io official licensing page, and whether the intended distribution model (commercial, closed-source) is compatible.
2. Confirm RtAudio's current license text in its repository (commonly cited as MIT, unverified here).
3. Record both in `THIRD_PARTY_LICENSES.md` once verified.

These are **assumptions, not facts**, per ENGINEERING_STANDARDS.md rule 3, until checked against primary sources (rule 15).

## Revisit Conditions
- If Qt licensing proves incompatible with the intended distribution/business model.
- If RtAudio proves insufficient for a required audio feature (e.g., specific low-latency driver support) discovered during Milestone 1.
- If cross-compilation/packaging friction with Qt+CMake becomes a blocker across all three target platforms.
