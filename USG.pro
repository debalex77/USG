QT       += core \
            gui \
            sql \
            widgets \
            network \
            webenginewidgets \
            printsupport \
            xml \
            multimedia \
            multimediawidgets \
            gui-private

#-----------------------------------------------------------------------
#------ Verifică dacă versiunea de Qt este cel puțin 6.5.3
greaterThan(QT_MAJOR_VERSION, 6) {
    message("Qt version is sufficient: "$$QT_MAJOR_VERSION"."$$QT_MINOR_VERSION"."$$QT_PATCH_VERSION"")
} else:equals(QT_MAJOR_VERSION, 6) {
    greaterThan(QT_MINOR_VERSION, 5) {
        message("Qt version is sufficient: "$$QT_MAJOR_VERSION"."$$QT_MINOR_VERSION"."$$QT_PATCH_VERSION"")
    } else:equals(QT_MINOR_VERSION, 5) {
        greaterThan(QT_PATCH_VERSION, 2) {
            message("Qt version is sufficient: "$$QT_MAJOR_VERSION"."$$QT_MINOR_VERSION"."$$QT_PATCH_VERSION"")
        } else {
            error("This project requires Qt version 6.5.3 or higher.")
        }
    }
} else {
    error("This project requires Qt version 6.5.3 or higher.")
}


#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_QML_DEBUG_NO_WARNING
CONFIG  -= qml_debug

#-----------------------------------------------------------------------
#------ INFO APP

# Definim componentele versiunii
USG_VERSION_MAJOR   = 3
USG_VERSION_MINOR   = 0
USG_VERSION_RELEASE = 2
USG_VERSION_FULL    = ""$$USG_VERSION_MAJOR"."$$USG_VERSION_MINOR"."$$USG_VERSION_RELEASE""
VERSION             = "$$USG_VERSION_MAJOR"."$$USG_VERSION_MINOR"."$$USG_VERSION_RELEASE"
DEFINES += USG_VERSION_MAJOR=$$USG_VERSION_MAJOR
DEFINES += USG_VERSION_MINOR=$$USG_VERSION_MINOR
DEFINES += USG_VERSION_RELEASE=$$USG_VERSION_RELEASE
DEFINES += USG_VERSION_FULL=\\\"$$USG_VERSION_FULL\\\"

# email companiei
DEFINES += USG_COMPANY_EMAIL="\\\"alovada.med@gmail.com\\\""

# mesaj de versiune a aplicatiei
message("VERSION: $$USG_VERSION_FULL")

# Denumirea companiei, product, desription
QMAKE_TARGET_COMPANY     = SC 'Alovada-Med' SRL
QMAKE_TARGET_PRODUCT     = USG project
QMAKE_TARGET_DESCRIPTION = Evidenta examinarilor ecografice
QMAKE_TARGET_COPYRIGHT   = Codreanu Alexandru
RC_ICONS = img/eco_512x512.ico
ICON = img/eco_512x512.icns

#-----------------------------------------------------------------------
#------ CONFIG APP

CONFIG += c++17

#-----------------------------------------------------------------------
#------ SOURCES, HEADERS, FORMS, TRANSLATIONS

mac {
    SOURCES += AppDelegate.mm
}

SOURCES += \
    catalogs/asistanttipapp.cpp \
    catalogs/catcontracts.cpp \
    catalogs/catforsqltablemodel.cpp \
    catalogs/catgeneral.cpp \
    catalogs/catorganizations.cpp \
    catalogs/catusers.cpp \
    catalogs/chooseformprint.cpp \
    catalogs/customperiod.cpp \
    catalogs/groupinvestigation.cpp \
    catalogs/groupinvestigationlist.cpp \
    catalogs/listform.cpp \
    catalogs/normograms.cpp \
    catalogs/patienthistory.cpp \
    common/datapercentage.cpp \
    customs/custommessage.cpp \
    customs/lineeditcustom.cpp \
    customs/searchlineedit.cpp \
    data/about.cpp \
    data/appsettings.cpp \
    data/authorizationuser.cpp \
    data/customdialoginvestig.cpp \
    data/database.cpp \
    data/databaseselection.cpp \
    data/downloader.cpp \
    data/downloaderversion.cpp \
    data/globals.cpp \
    data/initlaunch.cpp \
    data/loggingcategories.cpp \
    data/mainwindow.cpp \
    data/mdiareacontainer.cpp \
    data/popup.cpp \
    data/reports.cpp \
    data/updatereleasesapp.cpp \
    data/userpreferences.cpp \
    delegates/checkboxdelegate.cpp \
    delegates/combodelegate.cpp \
    delegates/doublespinboxdelegate.cpp \
    docs/choicecolumns.cpp \
    docs/docappointmentspatients.cpp \
    docs/docorderecho.cpp \
    docs/docpricing.cpp \
    docs/docreportecho.cpp \
    docs/listdoc.cpp \
    docs/listdocreportorder.cpp \
    infowindow.cpp \
    main.cpp \
    models/basecombomodel.cpp \
    models/basesortfilterproxymodel.cpp \
    models/basesqlquerymodel.cpp \
    models/basesqltablemodel.cpp \
    models/paginatedsqlmodel.cpp \
    models/registrationtablemodel.cpp \
    models/treeitem.cpp \
    models/treemodel.cpp \
    models/variantmaptablemodel.cpp

HEADERS += \
    catalogs/asistanttipapp.h \
    catalogs/catcontracts.h \
    catalogs/catforsqltablemodel.h \
    catalogs/catgeneral.h \
    catalogs/catorganizations.h \
    catalogs/catusers.h \
    catalogs/chooseformprint.h \
    catalogs/customperiod.h \
    catalogs/groupinvestigation.h \
    catalogs/groupinvestigationlist.h \
    catalogs/listform.h \
    catalogs/normograms.h \
    catalogs/patienthistory.h \
    common/datapercentage.h \
    customs/custommessage.h \
    customs/lineeditcustom.h \
    customs/searchlineedit.h \
    data/about.h \
    data/appsettings.h \
    data/authorizationuser.h \
    data/customdialoginvestig.h \
    data/database.h \
    data/databaseselection.h \
    data/downloader.h \
    data/downloaderversion.h \
    data/enums.h \
    data/globals.h \
    data/initlaunch.h \
    data/loggingcategories.h \
    data/mainwindow.h \
    data/mdiareacontainer.h \
    data/popup.h \
    data/reports.h \
    data/updatereleasesapp.h \
    data/userpreferences.h \
    data/version.h \
    delegates/checkboxdelegate.h \
    delegates/combodelegate.h \
    delegates/doublespinboxdelegate.h \
    docs/choicecolumns.h \
    docs/docappointmentspatients.h \
    docs/docorderecho.h \
    docs/docpricing.h \
    docs/docreportecho.h \
    docs/listdoc.h \
    docs/listdocreportorder.h \
    infowindow.h \
    models/basecombomodel.h \
    models/basesortfilterproxymodel.h \
    models/basesqlquerymodel.h \
    models/basesqltablemodel.h \
    models/paginatedsqlmodel.h \
    models/registrationtablemodel.h \
    models/treeitem.h \
    models/treemodel.h \
    models/variantmaptablemodel.h

win32 {
# pentru eroarea numai pe Windows:
# - error: LNK2019: unresolved external symbol "public: void __cdecl  LimeReport::ICallbackDatasource::getCallbackData etc."
# - error: LNK2001: unresolved external symbol "public: static struct QMetaObject const LimeReport::PreviewReportWidget::staticMetaObject etc."
# - error: LNK2001: unresolved external symbol "public: static struct QMetaObject const LimeReport::ReportEngine::staticMetaObject etc. etc."
# ... problema in LimeReport cu conectarile:
#       exemplu - connect(m_report, QOverload<int>::of(&LimeReport::ReportEngine::renderPageFinished), this, QOverload<int>::of(&Reports::renderPageFinished));
    HEADERS += \
    LimeReport/include/lrcallbackdatasourceintf.h \
    LimeReport/include/lrpreviewreportwidget.h \
    LimeReport/include/lrreportengine.h
}

FORMS += \
    catalogs/asistanttipapp.ui \
    catalogs/catcontracts.ui \
    catalogs/catforsqltablemodel.ui \
    catalogs/catgeneral.ui \
    catalogs/catorganizations.ui \
    catalogs/catusers.ui \
    catalogs/chooseformprint.ui \
    catalogs/customperiod.ui \
    catalogs/groupinvestigation.ui \
    catalogs/groupinvestigationlist.ui \
    catalogs/listform.ui \
    catalogs/normograms.ui \
    catalogs/patienthistory.ui \
    customs/custommessage.ui \
    data/about.ui \
    data/appsettings.ui \
    data/authorizationuser.ui \
    data/databaseselection.ui \
    data/initlaunch.ui \
    data/mainwindow.ui \
    data/reports.ui \
    data/userpreferences.ui \
    docs/docappointmentspatients.ui \
    docs/docorderecho.ui \
    docs/docpricing.ui \
    docs/docreportecho.ui \
    docs/listdoc.ui \
    docs/listdocreportorder.ui \
    infowindow.ui

TRANSLATIONS += \
    translate/USG_ro_RO.ts \
    translate/USG_ru_RU.ts

CONFIG += lrelease
CONFIG += embed_translations
CONFIG += use_lld_linker

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Fonts/Cantarell Bold.ttf \
    Fonts/Cantarell-Bold.otf \
    Fonts/Cantarell-BoldOblique.ttf \
    Fonts/Cantarell-Oblique.ttf \
    Fonts/Cantarell-Regular.ttf \
    Fonts/freefontsdownload.txt \
    Fonts/www.freefontsdownload.net.url \
    USG_ro_RO.qm \
    USG_ru_RU.qm \
    build_project.sh \
    debian/control \
    debian/postinst \
    debian/preinst \
    debian/prerm \
    debian/usr/share/applications/org.alovada.usg.desktop \
    debian/usr/share/doc/usg/changelog \
    debian/usr/share/doc/usg/changelog.Debian \
    debian/usr/share/doc/usg/copyright \
    debian/usr/share/metainfo/org.alovada.usg.metainfo.xml \
    installer/linux/config/config.xml \
    installer/linux/config/eco.png \
    installer/linux/config/eco_256x256.png \
    installer/linux/config/logo.png \
    installer/linux/config/style.qss \
    installer/linux/config/welcome.html \
    installer/linux/packages/com.alovada.usg/data/org.alovada.usg.desktop \
    installer/linux/packages/com.alovada.usg/meta/installscript.qs \
    installer/linux/packages/com.alovada.usg/meta/license.txt \
    installer/linux/packages/com.alovada.usg/meta/package.xml \
    styles/style_dark.qss \
    version.txt

RESOURCES += \
    installer/linux/config/installer.qrc \
    resource.qrc

macx{
    CONFIG += app_bundle
}

#----------------------------------------------------------------------------------------
#---------------------------------- LIMEREPORT ------------------------------------------

INCLUDEPATH += $$PWD/LimeReport/include
DEPENDPATH += $$PWD/LimeReport/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/LimeReport/release/ -llimereport
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/LimeReport/debug/ -llimereportd
else:unix:!macx: LIBS += -L$$PWD/LimeReport/release/ -llimereport
else:unix:!macx: LIBS += -L$$PWD/LimeReport/debug/ -llimereportd

win32:CONFIG(release, release|debug): LIBS += -L$$PWD/LimeReport/release/ -lQtZint
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/LimeReport/debug/ -lQtZintd
else:unix:!macx: LIBS += -L$$PWD/LimeReport/release/ -lQtZint
else:unix:!macx: LIBS += -L$$PWD/LimeReport/debug/ -lQtZintd

unix:{

    #-------------------------------------------
    macx: LIBS += -L$$PWD/LimeReport/debug/ -llimereportd
    macx: LIBS += -L$$PWD/LimeReport/debug/ -lQtZintd

    INCLUDEPATH += $$PWD/LimeReport/debug
    DEPENDPATH += $$PWD/LimeReport/debug

    #-------------------------------------------
    macx: LIBS += -L$$PWD/LimeReport/release/ -llimereport
    macx: LIBS += -L$$PWD/LimeReport/release/ -lQtZint

    INCLUDEPATH += $$PWD/LimeReport/release
    DEPENDPATH += $$PWD/LimeReport/release

    linux{
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/LimeReport/debug
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/../LimeReport/debug
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/LimeReport/release
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/../LimeReport/release
        QMAKE_LFLAGS_RPATH += #. .. ./libs
    }
}

LIBS += -L$$PWD/LimeReport
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/LimeReport/debug/ -llimereportd
} else {
    LIBS += -L$$PWD/LimeReport/release/ -llimereport
}
message($$LIBS)

!CONFIG(static_build) : CONFIG(zint) {
    LIBS += -L$${DEST_LIBS}
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/LimeReport/debug/ -lQtZintd
    } else {
        LIBS += -L$$PWD/LimeReport/release/ -lQtZint
    }
}

#----------------------------------------------------------------------------------------
#------------------------------------- OPENSSL ------------------------------------------

unix:!macx: LIBS += -L$$PWD/openssl/ -lssl -lcrypto
win32: LIBS += -L$$PWD/openssl/ -llibssl -llibcrypto
INCLUDEPATH += $$PWD/openssl

#----------------------------------------------------------------------------------------
#------------------------------------- ERROR WINDOWS ------------------------------------

# :-1: error: dependent '..\..\..\..\..\..\Qt\6.5.3\msvc2019_64\include\QtWidgets\QMainWindow' does not exist.
# corecteaza QMAKE_PROJECT_DEPTH = 0
win32{
    QMAKE_PROJECT_DEPTH = 0
}
