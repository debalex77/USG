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
USG_VERSION_TEXT    = $$cat($$PWD/version.txt, lines)
USG_VERSION_PARTS   = $$split(USG_VERSION_TEXT, .)
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
#------ CONFIG C++

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

#-----------------------------------------------------------------------
#------ USER MANUAL

DISTFILES += \
    docs/user_manual/USER_MANUAL.md \
    docs/user_manual/screenshots/0-about.png \
    docs/user_manual/screenshots/01-overview.png \
    docs/user_manual/screenshots/02-startup.png \
    docs/user_manual/screenshots/03-first-run-wizard.png \
    docs/user_manual/screenshots/04-login.png \
    docs/user_manual/screenshots/05-main-window.png \
    docs/user_manual/screenshots/06-catalogs.png \
    docs/user_manual/screenshots/07-appointments.png \
    docs/user_manual/screenshots/08-patients.png \
    docs/user_manual/screenshots/09-patient-history.png \
    docs/user_manual/screenshots/10-order.png \
    docs/user_manual/screenshots/11-informed-consent.png \
    docs/user_manual/screenshots/12-order-journal.png \
    docs/user_manual/screenshots/13-report.png \
    docs/user_manual/screenshots/14-report-internal-organs.png \
    docs/user_manual/screenshots/15-report-urinary.png \
    docs/user_manual/screenshots/16-report-prostate.png \
    docs/user_manual/screenshots/17-report-gynecology.png \
    docs/user_manual/screenshots/18-report-breast.png \
    docs/user_manual/screenshots/19-report-thyroid.png \
    docs/user_manual/screenshots/20-report-pregnancy.png \
    docs/user_manual/screenshots/21-report-soft-tissue.png \
    docs/user_manual/screenshots/22-report-images.png \
    docs/user_manual/screenshots/23-report-video.png \
    docs/user_manual/screenshots/24-normograms.png \
    docs/user_manual/screenshots/25-report-journal.png \
    docs/user_manual/screenshots/26-print-preview.png \
    docs/user_manual/screenshots/27-email.png \
    docs/user_manual/screenshots/28-pricing.png \
    docs/user_manual/screenshots/29-statistics.png \
    docs/user_manual/screenshots/30-settings.png \
    docs/user_manual/screenshots/31-backup.png

#------------------------------------------------------------------------
#------- TRANSLATION TOOLS

# lupdate USG.pro -ts translate/USG_ru_RU.ts
# linguist translate/USG_ru_RU.ts
# lrelease translate/USG_ru_RU.ts
