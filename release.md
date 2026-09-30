# USG v4.2.4

## Highlights

- Corrected the separation between the document organization and the printing
  identity. The organization stored in an ultrasound Order remains the
  referring organization, while printed headers and images use the performing
  organization and default doctor configured in User Preferences.
- Ultrasound Orders now use the performing organization's logo and organization
  stamp.
- Ultrasound Reports use the performing organization's identity and the default
  doctor's stamp and signature. Their visibility continues to follow the print
  parameter checkboxes.
- PDF attachments generated for e-mail now use the same printing identity as
  document preview and printing.
- Corrected the image layout used by statistical reports: organization logo,
  doctor stamp, and doctor signature. The organization stamp no longer replaces
  the doctor's signature.

## Included changes from v4.2.3

- Rewritten Romanian User Manual with updated screenshots and direct access
  through the **Online Manual** action in the main window's Help menu.
- Added the **Cloud Servers** window for viewing, adding, editing, and removing
  synchronization configurations.
- SQLite–MariaDB synchronization can be enabled or disabled from application
  preferences.
- Revised PDF export, attachments, e-mail account selection, and the e-mail
  agent workflow.
- Completed patient-history actions and the ultrasound Order/Report journals.
- Corrected the Cloud Servers window and application shutdown flow.
- Linux packages use a portable RPATH and bundle LimeReport without requiring a
  global LimeReport installation.
- Updated the Russian interface translation for the recently added features.

## Upgrade and verification

Create backups of the main database, image database, and application profile
before installing the update.

For a SQLite profile synchronized with MariaDB:

1. Update and open the SQLite profile first while the cloud server is available.
2. Wait until startup checks finish and review the application log.
3. Open the MariaDB profile directly only after the SQLite-side update has been
   confirmed.

After upgrading, verify authentication, open an existing ultrasound Order and
Report, and confirm that:

- the referring organization is still stored in the Order;
- the printed header and logo belong to the performing organization configured
  in User Preferences;
- the ultrasound Report shows the configured doctor's stamp and signature when
  enabled;
- statistical reports show the doctor stamp and signature, not the organization
  stamp;
- PDF export and e-mail attachments use the same printing identity.

## Windows packages

- `USG_v4.2.4_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

## Linux packages

- `USG_v4.2.4_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.4_Linux_amd64.deb` — Debian package.
- `USG_v4.2.4-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.4-x86_64.AppImage
./USG_v4.2.4-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.4-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
