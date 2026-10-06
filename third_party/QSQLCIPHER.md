# QSQLCIPHER

USG optionally uses a dynamically loaded `QSQLCIPHER` Qt SQL driver for its
local main and image databases. SQLite remains the default local database
driver.

## Driver source

The driver is derived from the official `qsqlite` driver included in Qt
6.9.3. The source files retain their original Qt copyright and SPDX notices:

- `LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only`.

The driver distributed with the GPL-3.0-or-later USG application is used under
the GPL-3.0-only option offered by those source files. The complete GNU General
Public License version 3 is included in the project-level `LICENSE.txt` and in
every USG distribution package.

Source reference:

- Qt repository: <https://github.com/qt/qtbase>
- Qt source tag: `v6.9.3`
- Source directory: `src/plugins/sqldrivers/sqlite`
- Required Qt runtime and build version: `6.9.3`

The USG driver changes the plugin key to `QSQLCIPHER`, links SQLCipher instead
of ordinary SQLite, applies the encryption key during `open()`, verifies the
key before returning a usable connection and supports the additional
`QSQLCIPHER_COMPATIBILITY` and `QSQLCIPHER_RAW_KEY` connection options.

## SQLCipher

The driver embeds SQLCipher Community Edition:

- Upstream project: <https://github.com/sqlcipher/sqlcipher>
- Release tag: `v4.19.0`
- Pinned commit: `c4b275a47932888216bade83aff2bbc73df0ff85`
- License: BSD 3-Clause
- Copyright: Copyright (c) 2025, ZETETIC LLC

The complete applicable SQLCipher license is preserved in
`licenses/SQLCipher-BSD-3-Clause.txt` and must be included in every binary
package containing `libqsqlcipher.so` (Linux) or `qsqlcipher.dll` (Windows).

## OpenSSL

SQLCipher uses OpenSSL 3 as its cryptographic provider. The resulting Linux
plugin requires `libcrypto.so.3`. The Linux release runtime is built from the
pinned OpenSSL 3.5.9 LTS source at commit
`45e844fa2a14ec92d146bd8f5778ac130b6625fb`. OpenSSL 3 is licensed under
Apache License 2.0. The complete license is preserved in
`licenses/OpenSSL-Apache-2.0.txt` and must be included with every package that
distributes the OpenSSL runtime.

The Windows plugin `qsqlcipher.dll` is built with MSVC 2022 x64 and links
OpenSSL statically (`/MD` runtime), so no separate OpenSSL DLL is required.
The Windows release currently uses OpenSSL 3.5.2 from the `openssl` port of
vcpkg at the pinned commit `4334d8b4c8916018600212ab4dd4bbdc343065d1`
(triplet `x64-windows-static-md`), the same static library linked into
`USG.exe`. Aligning Windows with the Linux OpenSSL 3.5.9 source build is a
planned follow-up.

## Corresponding driver source

The preferred source form of the modified Qt driver is preserved in
`third_party/qsqlcipher/source`. It includes the driver sources, CMake build
definition, test program and deterministic build scripts (`build.sh` for
Linux, `build.ps1` for Windows). The build script
checks both the SQLCipher tag and its exact commit before compiling.

The reviewed original source archive was:

- File: `qsqlcipher-qt6.9.3.tar.gz`
- SHA-256: `3494c64f2ab746f3b497697805c181e9cb53fb73e8e46a8c63e1ce0d9cea1973`

The source directory above was extracted from that reviewed archive. SQLCipher
itself is fetched from its pinned upstream commit, generates the SQLCipher
amalgamation and is compiled into the plugin with Qt 6.9.3.

Before publishing an USG release containing QSQLCIPHER:

1. preserve this notice and all applicable license texts in the package;
2. preserve the corresponding driver source included with the package;
3. build the plugin from the pinned sources with the same Qt version and ABI
   as USG;
4. verify the packaged plugin by creating and reopening an encrypted database;
5. update this inventory whenever Qt, SQLCipher or OpenSSL changes.

This inventory documents the dependency and its redistribution requirements;
it is not legal advice.
