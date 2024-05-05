QT       += core \
            gui \
            sql \
            webenginewidgets \
            printsupport \
            xml \
            multimedia \
            multimediawidgets \
            gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_QML_DEBUG_NO_WARNING
CONFIG- = qml_debug

VERSION                  = 2.0.6
QMAKE_TARGET_COMPANY     = SC 'Alovada-Med' SRL
QMAKE_TARGET_PRODUCT     = USG project
QMAKE_TARGET_DESCRIPTION = Evidenta examinarilor ecografice
QMAKE_TARGET_COPYRIGHT   = Codreanu Alexandru
RC_ICONS = img/eco_512x512.ico
ICON = img/eco_512x512.icns

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    catalogs/asistanttipapp.cpp \
    catalogs/catcontracts.cpp \
    catalogs/catforsqltablemodel.cpp \
    catalogs/catgeneral.cpp \
    catalogs/catorganizations.cpp \
    catalogs/catusers.cpp \
    catalogs/chooseformprint.cpp \
    catalogs/customperiod.cpp \
    catalogs/listform.cpp \
    catalogs/normograms.cpp \
    catalogs/patienthistory.cpp \
    data/about.cpp \
    data/appsettings.cpp \
    data/authorizationuser.cpp \
    data/customdialoginvestig.cpp \
    data/database.cpp \
    data/databaseselection.cpp \
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
    models/registrationtablemodel.cpp \
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
    catalogs/listform.h \
    catalogs/normograms.h \
    catalogs/patienthistory.h \
    data/about.h \
    data/appsettings.h \
    data/authorizationuser.h \
    data/customdialoginvestig.h \
    data/database.h \
    data/databaseselection.h \
    data/downloaderversion.h \
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
    models/registrationtablemodel.h \
    models/variantmaptablemodel.h \
    resources.rc

FORMS += \
    catalogs/asistanttipapp.ui \
    catalogs/catcontracts.ui \
    catalogs/catforsqltablemodel.ui \
    catalogs/catgeneral.ui \
    catalogs/catorganizations.ui \
    catalogs/catusers.ui \
    catalogs/chooseformprint.ui \
    catalogs/customperiod.ui \
    catalogs/listform.ui \
    catalogs/normograms.ui \
    catalogs/patienthistory.ui \
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
    USG_ro_RO.ts \
    USG_ru_RU.ts \

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
    Fonts/Cantarell-Bold.ttf \
    Fonts/Cantarell-BoldOblique.ttf \
    Fonts/Cantarell-Oblique.ttf \
    Fonts/Cantarell-Regular.otf \
    Fonts/Cantarell-Regular.ttf \
    Fonts/freefontsdownload.txt \
    Fonts/www.freefontsdownload.net.url \
    USG_ro_RO.qm \
    USG_ru_RU.qm \
    releases.md \
    version.txt

RESOURCES += \
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


unix:!macx: LIBS += -L$$PWD/lib/openssl/ -lssl
unix:!macx: LIBS += -L$$PWD/lib/openssl/ -lcrypto
win32: LIBS += -L$$PWD/lib/openssl/ -llibssl
win32: LIBS += -L$$PWD/lib/openssl/ -llibcrypto

INCLUDEPATH += $$PWD/lib/openssl
DEPENDPATH += $$PWD/lib/openssl
