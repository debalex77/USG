#!/usr/bin/env bash
# Compilează SQLCipher + pluginul QSQLCIPHER pentru Qt 6.9.3 (Linux)
#
# Utilizare:
#   ./build.sh [cale_kit_Qt]            implicit: ~/Qt/6.9.3/gcc_64
#
# Variabile opționale:
#   SQLCIPHER_TAG=v4.19.0   versiunea SQLCipher
#   SQLCIPHER_COMMIT=...     commitul exact corespunzător tagului
#   INSTALL=1                    instalează pluginul în <kit>/plugins/sqldrivers
#   QSQLCIPHER_BUILD_JOBS=<număr> numărul de procese paralele (implicit: 75% CPU)
set -euo pipefail

QT_DIR="${1:-$HOME/Qt/6.9.3/gcc_64}"
SQLCIPHER_TAG="${SQLCIPHER_TAG:-v4.19.0}"
SQLCIPHER_COMMIT="${SQLCIPHER_COMMIT:-c4b275a47932888216bade83aff2bbc73df0ff85}"
OPENSSL_ROOT_DIR="${OPENSSL_ROOT_DIR:-}"
ROOT="$(cd "$(dirname "$0")" && pwd)"
WORK="$ROOT/_deps"
JOBS="${QSQLCIPHER_BUILD_JOBS:-$(( ($(nproc) * 3 + 3) / 4 ))}"

[[ -x "$QT_DIR/bin/qt-cmake" ]] || { echo "Nu găsesc $QT_DIR/bin/qt-cmake"; exit 1; }
for t in git make cc c++ cmake ninja pkg-config; do
    command -v "$t" >/dev/null || { echo "Lipsește: $t  (sudo apt install build-essential cmake ninja-build git pkg-config)"; exit 1; }
done
if [[ -n "$OPENSSL_ROOT_DIR" ]]; then
    [[ -f "$OPENSSL_ROOT_DIR/include/openssl/evp.h" ]] || {
        echo "Nu găsesc headerele OpenSSL în $OPENSSL_ROOT_DIR/include"
        exit 1
    }
else
    [[ -f /usr/include/openssl/evp.h ]] || { echo "Lipsește OpenSSL: sudo apt install libssl-dev"; exit 1; }
fi

# 1. SQLCipher -> amalgamare sqlite3.c / sqlite3.h
mkdir -p "$WORK"
if [[ ! -d "$WORK/sqlcipher" ]]; then
    git clone --depth 1 --branch "$SQLCIPHER_TAG" https://github.com/sqlcipher/sqlcipher.git "$WORK/sqlcipher"
fi
ACTUAL_SQLCIPHER_COMMIT="$(git -C "$WORK/sqlcipher" rev-parse HEAD)"
if [[ "$ACTUAL_SQLCIPHER_COMMIT" != "$SQLCIPHER_COMMIT" ]]; then
    echo "Commit SQLCipher neașteptat: $ACTUAL_SQLCIPHER_COMMIT (așteptat: $SQLCIPHER_COMMIT)"
    exit 1
fi
if [[ ! -f "$WORK/sqlcipher/sqlite3.c" ]]; then
    ( cd "$WORK/sqlcipher"
      ./configure --with-tempstore=yes \
          CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown" \
          LDFLAGS="-lcrypto"
      make -j"$JOBS" sqlite3.c )
fi

# 2. Pluginul
CMAKE_OPTIONS=(
    -DCMAKE_BUILD_TYPE=Release
    -DSQLCIPHER_AMALGAMATION_DIR="$WORK/sqlcipher"
)
if [[ -n "$OPENSSL_ROOT_DIR" ]]; then
    CMAKE_OPTIONS+=(
        -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR"
        -DOPENSSL_USE_STATIC_LIBS=FALSE
    )
fi
"$QT_DIR/bin/qt-cmake" -S "$ROOT" -B "$ROOT/build" -G Ninja "${CMAKE_OPTIONS[@]}"
cmake --build "$ROOT/build" --parallel "$JOBS"

# 3. Test (rulează pluginul direct din directorul de build)
if [[ -n "$OPENSSL_ROOT_DIR" ]]; then
    (
        cd "$ROOT/build"
        LD_LIBRARY_PATH="$OPENSSL_ROOT_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
            ./qsqlcipher_test
    )
else
    ( cd "$ROOT/build" && ./qsqlcipher_test )
fi

# 4. Instalare
if [[ "${INSTALL:-0}" == "1" ]]; then
    cmake --install "$ROOT/build"
    echo "Instalat în: $(ls "$QT_DIR"/plugins/sqldrivers/libqsqlcipher.so)"
else
    echo "Plugin: $ROOT/build/plugins/sqldrivers/libqsqlcipher.so"
    echo "Pentru instalare în kit:  INSTALL=1 $0 $QT_DIR"
fi
