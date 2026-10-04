# USG v4.2.8

## Highlights

- The selected database profile (`.conf`) can be opened for editing directly
  from the Database Selection window.
- On Linux the profile is opened with the system default application; on
  Windows it is opened with Notepad. Missing files and launch failures are
  reported to the user.
- Database Selection action buttons now use consistent icon and text
  alignment, including the new **Edit** action.
- Corrected Qt resource references in the initial-launch, cloud server,
  online account and e-mail agent dialogs.
- Added example placeholders for MariaDB, cloud server and e-mail account
  connection fields.
- Refreshed the application splash artwork and updated the third-party icon
  inventory and attribution for the database settings icon.
- The Russian interface translation was updated and regenerated; all current
  source texts are translated.
- User manual and translation-tool files are explicitly listed in the qmake
  project metadata for easier project maintenance.

## Upgrade and verification

Version 4.2.8 does not introduce a new database-schema migration. Normal backup
practice still applies: back up the main database, image database and
application profile before installing the update.

After upgrading, verify that:

- the Database Selection window lists existing profiles and **Edit** opens the
  selected `.conf` file;
- Database Selection buttons have aligned icons and labels;
- icons are visible in the initial-launch, cloud server, online account and
  e-mail agent dialogs;
- connection fields display the new examples without replacing saved values;
- the Romanian and Russian interfaces display the updated texts correctly.

## Windows packages

- `USG_v4.2.8_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

## Linux packages

- `USG_v4.2.8_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.8_Linux_amd64.deb` — Debian package.
- `USG_v4.2.8-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.8-x86_64.AppImage
./USG_v4.2.8-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.8-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
