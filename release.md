# USG v4.2.0

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
- Reorganized application settings and runtime state around dedicated services
  and contexts for paths, database connections, cloud configuration, session,
  organization and doctor data.
- Migrated application, organization and user preferences from the legacy
  settings layout to dedicated database tables, with post-migration checks.
- Reorganized the source tree and qmake project fragments by application layer
  and feature, and revised dependency usage and the Windows build script.
- Added informed-consent text generation for four investigation categories:
  invasive, non-invasive, endocavitary and obstetric screening examinations.
- Added an Options menu to the ultrasound report for selecting additional
  investigations and controlling doctor stamp/signature visibility when
  previewing or printing. Existing automatic PDF-export behavior is preserved.
- Updated the Russian Qt translation catalog for 4.2.0, including the medical
  labels in the gestation and internal-organ report pages.
- Fixed language-change restart: the current window closes through its normal
  cleanup without the exit/tray prompt, and the database-selection dialog uses
  the last selected profile language from the start of the next process.
- Fixed e-mail account passwords not being saved on upgraded databases: the
  missing `cryptoSplitKey` table is now created by the 4.2.0 migration and
  repaired at startup for databases already marked 4.2.0.
- Implemented the SMTP connection check in the online account dialog; the result
  is shown in a notification without sending a message.
- Online account save failures now show the reason instead of failing silently.
- Required fields are highlighted consistently with balloon tips in the pricing,
  catalog, cloud, e-mail, login and statistical report dialogs.

## Updating

Back up your database and configuration before updating. Existing databases gain
an optional `site` column in `organizations`, followed by migration of legacy
settings into the dedicated 4.2.0 settings tables. Update both local and MariaDB
installations when using synchronization.

## Linux packages

- `USG_v4.2.0_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.0_Linux_amd64.deb` — Debian package.
- `USG_v4.2.0-x86_64.AppImage` — portable package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — corresponding LimeReport source and
  the patch used by USG.

Verify each package against its supplied SHA-256 checksum before installation.

### Running the AppImage

```bash
chmod +x USG_v4.2.0-x86_64.AppImage
./USG_v4.2.0-x86_64.AppImage
```

If FUSE is unavailable, use temporary extraction:

```bash
./USG_v4.2.0-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
