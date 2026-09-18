# USG v4.1.2

## Changes

- Renamed global variables and profile path keys, with automatic migration of old
  `.conf` profiles and a `.pre-4.1.2.bak` backup.
- Fixed bullet rendering in the update information panel.

- Added the organization website field, including saving, loading and display in
  the printed ultrasound order (`Order.lrxml`).
- Added creation and editing of the referring doctor directly from
  `OrderDialog`, with automatic model refresh and selection of the newly created
  doctor.
- Added a resumable SQLite/MariaDB migration for `organizations.site`, preserving
  existing values when the column has already been added manually.
- Kept organization data loading compatible with databases awaiting migration.
- Kept the previous version in the main window title until the update succeeds.
- Added migration steps, UUID update counts and errors to the information panel.

## Updating

Back up your database and configuration before updating. Existing databases gain
an optional `site` column in `organizations`. Update both local and MariaDB
installations when using synchronization.

## Linux packages

- `USG_v4.1.2_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.1.2_Linux_amd64.deb` — Debian package.
- `USG_v4.1.2-x86_64.AppImage` — portable package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — corresponding LimeReport source and
  the patch used by USG.

Verify each package against its supplied SHA-256 checksum before installation.

### Running the AppImage

```bash
chmod +x USG_v4.1.2-x86_64.AppImage
./USG_v4.1.2-x86_64.AppImage
```

If FUSE is unavailable, use temporary extraction:

```bash
./USG_v4.1.2-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
