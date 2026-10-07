TRANSLATIONS += \
    $$USG_ROOT/translate/USG_ro_RO.ts \
    $$USG_ROOT/translate/USG_ru_RU.ts

CONFIG += lrelease
CONFIG += embed_translations

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    $$PWD/../../.github/workflows/build-windows.yml \
    $$PWD/../../.github/workflows/release-windows.yml \
    $$PWD/../../build_scripts/build_demo \
    $$PWD/../../build_scripts/build_openssl \
    $$PWD/../../installer/linux-demo/README.md \
    $$PWD/../../installer/linux-demo/config/config.xml \
    $$PWD/../../installer/linux-demo/packages/com.alovada.usg.demo/meta/installscript.qs \
    $$PWD/../../installer/linux-demo/packages/com.alovada.usg.demo/meta/package.xml \
    $$PWD/../../installer/linux-demo/runtime/README-DEMO.md \
    $$PWD/../../installer/linux-demo/runtime/USG-Demo.sh \
    $$PWD/../../installer/linux-demo/runtime/configure-demo-profile.sh \
    $$PWD/../../installer/linux-demo/tools/verify_demo_database.py \
    $$USG_ROOT/LICENSE.txt \
    $$USG_ROOT/README.md \
    $$USG_ROOT/README-RO.md \
    $$USG_ROOT/docs/sqlcipher.md \
    $$USG_ROOT/tests/sqlcipher/sqlcipher.pro \
    $$USG_ROOT/tests/sqlcipher/tst_sqlcipher.cpp \
    $$USG_ROOT/tests/sqlcipherkeyprompt/sqlcipherkeyprompt.pro \
    $$USG_ROOT/tests/sqlcipherkeyprompt/tst_sqlcipherkeyprompt.cpp \
    $$USG_ROOT/release.md \
    $$USG_ROOT/index.html \
    $$USG_ROOT/privacy.html \
    $$USG_ROOT/robots.txt \
    $$USG_ROOT/sitemap.xml \
    $$USG_ROOT/third_party/THIRD_PARTY_ICONS.md \
    $$USG_ROOT/third_party/LIMEREPORT.md \
    $$USG_ROOT/third_party/QSQLCIPHER.md \
    $$USG_ROOT/third_party/licenses/OpenSSL-Apache-2.0.txt \
    $$USG_ROOT/third_party/licenses/SQLCipher-BSD-3-Clause.txt \
    $$USG_ROOT/third_party/qsqlcipher/source/CMakeLists.txt \
    $$USG_ROOT/third_party/qsqlcipher/source/README.md \
    $$USG_ROOT/third_party/qsqlcipher/source/build.sh \
    $$USG_ROOT/third_party/qsqlcipher/source/build.ps1 \
    $$USG_ROOT/third_party/qsqlcipher/source/src/qsql_sqlcipher.cpp \
    $$USG_ROOT/third_party/qsqlcipher/source/src/qsql_sqlcipher_p.h \
    $$USG_ROOT/third_party/qsqlcipher/source/src/qsql_sqlcipher_vfs.cpp \
    $$USG_ROOT/third_party/qsqlcipher/source/src/qsql_sqlcipher_vfs_p.h \
    $$USG_ROOT/third_party/qsqlcipher/source/src/smain.cpp \
    $$USG_ROOT/third_party/qsqlcipher/source/src/sqlcipher.json \
    $$USG_ROOT/third_party/qsqlcipher/source/test/main.cpp \
    $$USG_ROOT/patches/limereport/1.7.23/0001-make-singleton-destruction-idempotent.patch \
    $$USG_ROOT/.github/workflows/build-linux.yml \
    $$USG_ROOT/.github/workflows/release-linux.yml \
    $$USG_ROOT/TODO.md \
    $$USG_ROOT/build_scripts/build_maosx \
    $$USG_ROOT/build_scripts/build_new \
    $$USG_ROOT/build_scripts/build_win.bat \
    $$USG_ROOT/build_scripts/write_sha256.ps1 \
    $$USG_ROOT/resources/fonts/freefontsdownload.txt \
    $$USG_ROOT/resources/fonts/www.freefontsdownload.net.url \
    $$USG_ROOT/translate/USG_ro_RO.qm \
    $$USG_ROOT/translate/USG_ru_RU.qm \
    $$USG_ROOT/build_scripts/debian/control \
    $$USG_ROOT/build_scripts/debian/postinst \
    $$USG_ROOT/build_scripts/debian/preinst \
    $$USG_ROOT/build_scripts/debian/prerm \
    $$USG_ROOT/build_scripts/debian/usr/share/applications/org.alovada.usg.desktop \
    $$USG_ROOT/build_scripts/debian/usr/share/doc/usg/changelog \
    $$USG_ROOT/build_scripts/debian/usr/share/doc/usg/changelog.Debian \
    $$USG_ROOT/build_scripts/debian/usr/share/doc/usg/copyright \
    $$USG_ROOT/build_scripts/debian/usr/share/metainfo/org.alovada.usg.metainfo.xml \
    $$USG_ROOT/installer/linux/config/config.xml \
    $$USG_ROOT/installer/linux/config/eco.png \
    $$USG_ROOT/installer/linux/config/eco_256x256.png \
    $$USG_ROOT/installer/linux/config/logo.png \
    $$USG_ROOT/installer/linux/config/style.qss \
    $$USG_ROOT/installer/linux/config/welcome.html \
    $$USG_ROOT/installer/linux/packages/com.alovada.usg/meta/installscript.qs \
    $$USG_ROOT/installer/linux/packages/com.alovada.usg/meta/license.txt \
    $$USG_ROOT/installer/linux/packages/com.alovada.usg/meta/package.xml \
    $$USG_ROOT/resources/styles/style_dark.qss \
    $$USG_ROOT/version.txt

RESOURCES += \
    $$USG_ROOT/installer/linux/config/installer.qrc \
    $$USG_ROOT/resources/resource.qrc
