SOURCES += $$USG_ROOT/main.cpp

mac {
    SOURCES += $$USG_ROOT/src/others/AppDelegate.mm
}

SOURCES += \
    $$USG_ROOT/src/app/appcontroller.cpp \
    $$USG_ROOT/src/common/applicationpathscontext.cpp \
    $$USG_ROOT/src/common/cloudconnectioncontext.cpp \
    $$USG_ROOT/src/common/doctorcontext.cpp \
    $$USG_ROOT/src/app/firstrunwizard.cpp \
    $$USG_ROOT/src/common/globals.cpp \
    $$USG_ROOT/src/common/maindatabaseconnectioncontext.cpp \
    $$USG_ROOT/src/core/logging/logmanager.cpp \
    $$USG_ROOT/src/common/organizationcontext.cpp \
    $$USG_ROOT/src/common/sessioncontext.cpp \
    $$USG_ROOT/src/common/reportdialogcontext.cpp \
    $$USG_ROOT/src/common/orderdialogcontext.cpp \
    $$USG_ROOT/src/app/splashmanager.cpp \
    $$USG_ROOT/src/app/windowmanager.cpp

HEADERS += \
    $$USG_ROOT/src/app/appcontroller.h \
    $$USG_ROOT/src/common/applicationpathscontext.h \
    $$USG_ROOT/src/common/appmetatypes.h \
    $$USG_ROOT/src/common/cloudconnectioncontext.h \
    $$USG_ROOT/src/database/databaseinit.h \
    $$USG_ROOT/src/common/doctorcontext.h \
    $$USG_ROOT/src/app/firstrunwizard.h \
    $$USG_ROOT/src/common/globals.h \
    $$USG_ROOT/src/common/maindatabaseconnectioncontext.h \
    $$USG_ROOT/src/core/logging/logmanager.h \
    $$USG_ROOT/src/common/organizationcontext.h \
    $$USG_ROOT/src/common/property_macros.h \
    $$USG_ROOT/src/settings/layoutsettingsmanager.h \
    $$USG_ROOT/src/common/sessioncontext.h \
    $$USG_ROOT/src/app/splashmanager.h \
    $$USG_ROOT/src/common/structvariable.h \
    $$USG_ROOT/src/common/table_sections.h \
    $$USG_ROOT/src/common/reportdialogcontext.h \
    $$USG_ROOT/src/common/orderdialogcontext.h \
    $$USG_ROOT/src/core/version.h \
    $$USG_ROOT/src/app/windowmanager.h

FORMS += \
    $$USG_ROOT/src/app/firstrunwizard.ui

SOURCES += \
    $$USG_ROOT/src/ui/dialogs/customdialoginvestig.cpp \
    $$USG_ROOT/src/ui/dialogs/custommessage.cpp \
    $$USG_ROOT/src/ui/dialogs/infowindow.cpp \
    $$USG_ROOT/src/ui/dialogs/processingaction.cpp \
    $$USG_ROOT/src/ui/services/tablecolumnscontroller.cpp \
    $$USG_ROOT/src/ui/services/thememanager.cpp \
    $$USG_ROOT/src/ui/widgets/balloontip.cpp \
    $$USG_ROOT/src/ui/widgets/lineeditcustom.cpp \
    $$USG_ROOT/src/ui/widgets/lineeditopen.cpp \
    $$USG_ROOT/src/ui/widgets/lineeditpassword.cpp \
    $$USG_ROOT/src/ui/widgets/loglevelbutton.cpp \
    $$USG_ROOT/src/ui/widgets/searchlineedit.cpp \
    $$USG_ROOT/src/ui/widgets/toolbarcustom.cpp

HEADERS += \
    $$USG_ROOT/src/ui/dialogs/customdialoginvestig.h \
    $$USG_ROOT/src/ui/dialogs/custommessage.h \
    $$USG_ROOT/src/ui/dialogs/infowindow.h \
    $$USG_ROOT/src/ui/dialogs/processingaction.h \
    $$USG_ROOT/src/ui/services/tablecolumnscontroller.h \
    $$USG_ROOT/src/ui/services/thememanager.h \
    $$USG_ROOT/src/ui/widgets/balloontip.h \
    $$USG_ROOT/src/ui/widgets/lineeditcustom.h \
    $$USG_ROOT/src/ui/widgets/lineeditopen.h \
    $$USG_ROOT/src/ui/widgets/lineeditpassword.h \
    $$USG_ROOT/src/ui/widgets/loglevelbutton.h \
    $$USG_ROOT/src/ui/widgets/searchlineedit.h \
    $$USG_ROOT/src/ui/widgets/toolbarcustom.h

FORMS += \
    $$USG_ROOT/src/ui/dialogs/custommessage.ui \
    $$USG_ROOT/src/ui/dialogs/infowindow.ui \
    $$USG_ROOT/src/ui/dialogs/processingaction.ui

SOURCES += \
    $$USG_ROOT/src/ui/delegates/centericondelegate.cpp \
    $$USG_ROOT/src/ui/delegates/checkboxdelegate.cpp \
    $$USG_ROOT/src/ui/delegates/combodelegate.cpp \
    $$USG_ROOT/src/ui/delegates/doublespinboxdelegate.cpp

HEADERS += \
    $$USG_ROOT/src/ui/delegates/centericondelegate.h \
    $$USG_ROOT/src/ui/delegates/checkboxdelegate.h \
    $$USG_ROOT/src/ui/delegates/combodelegate.h \
    $$USG_ROOT/src/ui/delegates/doublespinboxdelegate.h

SOURCES += \
    $$USG_ROOT/src/settings/settingsdialog.cpp \
    $$USG_ROOT/src/settings/settingsrepository.cpp \
    $$USG_ROOT/src/settings/settingsservice.cpp

HEADERS += \
    $$USG_ROOT/src/settings/settingsdialog.h \
    $$USG_ROOT/src/settings/settingsrepository.h \
    $$USG_ROOT/src/settings/settingsservice.h \
    $$USG_ROOT/src/settings/settingstypes.h

FORMS += $$USG_ROOT/src/settings/settingsdialog.ui
