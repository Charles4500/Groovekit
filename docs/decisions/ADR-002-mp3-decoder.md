# ADR-002: MP3 Decoding Library

## Status
Accepted — 2026-08-01

## Problem
Milestone 1 requires playback of both WAV and MP3 files. Need a decode-only MP3 library that is simple to integrate into a CMake build across Linux/macOS/Windows and doesn't introduce licensing friction.

## Context
- Only decoding is needed (not encoding) — DJ playback, not authoring.
- Must not complicate the build system across all three target platforms (rule 7).
- Must not introduce a dependency that risks commercial distribution (rule 17).

## Options Considered
- **dr_mp3** — single-header, public domain (also dual-licensed MIT-0), decode-only, trivial to vendor and build on all platforms.
- **minimp3** — single-header, CC0/public domain, minimal API, similarly easy to vendor.
- **libmpg123** — full library, LGPL-licensed, more mature/robust but requires linking a shared/static library and LGPL compliance review for closed-source distribution.

## Decision
Use **dr_mp3** (single-header, `dr_libs` project by David Reid).

## Why
- Public-domain / MIT-0 dual license avoids the LGPL compliance overhead libmpg123 would introduce (rule 17).
- Single-header integration means no separate build target, shared library, or platform-specific linking concerns — keeps the build skeleton simple (rule 16, avoid unnecessary dependency surface).
- Decode-only fits the requirement exactly; no unused encoder surface area.

## Trade-offs
- Single-header decoders are generally less exhaustively battle-tested against malformed/adversarial MP3 files than libmpg123 — must fuzz/test against malformed files per rule 30 (security: malformed audio files) before trusting it on untrusted user libraries.
- No dedicated maintainer team backing it in the way mpg123 has; relies on the `dr_libs` repository's ongoing maintenance.

## Open Items (must verify before relying on this ADR)
1. Confirm current license text in the `dr_mp3` header/repository against the upstream source (public domain / MIT-0 dual-license is the commonly cited status — **not yet verified here**).
2. Record verified license in `THIRD_PARTY_LICENSES.md`.
3. Add malformed-file test cases once the decoder is integrated (rule 28, rule 30).

## Revisit Conditions
- If dr_mp3 proves unreliable on real-world malformed/edge-case MP3 files during testing.
- If gapless playback or VBR seeking accuracy prove insufficient for DJ workflow needs.
