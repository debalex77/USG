#!/bin/bash

#-----------------------------------------------------
# Determinam variabile dosarelor
RELEASE_DIR="build/Desktop_Qt_6_5_3_GCC_64bit-Release"
PREBUILD_DIR="build/prebuild_project"
BUILD_DIR="build/build_project"
QT_LIB_DIR="$HOME/Qt/6.5.3/gcc_64/lib"
QT_PLUGINS_DIR="$HOME/Qt/6.5.3/gcc_64/plugins"
QT_QIF_DIR="$HOME/Qt/Tools/QtInstallerFramework/4.8/bin"

#-----------------------------------------------------
# 1. Verificam daca sunt directorii
if [ ! -d "$PREBUILD_DIR" ]; then
    mkdir build/prebuild_project
    echo "A fost creat dosarul 'prebuild_project' ..."
else
    rm -r build/prebuild_project/* 
    echo "Se sterg fisierele din dosarul 'prebuild_project' ..."
fi

if [ ! -d "$BUILD_DIR" ]; then
    mkdir build/build_project
    echo "A fost creat dosarul 'build_project' ..."
else
    rm -r build/build_project/*
    echo "Se sterg fisierele din dosarul 'build_project' ..."
fi

#-----------------------------------------------------
# 2. Copierea fisierelor
cp "$RELEASE_DIR"/* "$PREBUILD_DIR"
cp -r icons "$PREBUILD_DIR"
cp -r templets "$PREBUILD_DIR"
cp -r LimeReport "$PREBUILD_DIR"
cp -r openssl "$PREBUILD_DIR"
cp -f version.txt "$PREBUILD_DIR"
echo "Sunt copiate fisierele pentru 'prebuild' ..."

#-----------------------------------------------------
# 3. Stergerea fisierelor 'moc_'
echo "Se sterg fișierele 'moc_' ..."
find "$PREBUILD_DIR" -type f -name "moc_*" -delete

#-----------------------------------------------------
# 4. Procesarea cqtdeployer
echo "-------------------------------------------------------------------------"
echo "Initierea procesarii cqtdeployer ..."

cqtdeployer clear -bin "$PREBUILD_DIR"/USG -qmake qmake -libDir $QT_LIB_DIR -extraData LimeReport,templets,icons -targetDir $BUILD_DIR

if [ ! -d "$BUILD_DIR"/plugins ]; then
    echo "Eroare de executare a cqtdeployer ... "
    exit 1
fi

echo "Finalizarea procesarii cqtdeployer ..."
echo "-------------------------------------------------------------------------"

#-----------------------------------------------------
# 5. Copierea librariilor
# -- openssl
cp -f openssl/libcrypto.so.3 "$BUILD_DIR"/lib/libcrypto.so.3
cp -f openssl/libssl.so.3 "$BUILD_DIR"/lib/libssl.so.3
echo "Sunt copiate librariile 'openssl' ..."
# -- lib LimeReport
cp -f LimeReport/release/liblimereport.so "$BUILD_DIR"/lib/liblimereport.so
cp -f LimeReport/release/liblimereport.so.1 "$BUILD_DIR"/lib/liblimereport.so.1
cp -f LimeReport/release/liblimereport.so.1.0 "$BUILD_DIR"/lib/liblimereport.so.1.0
cp -f LimeReport/release/liblimereport.so.1.0.0 "$BUILD_DIR"/lib/liblimereport.so.1.0.0
cp -f LimeReport/release/libQtZint.so "$BUILD_DIR"/lib/libQtZint.so
cp -f LimeReport/debug/liblimereportd.so "$BUILD_DIR"/lib/liblimereportd.so
cp -f LimeReport/debug/liblimereportd.so.1 "$BUILD_DIR"/lib/liblimereportd.so.1
cp -f LimeReport/debug/liblimereportd.so.1.0 "$BUILD_DIR"/lib/liblimereportd.so.1.0
cp -f LimeReport/debug/liblimereportd.so.1.0.0 "$BUILD_DIR"/lib/liblimereportd.so.1.0.0
cp -f LimeReport/debug/libQtZintd.so "$BUILD_DIR"/lib/libQtZintd.so
echo "Sunt copiate librariile 'LimeReport' ..."
# -- libQt6Designer
cp -f "$QT_LIB_DIR"/libQt6Designer.so.6 "$BUILD_DIR"/lib/libQt6Designer.so.6
cp -f "$QT_LIB_DIR"/libQt6Designer.so.6.5.3 "$BUILD_DIR"/lib/libQt6Designer.so.6.5.3
# -- libQt6DesignerComponents
cp -f "$QT_LIB_DIR"/libQt6DesignerComponents.so.6 "$BUILD_DIR"/lib/libQt6DesignerComponents.so.6
cp -f "$QT_LIB_DIR"/libQt6DesignerComponents.so.6.5.3 "$BUILD_DIR"/lib/libQt6DesignerComponents.so.6.5.3
# -- libQt6OpenGLWidgets
cp -f "$QT_LIB_DIR"/libQt6OpenGLWidgets.so.6 "$BUILD_DIR"/lib/libQt6OpenGLWidgets.so.6
cp -f "$QT_LIB_DIR"/libQt6OpenGLWidgets.so.6.5.3 "$BUILD_DIR"/lib/libQt6OpenGLWidgets.so.6.5.3
# -- libQt6PrintSupport
cp -f "$QT_LIB_DIR"/libQt6PrintSupport.so.6 "$BUILD_DIR"/lib/libQt6PrintSupport.so.6
cp -f "$QT_LIB_DIR"/libQt6PrintSupport.so.6.5.3 "$BUILD_DIR"/lib/libQt6PrintSupport.so.6.5.3
# -- libQt6UiTools
cp -f "$QT_LIB_DIR"/libQt6UiTools.so.6 "$BUILD_DIR"/lib/libQt6UiTools.so.6
cp -f "$QT_LIB_DIR"/libQt6UiTools.so.6.5.3 "$BUILD_DIR"/lib/libQt6UiTools.so.6.5.3

if [ ! -d "$BUILD_DIR"/plugins/printsupport ]; then
    mkdir "$BUILD_DIR"/plugins/printsupport
fi
# -- printsupport
cp -f "$QT_PLUGINS_DIR"/printsupport/libcupsprintersupport.so "$BUILD_DIR"/plugins/printsupport/libcupsprintersupport.so
echo "Sunt copiate alte librarii necesare ..."
echo "Fisierele proiectului sunt gata pentru formarea pachetului de instalare ..."
echo "-------------------------------------------------------------------------"

INSTALLER_DIR="build/usg_installer"
if [ ! -d $INSTALLER_DIR ]; then
    echo "Se creeaza dosarul 'usg_installer' ..."
    mkdir $INSTALLER_DIR
else
    echo "Se sterg fisierele din dosarul 'usg_installer' ..."
    rm -r $INSTALLER_DIR
    echo "Se creeaza dosarul 'usg_installer' ..."
    mkdir $INSTALLER_DIR
fi

if [ ! -d "$INSTALLER_DIR"/config ]; then
    mkdir "$INSTALLER_DIR"/config
fi

echo "Se copie fisierele pentru formarea pachetului de instalare ..."
cp -rf "installer/linux/config" "$INSTALLER_DIR"/
cp -rf "installer/linux/packages" "$INSTALLER_DIR"/
cp -rf "$BUILD_DIR"/* "$INSTALLER_DIR"/packages/com.alovada.usg/data

VERSION=$(cat version.txt)
ARCH="Linux_amd64"
FILENAME="USG_v${VERSION}_${ARCH}.run"
FILENAME_SHA256="USG_v${VERSION}_${ARCH}.sha256"

rm -f build/$FILENAME
rm -f build/$FILENAME_SHA256

echo "-------------------------------------------------------------------------"
echo "Se creeaza pachetul de instalare ..."
"$QT_QIF_DIR"/binarycreator -c "$INSTALLER_DIR"/config/config.xml -p "$INSTALLER_DIR"/packages build/$FILENAME
echo "Fisierul de instalare creat cu succes !!!"

echo "Se calculeaza suma SHA256 a fisierului de instalare ..."
SHA256_SUM_TXT=$(sha256sum build/$FILENAME | awk '{print $1}')
echo "$SHA256_SUM_TXT" > build/$FILENAME_SHA256

echo "-------------------------------------------------------------------------"
read -p "Doriți să lansati fisierul de instalare? (y/n): " response
if [[ "$response" == "y" || "$response" == "Y" ]]; then
    echo "Se lanseaza fisierul de instalare..."
    ./build/USG_v$VERSION.run
fi

echo "-------------------------------------------------------------------------"

echo "Initierea crearii pachetului .deb ..."
BUILD_DEB_DIR="build/build_deb"
FILENAME_DEB="USG_v${VERSION}_${ARCH}.deb"

if [ ! -d $BUILD_DEB_DIR ]; then
    echo "Crearea dosarului 'build_deb' ..."
    mkdir $BUILD_DEB_DIR
else
    rm -rf $BUILD_DEB_DIR
    echo "Se sterg fisierele din dosarul 'build_deb' ..."
fi

echo "Se copie fisiere in dosar 'build_deb' ..."
mkdir $BUILD_DEB_DIR/DEBIAN
cp -f debian/control $BUILD_DEB_DIR/DEBIAN
cp -f debian/postinst $BUILD_DEB_DIR/DEBIAN
cp -f debian/preinst $BUILD_DEB_DIR/DEBIAN
cp -f debian/prerm $BUILD_DEB_DIR/DEBIAN
cp -rf debian/usr $BUILD_DEB_DIR

chmod 755 $BUILD_DEB_DIR/DEBIAN
chmod 644 $BUILD_DEB_DIR/DEBIAN/control
chmod 775 $BUILD_DEB_DIR/DEBIAN/preinst
chmod 775 $BUILD_DEB_DIR/DEBIAN/postinst
chmod 775 $BUILD_DEB_DIR/DEBIAN/prerm

mkdir $BUILD_DEB_DIR/opt
mkdir $BUILD_DEB_DIR/opt/USG
cp -rf "$BUILD_DIR"/* $BUILD_DEB_DIR/opt/USG/

echo "Se creeaza pachetul de instalare .deb ..."
dpkg-deb --build $BUILD_DEB_DIR

mv build/build_deb.deb build/$FILENAME_DEB
echo "Se calculeaza suma SHA256 a fisierului .deb ..."
SHA256_SUM_DEB=$(sha256sum build/$FILENAME_DEB | awk '{print $1}')
echo "$SHA256_SUM_DEB" > build/$FILENAME_DEB_SHA256

echo "-------------------------------------------------------------------------"
echo "Se elimina fisiere si directoriile: prebuild_project, build_project, usg_installer ..."
rm -r $INSTALLER_DIR
rm -r build/prebuild_project
rm -r build/build_project
rm -r $BUILD_DEB_DIR
