# QSQLCIPHER – driver SQL criptat pentru Qt 6.9.3 (Linux, Windows)

Plugin separat `QSQLCIPHER`, derivat din driverul oficial `qsqlite` din Qt 6.9.3 (tag `v6.9.3`), cu SQLCipher compilat în interior. Poate coexista cu `QSQLITE`.

## Cerințe

```bash
sudo apt install build-essential cmake ninja-build git pkg-config perl
```

Ai nevoie și de Qt 6.9.3 pentru `gcc_64`, instalat din Online Installer. Headerele private sunt incluse implicit.

Pachetele USG folosesc copia privată OpenSSL 3.5.9 pregătită din sursa fixată:

```bash
../../../build_scripts/build_openssl
```

## Compilare rapidă

```bash
OPENSSL_ROOT_DIR="$PWD/../../../3rdparty/openssl" \
    ./build.sh ~/Qt/6.9.3/gcc_64
OPENSSL_ROOT_DIR="$PWD/../../../3rdparty/openssl" INSTALL=1 \
    ./build.sh ~/Qt/6.9.3/gcc_64
```

Scriptul face următorii pași:
1. Descarcă SQLCipher `v4.19.0`, commit
   `c4b275a47932888216bade83aff2bbc73df0ff85`.
2. Generează amalgamarea `sqlite3.c`.
3. Compilează pluginul.
4. Rulează testul.

Rezultatul este `build/plugins/sqldrivers/libqsqlcipher.so`. La instalare, fișierul ajunge în `<kit>/plugins/sqldrivers/`.

## Compilare pe Windows (MSVC 2022 x64)

Din „x64 Native Tools Command Prompt for VS 2022”, cu PowerShell 7 (`pwsh`),
Qt 6.9.3 `msvc2022_64`, Git, CMake și Ninja. OpenSSL trebuie să fie static,
cu runtime `/MD` (`include\openssl`, `lib\libcrypto.lib`); CI-ul folosește
`openssl:x64-windows-static-md` din vcpkg, copiat în `3rdparty\openssl`.

```powershell
$env:OPENSSL_ROOT_DIR = "$PWD\..\..\..\3rdparty\openssl"
pwsh -File .\build.ps1 C:\Qt\6.9.3\msvc2022_64
$env:INSTALL = '1'; pwsh -File .\build.ps1 C:\Qt\6.9.3\msvc2022_64
```

Amalgamarea se generează cu `nmake /f Makefile.msc sqlite3.c`; Makefile-ul
folosește `jimsh` inclus în sursa SQLCipher, deci Tcl nu este necesar.
Scriptul verifică, cu `dumpbin`, că `qsqlcipher.dll` nu depinde de un DLL
OpenSSL. Rezultatul este `build\plugins\sqldrivers\qsqlcipher.dll`.

## Compilare manuală (fără script)

```bash
git clone --depth 1 --branch v4.19.0 https://github.com/sqlcipher/sqlcipher.git
cd sqlcipher
test "$(git rev-parse HEAD)" = "c4b275a47932888216bade83aff2bbc73df0ff85"
./configure --with-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown" LDFLAGS="-lcrypto"
make sqlite3.c
cd ..

~/Qt/6.9.3/gcc_64/bin/qt-cmake -S qsqlcipher -B build -G Ninja \
    -DSQLCIPHER_AMALGAMATION_DIR=$PWD/sqlcipher
cmake --build build
cmake --install build
```

Ca alternativă, poți folosi `libsqlcipher-dev` din sistem: omite `SQLCIPHER_AMALGAMATION_DIR`, iar biblioteca se găsește prin pkg-config. Versiunea din apt este însă mai veche.

## Utilizare

```cpp
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLCIPHER");
db.setDatabaseName("date.db");
db.setPassword("parola mea");      // cheia SQLCipher
if (!db.open())
    qWarning() << db.lastError().text();   // cheie greșită -> open() eșuează
```

- **Cheia:** se dă prin `setPassword()`. Driverul o aplică imediat după deschidere și o verifică. Cu o cheie greșită, `open()` întoarce `false`.
- **Fără parolă:** baza de date se deschide necriptată, ca SQLite obișnuit.
- **Opțiuni** (`db.setConnectOptions("...;...")`):
  - `QSQLCIPHER_COMPATIBILITY=3`: deschide baze create cu SQLCipher 3.x.
  - `QSQLCIPHER_RAW_KEY`: parola este o cheie brută în hex (64 de caractere), fără derivare PBKDF2.
  - Toate opțiunile `QSQLITE_*` ale driverului original funcționează în continuare (`QSQLITE_BUSY_TIMEOUT`, `QSQLITE_OPEN_READONLY` etc.).
- **Schimbarea cheii:** `QSqlQuery q(db); q.exec("PRAGMA rekey = 'parola noua'");`
- **Criptarea unei baze existente necriptate:** folosește `ATTACH ... KEY` și `sqlcipher_export()`. Detalii în documentația SQLCipher.

## Distribuire

- Copiază `libqsqlcipher.so` în `plugins/sqldrivers/` al aplicației. `linuxdeploy`/`linuxdeployqt` îl preiau dacă este în kit.
- Pachetul Linux trebuie să includă `libcrypto.so.3` și `libssl.so.3` din
  copia privată OpenSSL 3.5.9. Nu este necesară modificarea OpenSSL instalat
  în sistemul utilizatorului.
- Pe Windows, `qsqlcipher.dll` se copiază în `sqldrivers\` lângă `USG.exe`;
  OpenSSL este inclus static, deci nu se distribuie DLL-uri OpenSSL pentru plugin.
- Simbolurile `sqlite3_*` sunt ascunse în plugin, deci nu intră în conflict cu `qsqlite` sau cu alt SQLite din proces.

## Licențe

- Codul driverului derivă din Qt (LGPL-3.0 / GPL), deci modificările lui intră sub aceeași licență.
- SQLCipher Community Edition este licențiat BSD.
- OpenSSL 3 este licențiat Apache-2.0.

## Ce s-a schimbat față de qsqlite

Sursele au fost generate din `qtbase/src/plugins/sqldrivers/sqlite` (v6.9.3), cu următoarele modificări:
- Clasele au fost redenumite `QSQLite*` → `QSQLCipher*`, cheia pluginului a devenit `QSQLCIPHER`, iar categoria de log a devenit `qt.sql.sqlcipher`.
- În `open()` (`src/qsql_sqlcipher.cpp`) au fost adăugate `sqlite3_key()`, verificarea cheii și cele două opțiuni `QSQLCIPHER_*`.

La actualizarea Qt (de ex. 6.10), regenerează sursele din noul `qtbase` și reaplică blocul din `open()`. Începând cu Qt 6.10, `SqlPrivate` trebuie găsit explicit, iar `CMakeLists.txt` tratează deja acest caz.
