# Third-Party Licenses

Tracked per ENGINEERING_STANDARDS.md rule 17. Entries must be verified against primary sources before relying on them for distribution — see open items in [ADR-001](docs/decisions/ADR-001-tech-stack.md).

| Dependency | Purpose | License (unverified) | Verified? | Notes |
|---|---|---|---|---|
| Qt | UI framework | LGPLv3 / GPLv3 / Commercial (commonly cited) | No | Must confirm against qt.io licensing page before commercial distribution. |
| RtAudio | Audio I/O backend | MIT (with non-binding request to share modifications upstream) | Yes (2026-08-01, v6.0.1 LICENSE file) | Permissive, no distribution restriction found. |
| dr_mp3 | MP3 decoding | Public domain / MIT-0 dual-license (commonly cited) | No | Must confirm against upstream `dr_libs` repository. See [ADR-002](docs/decisions/ADR-002-mp3-decoder.md). |

No dependency should be added to the build without an entry here (rule 16, 17).
