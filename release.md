# USG v4.2.2

## Changes

- Create the Windows log directory before first-run settings validation.
- Allow an empty password for the initial administrator.
- Fix first-run wizard icons and table icon rendering.
- Initialize investigation groups before import when the group table is empty.
- Fix organization creation and Qt 6/MSVC compatibility.
- Add Windows build and draft-release workflows, with corrected packaging paths and SHA-256 generation.
- Audit SQLite–MariaDB identities before UUID migration. Safe matches receive a
  shared UUID; ambiguous historical duplicates remain distinct and are reported.
  Patient matching validates IDNP together with NPP and birth date and never
  uses the numeric database ID as identity evidence.
  Price types use their stable commercial/CNAM role as identity, allowing their
  dependent contracts and pricing documents to be matched across translated
  names and different mutable discounts.
  Existing UUID conflicts and schema errors stop the transfer before commit.

## Updating and validation

Back up databases and configuration before updating. This patch uses the existing
4.2.0 schema. Reimport the investigation classifier if the initial group catalog
was left empty; existing custom groups are preserved.

For an SQLite profile synchronized with MariaDB, update the SQLite profile first
with the cloud connection enabled. Open the MariaDB profile directly only after
the shared UUID transfer has committed.

The Windows installer containing these fixes was tested and confirmed by the
maintainer. Newly versioned packages remain in draft pending package verification.

## Windows packages

- `USG_v4.2.2_Windows_amd64.exe` — installer.
- `LimeReport_v1.7.23_USG_source.zip` — corresponding dependency source.
- SHA-256 files are supplied alongside both archives.

## Linux packages

- `USG_v4.2.2_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.2_Linux_amd64.deb` — Debian package.
- `USG_v4.2.2-x86_64.AppImage` — portable package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — corresponding LimeReport source and
  the patch used by USG.

Verify each package against its supplied SHA-256 checksum before installation.

### Running the AppImage

```bash
chmod +x USG_v4.2.2-x86_64.AppImage
./USG_v4.2.2-x86_64.AppImage
```

If FUSE is unavailable, use temporary extraction:

```bash
./USG_v4.2.2-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
