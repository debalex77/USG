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
USG_VERSION_TEXT = $$cat($$PWD/version.txt, lines)
USG_VERSION_PARTS = $$split(USG_VERSION_TEXT, .)
USG_VERSION_MAJOR   = $$member(USG_VERSION_PARTS, 0)
USG_VERSION_MINOR   = $$member(USG_VERSION_PARTS, 1)
USG_VERSION_RELEASE = $$member(USG_VERSION_PARTS, 2)


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
#------ SURSE, ANTETE SI INTERFETE PE MODULE

USG_ROOT = $$PWD
INCLUDEPATH += $$USG_ROOT/src

include($$USG_ROOT/qmake/modules/app.pri)
include($$USG_ROOT/qmake/modules/features.pri)
include($$USG_ROOT/qmake/modules/infrastructure.pri)
include($$USG_ROOT/qmake/modules/resources.pri)
include($$USG_ROOT/qmake/modules/dependencies.pri)
