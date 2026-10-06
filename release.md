# USG v4.2.9

## Highlights

- Added optional SQLCipher support for the local main database and image
  database. Standard SQLite remains the default.
- SQLCipher keys are requested at runtime and are not stored in the database
  profile. A key can also be supplied through `USG_SQLCIPHER_KEY`.
- The application now checks that the compatible `QSQLCIPHER` Qt plugin is
  available before opening an encrypted profile and reports a clear error when
  it is missing.
- Main-thread and worker database connections, migrations, backups and SQLite
  compatibility checks now recognize both `QSQLITE` and `QSQLCIPHER`.
- Local database selectors accept `.sqlite3`, `.sqlite`, `.db` and
  `.sqlcipher` files and no longer assume a fixed filename extension.
- Legacy SQLite profiles with an empty or zero unused MariaDB port are read
  safely.
- Added a separate Linux demo-package builder. It validates the demo databases,
  configures the local demo profile automatically and creates
  `USG_v4.2.9_Linux_amd64_demo.run` without changing the production packaging
  workflow.
- Linux and Windows packages build and include the compatible SQLCipher plugin
  (`libqsqlcipher.so` and `qsqlcipher.dll`).
- Linux packages build and bundle the pinned OpenSSL 3.5.9 LTS runtime used by
  SQLCipher; they do not replace or update the operating system's OpenSSL
  installation. On Windows the plugin and USG are statically linked with the
  same OpenSSL 3.5.9 release.
- The About window shows the SQLite, SQLCipher and OpenSSL versions for
  encrypted databases, and the license tab lists the third-party components and
  the bundled `licenses` folder.
- Fixed the "Version history" window, which could not find the release notes.
- Fixed removal of the temporary online-version file on Windows.
- Completed and regenerated the Russian translation for all current texts.

## Database compatibility and upgrade

Version 4.2.9 does not introduce a new database-schema migration. Existing
SQLite and MariaDB databases remain compatible with the normal 4.2.x upgrade
path.

Enabling SQLCipher does **not** convert an existing plain SQLite database.
Prepare and verify encrypted copies of both database files first, then select
those files and enable SQLCipher in the profile. Keep the encryption key in a
safe place: it is required at each launch and cannot be recovered by USG.

Before upgrading, back up:

- the main database;
- the image database;
- the selected `.conf` profile;
- the profile's `crypto/` and settings directories when applicable.

## Demo package

The demo installer is a separate, manually built artifact and is not added to
the normal production workflows. It uses the bundled demo databases, creates
the `usg_demo.conf` profile automatically and authenticates with user `admin`
and an empty password.

Only anonymized data may be distributed in a demo package. The build script
checks database integrity, schema and cross-database references, but the publisher remains
responsible for manually confirming that all patient and image data are
fictitious. If no verified demo image database is supplied, an empty one is
generated.

## Verification performed

- Release qmake build with Qt 6.9.3.
- SQLCipher connection test: encrypted database creation, reopen, foreign keys,
  wrong key and missing key.
- SQLCipher key-dialog test.
- Demo database validation in verification-only mode.
- Russian translation generation with zero unfinished entries.
- Shell, XML and release-metadata consistency checks.

## Windows packages

- `USG_v4.2.9_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

The Windows installer includes `qsqlcipher.dll`, built by the workflow and
statically linked with OpenSSL 3.5.9.

## Linux packages

- `USG_v4.2.9_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.9_Linux_amd64.deb` — Debian package.
- `USG_v4.2.9-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.
- `USG_v4.2.9_Linux_amd64_demo.run` — optional, separately built demo installer.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.9-x86_64.AppImage
./USG_v4.2.9-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.9-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
