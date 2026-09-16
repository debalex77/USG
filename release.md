# USG v4.1.1

This maintenance release fixes price-list printing and the A4 landscape layout
of ultrasound orders.

## Fixes

- Fixed price-list printing in `PricingDialog` on SQLite and MariaDB after the
  migration to numeric investigation group IDs.
- Distinguished empty investigation groups from SQL query failures and improved
  cleanup when printing is cancelled or fails.
- Corrected the `Order.lrxml` paper size for Qt 6 so that A4 landscape is preserved
  when the template is loaded.
- Centered the order separator at 148.5 mm and adjusted both copies to fit the
  A4 landscape page, with printing at the configured page size.

## Updating

Back up your database, application configuration and customized print templates
before updating. If you use a custom template directory, update `Order.lrxml`
and `Pricing.lrxml` there as well, preserving any local customizations.

This release adds no database schema migration. Earlier migrations still apply
when upgrading from versions older than 4.1.0.

## Linux packages

- `USG_v4.1.1_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.1.1_Linux_amd64.deb` — Debian package.
- `USG_v4.1.1-x86_64.AppImage` — portable package.
- `LimeReport_v1.7.23_USG_source.tar.gz` — corresponding LimeReport source and
  the patch used by USG.

Verify each package against its supplied SHA-256 checksum before installation.

### Running the AppImage

```bash
chmod +x USG_v4.1.1-x86_64.AppImage
./USG_v4.1.1-x86_64.AppImage
```

If FUSE is unavailable, use temporary extraction:

```bash
./USG_v4.1.1-x86_64.AppImage --appimage-extract-and-run
```

Full release history: [`resources/RELEASES.md`](resources/RELEASES.md).
