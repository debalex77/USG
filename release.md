# USG v4.2.7

## Highlights

- User passwords are stored as salted PBKDF2-SHA256 hashes. Existing SHA-256
  hashes are converted during the database migration; passwords do not
  change.
- The legacy reversibly encoded password column (databases created before
  4.1.0) is cleared.
- The cloud server password is encrypted with the organization split key
  (one part in the database, one part in a local profile file), like e-mail
  account passwords. Changing a user password no longer affects the cloud
  configuration. After restoring the database on another computer the cloud
  password must be entered again.
- User names are unique and case-insensitive on SQLite and MariaDB. Existing
  duplicates are renamed during migration ("name (2)") and listed in the
  information panel.
- The duplicate user name check when creating or renaming users was fixed.
- New or changed passwords must have at least 8 characters.
- Login: after 3 failed attempts for the same name a pause is enforced
  (30 s, doubling up to 300 s); the error message no longer reveals whether
  the name or the password is wrong. Legacy hashes are rewritten in the new
  format after the first successful login.
- A database moved without the profile `crypto/` directory is detected; the
  application asks for the cloud and e-mail account passwords to be entered
  again.
- 7z archiving: correct progress, a progress dialog for archiving on exit,
  optional settings and encryption key directories, optional AES-256
  password-protected archives, and verification after creation.
- Patients catalog: search by name, IDNP or birth date (Ctrl+F); paging keeps
  the current row; patients without documents or appointments can be removed
  from the database (Delete button menu, context menu, Shift+Delete),
  otherwise the referencing documents are listed. Marking patients for
  deletion was fixed.
- Patient search in Orders, Reports, patient history and appointments also
  matches "Last name First name".
- User preferences: OK, Save, Close button order; translated confirmation for
  unsaved changes.
- Ultrasound Order journal: search field (toolbar button or Ctrl+F) by
  patient (name, IDNP) or by investigation (code or name); the criterion is
  chosen from the field menu, and investigation search shows the matching
  investigations.
- Patient appointments: print form for the day's appointments (A4
  landscape, organization header and doctor name) from the **Print** button.
- Patient appointments: fixed editor warnings when editing cells (patient,
  investigations, organization, doctor, "Done") and the display of completed
  rows.

## Included changes from v4.2.6

- Several validated ultrasound reports can be sent in one e-mail to the
  referring organization, optionally with the report images.
- Seasonal splash screens for autumn and spring; Russian translation updated.

## Upgrade and verification

Version 4.2.7 migrates the database schema (account security). Create backups
of the main database, image database, and application profile before
installing the update. On a MariaDB database shared by several workstations,
update all workstations: earlier versions cannot verify the converted
passwords.

For a SQLite profile synchronized with MariaDB:

1. Update and open the SQLite profile first while the cloud server is available.
2. Wait until startup checks finish and review the application log.
3. Open the MariaDB profile directly only after the SQLite-side update has been
   confirmed.

After upgrading, verify authentication, open the ultrasound Order journal and
confirm that:

- every user can log in with the existing password, and the user name is
  accepted regardless of letter case;
- the information panel lists any renamed duplicate users;
- the cloud server configuration shows the saved password and
  synchronization works;
- creating or renaming a user with an existing name is rejected;
- changing a user password keeps the cloud configuration readable;
- repeated failed logins are paused and short passwords are rejected;
- encrypted archives can be created and restored with 7-Zip;
- the Patients catalog search and patient removal work as expected;
- the Order journal search by patient and by investigation finds the
  expected documents;
- the appointments print form opens in preview and in the designer.

## Windows packages

- `USG_v4.2.7_Windows_amd64.exe` — Windows installer.
- `LimeReport_v1.7.23_USG_source.zip` — bundled dependency source.
- SHA-256 checksum files are published with the artifacts.

## Linux packages

- `USG_v4.2.7_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.7_Linux_amd64.deb` — Debian package.
- `USG_v4.2.7-x86_64.AppImage` — portable AppImage package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — LimeReport source and the local patch
  used by USG.

Verify downloaded packages using the published SHA-256 checksums.

### Running the AppImage

```bash
chmod +x USG_v4.2.7-x86_64.AppImage
./USG_v4.2.7-x86_64.AppImage
```

If FUSE is not available:

```bash
./USG_v4.2.7-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
