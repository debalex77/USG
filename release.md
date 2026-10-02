# USG v4.2.6

## Highlights

- Several ultrasound reports in one e-mail: the ultrasound Report journal has
  an e-mail button (toolbar and context menu) that opens a selection window
  for validated reports, filtered by the referring organization and period.
  Rows selected in the journal (Ctrl/Shift) are checked by default.
- The selected reports are exported to PDF and attached to a single message
  addressed to the referring organization (address from the Organizations
  catalog). Images attached to the reports can optionally be included.
- Each report is re-checked before export (still validated and belonging to
  the chosen organization). On partial export the errors are shown and the
  user decides whether to continue; attachments over 20 MB require
  confirmation.
- While reports are being prepared, the journal cannot be closed and a second
  export cannot be started; the temporary directory is removed after sending
  or on cancel.
- Report image export uses a shared service for single-document and
  multi-report e-mails.
- New seasonal splash screens for autumn (1 September – 30 November) and
  spring (1 March – 31 May).
- Russian translation updated for the new features.

## Included changes from v4.2.5

- Startup checks for missing SQLite files and MariaDB databases without the
  application schema; first launch no longer recreates an existing database.
- Ultrasound Order journal: deletion, filters, sorting, preview and loading
  errors fixed; journal printing uses the shared print services.
- Safer e-mail export, pricing and statistical report fixes.

## Upgrade and verification

Version 4.2.6 does not change the database schema. Create backups of the main
database, image database, and application profile before installing the
update.

For a SQLite profile synchronized with MariaDB:

1. Update and open the SQLite profile first while the cloud server is available.
2. Wait until startup checks finish and review the application log.
3. Open the MariaDB profile directly only after the SQLite-side update has been
   confirmed.

After upgrading, verify authentication, open the ultrasound Order journal and
confirm that:

- several validated reports of one organization can be selected in the
  ultrasound Report journal and sent in a single e-mail, with and without
  images;
- the e-mail export of a single Order or Report works as before;
- filters, sorting and the document preview work on the whole period;
- statistical reports list the available templates.

## Windows packages

- `USG_v4.2.6_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

## Linux packages

- `USG_v4.2.6_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.6_Linux_amd64.deb` — Debian package.
- `USG_v4.2.6-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.6-x86_64.AppImage
./USG_v4.2.6-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.6-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
