# USG v4.2.5

## Highlights

- Startup checks: on a regular launch the application verifies that the SQLite
  database files configured in the profile (main and image databases) exist,
  and stops with a message instead of silently creating empty databases. An
  empty MariaDB database, or one without the application schema, is now
  reported explicitly with a hint to use the first-launch setup.
- First launch: an existing database selected during setup is no longer
  recreated; the schema version is written only after full verification. The
  profile name and the image database are proposed from the selected file.
- Schema initialization and post-login constant loading use dedicated
  connections; closing the login window while loading no longer fails.
- The initial administrator is created only when the users table is empty; if
  all users are marked as deleted, the application reports it and stops.
- First-run wizard: a password is required for added users; investigation and
  price-type catalogs are no longer duplicated on repeated clicks.
- Ultrasound Order journal: deletion asks for confirmation and also removes
  images, videos and, optionally, the cloud copy; number, organization and
  contract filters are fixed; sorting by any column applies to all documents;
  the preview follows the current row; loading errors are shown.
- Printing from journals uses the same print services as the documents (stamp
  and signature hidden by default); the image in the Order template was
  resized.
- E-mail: correct detection of attached image formats, reliable cleanup of the
  temporary directory, rejection of invalid addresses, and protection against
  starting a second export in parallel.
- Pricing: fixed removal of the document header when saving rows fails and the
  error when selecting an organization (contracts).
- Statistical reports: the list contains only available `.lrxml` templates,
  with an explicit selection row.
- Russian splash screen alignment fixed; Russian translation completed.

## Included changes from v4.2.4

- Separation between the referring organization stored in the Order and the
  performing organization's printing identity (header, logo, stamps and
  signature from User Preferences).
- PDF attachments for e-mail use the same printing identity as preview.
- Statistical reports show the doctor stamp and signature, not the
  organization stamp.

## Upgrade and verification

Version 4.2.5 does not change the database schema. Create backups of the main
database, image database, and application profile before installing the
update.

For a SQLite profile synchronized with MariaDB:

1. Update and open the SQLite profile first while the cloud server is available.
2. Wait until startup checks finish and review the application log.
3. Open the MariaDB profile directly only after the SQLite-side update has been
   confirmed.

After upgrading, verify authentication, open the ultrasound Order journal and
confirm that:

- filters, sorting and the document preview work on the whole period;
- deleting a test Order removes its Report and images;
- printing and PDF/e-mail export of an Order and a Report work as before;
- statistical reports list the available templates.

## Windows packages

- `USG_v4.2.5_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

## Linux packages

- `USG_v4.2.5_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.5_Linux_amd64.deb` — Debian package.
- `USG_v4.2.5-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.5-x86_64.AppImage
./USG_v4.2.5-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.5-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
