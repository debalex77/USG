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
            gui-private \
            concurrent

#-----------------------------------------------------------------------
#------ Verifică dacă versiunea de Qt este cel puțin 6.9.3

lessThan(QT_MAJOR_VERSION, 6) {
    error("This project requires Qt version 6.9.3 or higher.")
} else:equals(QT_MAJOR_VERSION, 6) {
    lessThan(QT_MINOR_VERSION, 9) {
        error("This project requires Qt version 6.9.3 or higher.")
    } else:equals(QT_MINOR_VERSION, 9) {
        lessThan(QT_PATCH_VERSION, 3) {
            error("This project requires Qt version 6.9.3 or higher.")
        }
    }
}

message("Qt version is sufficient: "$$QT_MAJOR_VERSION"."$$QT_MINOR_VERSION"."$$QT_PATCH_VERSION)


#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_QML_DEBUG_NO_WARNING
CONFIG  -= qml_debug

#-----------------------------------------------------------------------
#------ INFO APP

# Definim componentele versiunii
USG_VERSION_MAJOR   = 4
USG_VERSION_MINOR   = 1
USG_VERSION_RELEASE = 1


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
RC_ICONS = resources/img/app_ico/eco_512x512.ico
ICON = resources/img/app_ico/eco_512x512.icns

#-----------------------------------------------------------------------
#------ CONFIG APP

CONFIG += c++20

#-----------------------------------------------------------------------
#------ SOURCES, HEADERS, FORMS, TRANSLATIONS

INCLUDEPATH += src

mac {
    SOURCES += src/others/AppDelegate.mm
}

SOURCES += \
    src/catalogs/asistanttipapp.cpp \
    src/catalogs/catalogdialog.cpp \
    src/catalogs/onlineaccountdialog.cpp \
    src/catalogs/userdialog.cpp \
    src/common/tablecolumnscontroller.cpp \
    src/data/fetalreferenceranges.cpp \
    src/documents/orderdialog.cpp \
    src/documents/pricingdialog.cpp \
    src/documents/reportdialog.cpp \
    src/documents/reportpagebreast.cpp \
    src/documents/reportpagegestation0.cpp \
    src/documents/reportpagegestation1.cpp \
    src/documents/reportpagegestation2.cpp \
    src/documents/reportpagegynecology.cpp \
    src/documents/reportpageimage.cpp \
    src/documents/reportpagelymphnodes.cpp \
    src/documents/reportpageorgansinternal.cpp \
    src/documents/reportpageprostate.cpp \
    src/documents/reportpagethyroid.cpp \
    src/documents/reportpageurinarysystem.cpp \
    src/documents/reportpagevideo.cpp \
    src/threads/patientsaverworker.cpp \
    src/threads/syncdocsworker.cpp \
    src/threads/syncorderworker.cpp \
    src/threads/syncpatientworker.cpp \
    src/threads/syncreportworker.cpp \
    src/views/catalogtableeditor.cpp \
    src/views/catalogview.cpp \
    src/catalogs/catforsqltablemodel.cpp \
    src/catalogs/chooseformprint.cpp \
    src/catalogs/contractdialog.cpp \
    src/catalogs/customperiod.cpp \
    src/catalogs/groupinvestigation.cpp \
    src/catalogs/groupinvestigationlist.cpp \
    src/catalogs/normograms.cpp \
    src/catalogs/organizationdialog.cpp \
    src/catalogs/patienthistory.cpp \
    src/catalogs/agentsendemail.cpp \
    src/common/appcontroller.cpp \
    src/common/archivecreationhandler.cpp \
    src/common/balloontip.cpp \
    src/common/cloudserverconfig.cpp \
    src/common/contonline.cpp \
    src/common/cryptomanager.cpp \
    src/common/globals.cpp \
    src/common/emailcore.cpp \
    src/common/firstrunwizard.cpp \
    src/common/handlerfunctionthread.cpp \
    src/common/logmanager.cpp \
    src/common/processingaction.cpp \
    src/common/splashmanager.cpp \
    src/common/thememanager.cpp \
    src/common/windowmanager.cpp \
    src/customs/custommessage.cpp \
    src/customs/lineeditcustom.cpp \
    src/customs/lineeditopen.cpp \
    src/customs/lineeditpassword.cpp \
    src/customs/loglevelbutton.cpp \
    src/customs/searchlineedit.cpp \
    src/customs/customdialoginvestig.cpp \
    src/customs/toolbarcustom.cpp \
    src/data/about.cpp \
    src/data/appsettings.cpp \
    src/data/appsettingsstore.cpp \
    src/data/appsettingsvalidator.cpp \
    src/data/authorizationuser.cpp \
    src/data/database.cpp \
    src/data/database_common.cpp \
    src/data/databaseselection.cpp \
    src/data/downloader.cpp \
    src/data/downloaderversion.cpp \
    src/data/initlaunch.cpp \
    src/data/loggingcategories.cpp \
    src/data/legacysettingscodec.cpp \
    src/data/mainwindow.cpp \
    src/data/mdiareacontainer.cpp \
    src/data/popup.cpp \
    src/data/reports.cpp \
    src/data/updatereleasesapp.cpp \
    src/data/userpreferences.cpp \
    src/delegates/centericondelegate.cpp \
    src/delegates/checkboxdelegate.cpp \
    src/delegates/combodelegate.cpp \
    src/delegates/doublespinboxdelegate.cpp \
    src/documents/appointmentdialog.cpp \
    src/common/infowindow.cpp \
    main.cpp \
    src/views/onlineaccountview.cpp \
    src/views/orderview.cpp \
    src/views/reportview.cpp \
    src/views/pricingview.cpp \
    src/models/baseabstractmodel.cpp \
    src/models/basesqlquerymodel.cpp \
    src/models/basesqltablemodel.cpp \
    src/models/catalogsloader.cpp \
    src/models/catalogsmodel.cpp \
    src/models/orderjournalloader.cpp \
    src/models/orderjournalmodel.cpp \
    src/models/orderinvestigationmodel.cpp \
    src/models/onlineaccountmodel.cpp \
    src/models/organizationcontractmodel.cpp \
    src/models/reportjournalloader.cpp \
    src/models/reportjournalmodel.cpp \
    src/models/pricingmodel.cpp \
    src/models/pricingsortmodel.cpp \
    src/models/queryrolesmodel.cpp \
    src/models/registrationtablemodel.cpp \
    src/models/sortmodel.cpp \
    src/models/tabledocmodel.cpp \
    src/models/treeitem.cpp \
    src/models/treemodel.cpp \
    src/models/variantmaptablemodel.cpp \
    src/reports/reportdashboard.cpp \
    src/threads/databaseprovider.cpp \
    src/threads/dataconstantsworker.cpp \
    src/threads/docemailexporterworker.cpp \
    src/threads/docsyncworker.cpp \
    src/threads/patientdatasaverworker.cpp \
    src/threads/syncpatientdataworker.cpp

HEADERS += \
    src/catalogs/asistanttipapp.h \
    src/catalogs/catalogdialog.h \
    src/catalogs/onlineaccountdialog.h \
    src/catalogs/userdialog.h \
    src/common/tablecolumnscontroller.h \
    src/data/fetalreferenceranges.h \
    src/documents/orderdialog.h \
    src/documents/pricingdialog.h \
    src/documents/reportdialog.h \
    src/documents/reportpagebase.h \
    src/documents/reportpagebreast.h \
    src/documents/reportpagegestation0.h \
    src/documents/reportpagegestation1.h \
    src/documents/reportpagegestation2.h \
    src/documents/reportpagegynecology.h \
    src/documents/reportpageimage.h \
    src/documents/reportpagelymphnodes.h \
    src/documents/reportpageorgansinternal.h \
    src/documents/reportpageprostate.h \
    src/documents/reportpagethyroid.h \
    src/documents/reportpageurinarysystem.h \
    src/documents/reportpagevideo.h \
    src/threads/patientsaverworker.h \
    src/threads/syncdocsworker.h \
    src/threads/syncorderworker.h \
    src/threads/syncpatientworker.h \
    src/threads/syncreportworker.h \
    src/views/catalogtableeditor.h \
    src/views/catalogview.h \
    src/catalogs/catforsqltablemodel.h \
    src/catalogs/chooseformprint.h \
    src/catalogs/contractdialog.h \
    src/catalogs/customperiod.h \
    src/catalogs/groupinvestigation.h \
    src/catalogs/groupinvestigationlist.h \
    src/catalogs/normograms.h \
    src/catalogs/organizationdialog.h \
    src/catalogs/patienthistory.h \
    src/catalogs/agentsendemail.h \
    src/common/appcontroller.h \
    src/common/appmetatypes.h \
    src/common/archivecreationhandler.h \
    src/common/balloontip.h \
    src/common/cloudserverconfig.h \
    src/common/contonline.h \
    src/common/cryptomanager.h \
    src/common/globals.h \
    src/common/emailcore.h \
    src/common/firstrunwizard.h \
    src/common/handlerfunctionthread.h \
    src/common/logmanager.h \
    src/common/processingaction.h \
    src/common/property_macros.h \
    src/common/reportsettingsmanager.h \
    src/common/splashmanager.h \
    src/common/structvariable.h \
    src/common/table_sections.h \
    src/common/thememanager.h \
    src/common/version.h \
    src/common/windowmanager.h \
    src/customs/custommessage.h \
    src/customs/lineeditcustom.h \
    src/customs/lineeditopen.h \
    src/customs/lineeditpassword.h \
    src/customs/loglevelbutton.h \
    src/customs/searchlineedit.h \
    src/customs/customdialoginvestig.h \
    src/customs/toolbarcustom.h \
    src/data/about.h \
    src/data/appsettings.h \
    src/data/appsettingsstore.h \
    src/data/appsettingsvalidator.h \
    src/data/authorizationuser.h \
    src/data/database.h \
    src/data/database_common.h \
    src/data/databaseselection.h \
    src/data/downloader.h \
    src/data/downloaderversion.h \
    src/data/enums.h \
    src/data/initlaunch.h \
    src/data/loggingcategories.h \
    src/data/legacysettingscodec.h \
    src/data/mainwindow.h \
    src/data/mdiareacontainer.h \
    src/data/popup.h \
    src/data/reports.h \
    src/data/updatereleasesapp.h \
    src/data/userpreferences.h \
    src/common/databaseinit.h \
    src/delegates/centericondelegate.h \
    src/delegates/checkboxdelegate.h \
    src/delegates/combodelegate.h \
    src/delegates/doublespinboxdelegate.h \
    src/documents/appointmentdialog.h \
    src/common/infowindow.h \
    src/views/onlineaccountview.h \
    src/views/orderview.h \
    src/views/reportview.h \
    src/views/pricingview.h \
    src/models/baseabstractmodel.h \
    src/models/basesqlquerymodel.h \
    src/models/basesqltablemodel.h \
    src/models/catalogsloader.h \
    src/models/catalogsmodel.h \
    src/models/orderjournalloader.h \
    src/models/orderjournalmodel.h \
    src/models/orderinvestigationmodel.h \
    src/models/onlineaccountmodel.h \
    src/models/organizationcontractmodel.h \
    src/models/reportjournalloader.h \
    src/models/reportjournalmodel.h \
    src/models/pricingmodel.h \
    src/models/pricingsortmodel.h \
    src/models/queryrolesmodel.h \
    src/models/registrationtablemodel.h \
    src/models/sortmodel.h \
    src/models/tabledocmodel.h \
    src/models/treeitem.h \
    src/models/treemodel.h \
    src/models/variantmaptablemodel.h \
    src/reports/reportdashboard.h \
    src/threads/databaseprovider.h \
    src/threads/dataconstantsworker.h \
    src/threads/docemailexporterworker.h \
    src/threads/docsyncworker.h \
    src/threads/patientdatasaverworker.h \
    src/threads/syncpatientdataworker.h

win32 {
# pentru eroarea numai pe Windows:
# - error: LNK2019: unresolved external symbol "public: void __cdecl  LimeReport::ICallbackDatasource::getCallbackData etc."
# - error: LNK2001: unresolved external symbol "public: static struct QMetaObject const LimeReport::PreviewReportWidget::staticMetaObject etc."
# - error: LNK2001: unresolved external symbol "public: static struct QMetaObject const LimeReport::ReportEngine::staticMetaObject etc. etc."
# ... problema in LimeReport cu conectarile:
#       exemplu - connect(m_report, QOverload<int>::of(&LimeReport::ReportEngine::renderPageFinished), this, QOverload<int>::of(&Reports::renderPageFinished));
    HEADERS += \
        3rdparty/LimeReport/include/lrcallbackdatasourceintf.h \
        3rdparty/LimeReport/include/lrpreviewreportwidget.h \
        3rdparty/LimeReport/include/lrreportengine.h
}

FORMS += \
    src/catalogs/asistanttipapp.ui \
    src/catalogs/catalogdialog.ui \
    src/catalogs/onlineaccountdialog.ui \
    src/catalogs/userdialog.ui \
    src/documents/orderdialog.ui \
    src/documents/pricingdialog.ui \
    src/documents/reportdialog.ui \
    src/documents/reportpagebreast.ui \
    src/documents/reportpagegestation0.ui \
    src/documents/reportpagegestation1.ui \
    src/documents/reportpagegestation2.ui \
    src/documents/reportpagegynecology.ui \
    src/documents/reportpageimage.ui \
    src/documents/reportpagelymphnodes.ui \
    src/documents/reportpageorgansinternal.ui \
    src/documents/reportpageprostate.ui \
    src/documents/reportpagethyroid.ui \
    src/documents/reportpageurinarysystem.ui \
    src/documents/reportpagevideo.ui \
    src/views/catalogtableeditor.ui \
    src/views/catalogview.ui \
    src/catalogs/catforsqltablemodel.ui \
    src/catalogs/chooseformprint.ui \
    src/catalogs/contractdialog.ui \
    src/catalogs/customperiod.ui \
    src/catalogs/groupinvestigation.ui \
    src/catalogs/groupinvestigationlist.ui \
    src/catalogs/normograms.ui \
    src/catalogs/organizationdialog.ui \
    src/catalogs/patienthistory.ui \
    src/catalogs/agentsendemail.ui \
    src/common/archivecreationhandler.ui \
    src/common/cloudserverconfig.ui \
    src/common/contonline.ui \
    src/common/firstrunwizard.ui \
    src/common/processingaction.ui \
    src/customs/custommessage.ui \
    src/data/about.ui \
    src/data/appsettings.ui \
    src/data/authorizationuser.ui \
    src/data/databaseselection.ui \
    src/data/initlaunch.ui \
    src/data/mainwindow.ui \
    src/data/reports.ui \
    src/data/userpreferences.ui \
    src/documents/appointmentdialog.ui \
    src/common/infowindow.ui \
    src/views/onlineaccountview.ui \
    src/views/orderview.ui \
    src/views/pricingview.ui \
    src/reports/reportdashboard.ui \
    src/views/reportview.ui

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
    LICENSE.txt \
    README.md \
    README-RO.md \
    release.md \
    index.html \
    privacy.html \
    robots.txt \
    sitemap.xml \
    third_party/THIRD_PARTY_ICONS.md \
    third_party/LIMEREPORT.md \
    patches/limereport/1.7.23/0001-make-singleton-destruction-idempotent.patch \
    .github/workflows/build-linux.yml \
    .github/workflows/release-linux.yml \
    TODO.md \
    build_scripts/build_maosx \
    build_scripts/build_new \
    build_scripts/build_win \
    resources/fonts/freefontsdownload.txt \
    resources/fonts/www.freefontsdownload.net.url \
    translate/USG_ro_RO.qm \
    translate/USG_ru_RU.qm \
    build_scripts/debian/control \
    build_scripts/debian/postinst \
    build_scripts/debian/preinst \
    build_scripts/debian/prerm \
    build_scripts/debian/usr/share/applications/org.alovada.usg.desktop \
    build_scripts/debian/usr/share/doc/usg/changelog \
    build_scripts/debian/usr/share/doc/usg/changelog.Debian \
    build_scripts/debian/usr/share/doc/usg/copyright \
    build_scripts/debian/usr/share/metainfo/org.alovada.usg.metainfo.xml \
    installer/linux/config/config.xml \
    installer/linux/config/eco.png \
    installer/linux/config/eco_256x256.png \
    installer/linux/config/logo.png \
    installer/linux/config/style.qss \
    installer/linux/config/welcome.html \
    installer/linux/packages/com.alovada.usg/meta/installscript.qs \
    installer/linux/packages/com.alovada.usg/meta/license.txt \
    installer/linux/packages/com.alovada.usg/meta/package.xml \
    resources/styles/style_dark.qss \
    version.txt

RESOURCES += \
    installer/linux/config/installer.qrc \
    resources/resource.qrc

macx{
    CONFIG += app_bundle
}

#----------------------------------------------------------------------------------------
#---------------------------------- LIMEREPORT ------------------------------------------

INCLUDEPATH += $$PWD/3rdparty/LimeReport/include
DEPENDPATH  += $$PWD/3rdparty/LimeReport/include

# Adaugă ambele directoare pentru build în INCLUDEPATH (dacă header-ele pot varia)
INCLUDEPATH += $$PWD/3rdparty/LimeReport/debug
INCLUDEPATH += $$PWD/3rdparty/LimeReport/release

unix:!macx {
    # În build-ul shadow al Qt Creator bibliotecile rămân în proiect,
    # nu în directorul în care este generat executabilul.
    # DT_RPATH este necesar și pentru dependențele indirecte LimeReport
    # (în special libQtZint.so).
    QMAKE_LFLAGS += -Wl,--disable-new-dtags
    QMAKE_RPATHDIR += $$PWD/3rdparty/LimeReport/debug
    QMAKE_RPATHDIR += $$PWD/3rdparty/LimeReport/release
}

# LimeReport Library
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3rdparty/LimeReport/debug/ -llimereportd
} else {
    LIBS += -L$$PWD/3rdparty/LimeReport/release/ -llimereport
}

# QtZint Library
CONFIG(debug, debug|release) {
    unix:!macx {
        LIBS += -Wl,--no-as-needed -L$$PWD/3rdparty/LimeReport/debug/ -lQtZintd -Wl,--as-needed
    } else {
        LIBS += -L$$PWD/3rdparty/LimeReport/debug/ -lQtZintd
    }
} else {
    unix:!macx {
        LIBS += -Wl,--no-as-needed -L$$PWD/3rdparty/LimeReport/release/ -lQtZint -Wl,--as-needed
    } else {
        LIBS += -L$$PWD/3rdparty/LimeReport/release/ -lQtZint
    }
}

# macOS specific
macx {
    LIBS += -L$$PWD/3rdparty/LimeReport/debug/ -llimereportd -lQtZintd
    LIBS += -L$$PWD/3rdparty/LimeReport/release/ -llimereport -lQtZint
}

# Opțional: suport pentru variabilă externă DEST_LIBS (dacă o folosești)
!CONFIG(static_build):CONFIG(zint) {
    isEmpty(DEST_LIBS): DEST_LIBS = $$PWD/3rdparty/LimeReport/release
    CONFIG(debug, debug|release) {
        LIBS += -L$${DEST_LIBS} -lQtZintd
    } else {
        LIBS += -L$${DEST_LIBS} -lQtZint
    }
}

#----------------------------------------------------------------------------------------
#------------------------------------- OPENSSL ------------------------------------------

OPENSSL_DIR = $$PWD/3rdparty/openssl

INCLUDEPATH += $$PWD/3rdparty/openssl

# Linkare librării OpenSSL specifice platformelor
win32 {
    INCLUDEPATH += $$OPENSSL_DIR/include
    LIBS += /LIBPATH:$$OPENSSL_DIR/lib libssl.lib libcrypto.lib
}

unix:!macx {
    LIBS += -L$$PWD/3rdparty/openssl -lssl -lcrypto
}

macx {
    LIBS += -L$$PWD/3rdparty/openssl -lssl -lcrypto
}

#----------------------------------------------------------------------------------------
#------------------------------------- ERROR WINDOWS ------------------------------------

# :-1: error: dependent '..\..\..\..\..\..\Qt\6.5.3\msvc2019_64\include\QtWidgets\QMainWindow' does not exist.
# corecteaza QMAKE_PROJECT_DEPTH = 0
win32{
    QMAKE_PROJECT_DEPTH = 0
}
