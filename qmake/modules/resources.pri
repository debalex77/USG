TRANSLATIONS += \
    $$USG_ROOT/translate/USG_ro_RO.ts \
    $$USG_ROOT/translate/USG_ru_RU.ts

CONFIG += lrelease
CONFIG += embed_translations

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    $$USG_ROOT/LICENSE.txt \
    $$USG_ROOT/README.md \
    $$USG_ROOT/README-RO.md \
    $$USG_ROOT/release.md \
    $$USG_ROOT/index.html \
    $$USG_ROOT/privacy.html \
    $$USG_ROOT/robots.txt \
    $$USG_ROOT/sitemap.xml \
    $$USG_ROOT/third_party/THIRD_PARTY_ICONS.md \
    $$USG_ROOT/third_party/LIMEREPORT.md \
    $$USG_ROOT/patches/limereport/1.7.23/0001-make-singleton-destruction-idempotent.patch \
    $$USG_ROOT/.github/workflows/build-linux.yml \
    $$USG_ROOT/.github/workflows/release-linux.yml \
    $$USG_ROOT/TODO.md \
    $$USG_ROOT/build_scripts/build_maosx \
    $$USG_ROOT/build_scripts/build_new \
    $$USG_ROOT/build_scripts/build_win.bat \
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
