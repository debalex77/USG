# Windows CI and release

`build-windows.yml` runs on branch pushes, pull requests and manual dispatch.
It is also reused by `release-windows.yml`, which runs on `v*` tags or manually.

The runner is Windows Server 2022 with MSVC x64 and Qt 6.9.3. Dependencies are
built on the runner; no files from a developer's `C:\Projects` or `C:\Qt` are used.
LimeReport uses the same pinned commit and shutdown patch as Linux. vcpkg is
pinned to commit `4334d8b4c8916018600212ab4dd4bbdc343065d1` (2025.09.17).
OpenSSL uses the static library / dynamic MSVC runtime triplet.

QMYSQL is compiled from Qt 6.9.3 sources against MariaDB Connector/C, which
supports MySQL and MariaDB. The client uses Schannel. The CI overlay embeds
the standard authentication plugins in the client DLL; optional ed25519
authentication is disabled. Servers requiring that plugin need an additional
deployment configuration. Dependency DLLs and license notices accompany USG.
Qt Installer Framework 4.11 is downloaded from Qt's versioned repository,
with its archive checksum verified before extraction. The legacy aqt
`tools_ifw` feed still supplies 4.7 and is not used.

Each successful run uploads `USG-Windows-amd64`, containing:

- the Windows installer and its SHA-256 checksum;
- the patched LimeReport source archive and its SHA-256 checksum.

For a release, update `version.txt`, the version constants in `USG.pro`, and
`release.md`, then push the matching `vX.Y.Z` tag. The workflow verifies the tag
against `version.txt`. It creates or updates a **draft** GitHub Release, leaving
Linux assets in place. Published releases are not overwritten. A manual run on
a branch produces Actions artifacts without creating a GitHub Release.

No extra repository secrets are required. Release upload uses `github.token`
with `contents: write`; build jobs have read-only repository access.

Local checks validate workflow syntax and the packaging prerequisites. The
complete clean build and installer execution must be confirmed on the first
GitHub Actions run. Installers are not code-signed by this workflow.
