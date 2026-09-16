<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Linux%20%7C%20Windows-blue">
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6.9.3-brightgreen">
  <img alt="Latest Release" src="https://img.shields.io/github/v/release/debalex77/USG">
  <img alt="Downloads" src="https://img.shields.io/github/downloads/debalex77/USG/total">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0--or--later-blue">
  <a href="https://github.com/debalex77/USG/actions/workflows/build-linux.yml"><img alt="Linux build" src="https://github.com/debalex77/USG/actions/workflows/build-linux.yml/badge.svg?branch=master"></a>
  <a href="https://github.com/sponsors/debalex77"><img alt="Sponsor" src="https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github"></a>
</p>

# USG – Records of Ultrasound Examinations

[English](README.md) | [Română](README-RO.md)

USG is an open-source desktop application for managing patients, ultrasound
orders, structured examination reports and attached medical images. It is
intended for physicians, medical offices and small clinics.

The application can work with a local SQLite database or a MariaDB database.
An optional, explicitly configured synchronization workflow can replicate
local records to MariaDB. USG does not require a cloud service for normal
local operation.

Official website: <https://debalex77.github.io/USG/>

## Screenshots

![Main window](https://github.com/user-attachments/assets/89b3964d-31d1-44ed-bf22-5c2b13642851)
![Patient records](https://github.com/user-attachments/assets/667abdf9-c456-49e2-91bc-4981ce476da9)

## Main features

- patient records and ultrasound examination history;
- ultrasound orders and structured ultrasound reports;
- dedicated views for orders and reports, with filtering and preview;
- examination sections for internal organs, urinary system, prostate,
  gynecology, breast, thyroid, pregnancy and lymph nodes;
- image and video attachments;
- customizable print templates and PDF export through LimeReport;
- sending order, report and image attachments by e-mail;
- patient appointment scheduling with multiple investigations;
- statistical, medical and pricing reports;
- SQLite and MariaDB support;
- optional UUID-based background synchronization to MariaDB;
- Romanian, English and Russian user interfaces.

## Version 4.1.1

Version 4.1.1 fixes price-list printing on SQLite and MariaDB and corrects
the A4 landscape order layout, including the central separator.

Version 4.1.0 includes the patient-schema migration, revised SQLite/MariaDB
compatibility, UUID synchronization, the new `OrderView` and `ReportView`,
appointment improvements, safer application settings and logging, and fixes
for report validation, printing, PDF export and e-mail preparation.

LimeReport was updated from 1.7.14 to 1.7.23 and is built for Qt 6.9.3. The
exact upstream revision and the USG-specific shutdown patch are documented in
[`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md).

See [`resources/RELEASES.md`](resources/RELEASES.md) for the complete release
history.

> Before upgrading an existing installation, back up the main database, the
> image database and the application configuration. A shared database should
> be migrated by a single client, without running old and new USG versions at
> the same time.

## Installation packages

Linux release artifacts may include:

- a Qt Installer Framework `.run` installer;
- a Debian `.deb` package;
- a portable `.AppImage`.

Download published binaries only from the project's
[GitHub Releases](https://github.com/debalex77/USG/releases) page and verify the
provided SHA-256 checksum before installation.

Database copies, test databases and user configuration files are never part
of the release packages.

### Running the AppImage

The AppImage runtime normally uses FUSE 2. On Debian 13, install the compatible
runtime library with:

```bash
sudo apt install libfuse2t64
chmod +x USG_v4.1.1-x86_64.AppImage
./USG_v4.1.1-x86_64.AppImage
```

If FUSE cannot be installed, use the AppImage runtime's extraction fallback:

```bash
./USG_v4.1.1-x86_64.AppImage --appimage-extract-and-run
```

The fallback extracts the package temporarily and is therefore slower to
start. It does not require FUSE.

## Building from source

The supported reference configuration for version 4.1.1 is:

| Component | Version / requirement |
|---|---|
| Qt | 6.9.3 |
| C++ | C++20 |
| Build system | qmake |
| LimeReport | 1.7.23, pinned revision and USG patch |
| OpenSSL | 3.x |
| Database | SQLite or MariaDB |

USG intentionally uses qmake; CMake files are not provided.

Prepare LimeReport and the required third-party libraries as described in
[`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md), then run:

```bash
git clone https://github.com/debalex77/USG.git
cd USG
mkdir -p build/Desktop_Qt_6_9_3-Release
cd build/Desktop_Qt_6_9_3-Release
/path/to/Qt/6.9.3/gcc_64/bin/qmake ../../USG.pro CONFIG+=release
make -j2
```

The Linux packaging workflow is implemented in
[`build_scripts/build_new`](build_scripts/build_new). It requires
CQtDeployer, Qt Installer Framework and AppImageTool in addition to the build
dependencies.

## Data privacy

Patient databases, images, configuration files, passwords and logs can contain
confidential information. Do not attach real production data to bug reports.
Reproduce database problems using an anonymized copy and remove personal,
medical and authentication data before sharing it.

## Licensing and third-party material

USG is licensed under the GNU General Public License, version 3 or any later
version. See [`LICENSE.txt`](LICENSE.txt).

Third-party icon and image provenance is documented in
[`third_party/THIRD_PARTY_ICONS.md`](third_party/THIRD_PARTY_ICONS.md).
LimeReport licensing, source correspondence and the local patch are documented
in [`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md).

## Issues

Please report reproducible problems through
[GitHub Issues](https://github.com/debalex77/USG/issues). Include the USG
version, operating system, database engine and relevant anonymized log lines.
Do not publish patient data or credentials.
