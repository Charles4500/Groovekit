# Third-Party Licenses

Tracked per ENGINEERING_STANDARDS.md rule 17. Entries must be verified against primary sources before relying on them for distribution — see open items in [ADR-001](docs/decisions/ADR-001-tech-stack.md).

| Dependency | Purpose | License (unverified) | Verified? | Notes |
|---|---|---|---|---|
| Qt | UI framework | LGPLv3 / GPLv3 / Commercial (commonly cited) | No | Must confirm against qt.io licensing page before commercial distribution. |
| RtAudio | Audio I/O backend | MIT (commonly cited) | No | Must confirm against upstream repository LICENSE file. |

No dependency should be added to the build without an entry here (rule 16, 17).
