SOURCES += \
    $$USG_ROOT/src/app/about.cpp \
    $$USG_ROOT/src/settings/appsettings.cpp \
    $$USG_ROOT/src/settings/appsettingsstore.cpp \
    $$USG_ROOT/src/settings/appsettingsvalidator.cpp \
    $$USG_ROOT/src/settings/profilesecretcodec.cpp \
    $$USG_ROOT/src/app/authorizationuser.cpp \
    $$USG_ROOT/src/database/database.cpp \
    $$USG_ROOT/src/database/database_common.cpp \
    $$USG_ROOT/src/app/databaseselection.cpp \
    $$USG_ROOT/src/app/downloader.cpp \
    $$USG_ROOT/src/app/downloaderversion.cpp \
    $$USG_ROOT/src/features/reports/fetalreferenceranges.cpp \
    $$USG_ROOT/src/app/initlaunch.cpp \
    $$USG_ROOT/src/settings/legacysettingscodec.cpp \
    $$USG_ROOT/src/core/loggingcategories.cpp \
    $$USG_ROOT/src/app/mainwindow.cpp \
    $$USG_ROOT/src/app/mdiareacontainer.cpp \
    $$USG_ROOT/src/app/popup.cpp \
    $$USG_ROOT/src/infrastructure/reporting/reports.cpp \
    $$USG_ROOT/src/database/updatereleasesapp.cpp \
    $$USG_ROOT/src/database/uuidmigrationplan.cpp

HEADERS += \
    $$USG_ROOT/src/app/about.h \
    $$USG_ROOT/src/settings/appsettings.h \
    $$USG_ROOT/src/settings/appsettingsstore.h \
    $$USG_ROOT/src/settings/appsettingsvalidator.h \
    $$USG_ROOT/src/settings/profilesecretcodec.h \
    $$USG_ROOT/src/app/authorizationuser.h \
    $$USG_ROOT/src/database/database.h \
    $$USG_ROOT/src/database/database_common.h \
    $$USG_ROOT/src/app/databaseselection.h \
    $$USG_ROOT/src/app/downloader.h \
    $$USG_ROOT/src/app/downloaderversion.h \
    $$USG_ROOT/src/core/enums.h \
    $$USG_ROOT/src/features/reports/fetalreferenceranges.h \
    $$USG_ROOT/src/app/initlaunch.h \
    $$USG_ROOT/src/settings/legacysettingscodec.h \
    $$USG_ROOT/src/core/loggingcategories.h \
    $$USG_ROOT/src/app/mainwindow.h \
    $$USG_ROOT/src/app/mdiareacontainer.h \
    $$USG_ROOT/src/app/popup.h \
    $$USG_ROOT/src/infrastructure/reporting/reports.h \
    $$USG_ROOT/src/database/updatereleasesapp.h \
    $$USG_ROOT/src/database/uuidmigrationplan.h

FORMS += \
    $$USG_ROOT/src/app/about.ui \
    $$USG_ROOT/src/settings/appsettings.ui \
    $$USG_ROOT/src/app/authorizationuser.ui \
    $$USG_ROOT/src/app/databaseselection.ui \
    $$USG_ROOT/src/app/initlaunch.ui \
    $$USG_ROOT/src/app/mainwindow.ui \
    $$USG_ROOT/src/infrastructure/reporting/reports.ui

SOURCES += \
    $$USG_ROOT/src/infrastructure/backup/archivecreationhandler.cpp \
    $$USG_ROOT/src/infrastructure/database/databaseprovider.cpp \
    $$USG_ROOT/src/infrastructure/database/dataconstantsworker.cpp \
    $$USG_ROOT/src/infrastructure/email/docemailexporterworker.cpp \
    $$USG_ROOT/src/infrastructure/email/emailcore.cpp \
    $$USG_ROOT/src/infrastructure/security/cryptomanager.cpp \
    $$USG_ROOT/src/infrastructure/sync/docsyncworker.cpp \
    $$USG_ROOT/src/infrastructure/persistence/patientdatasaverworker.cpp \
    $$USG_ROOT/src/infrastructure/persistence/patientsaverworker.cpp \
    $$USG_ROOT/src/infrastructure/sync/syncdocsworker.cpp \
    $$USG_ROOT/src/infrastructure/sync/syncorderworker.cpp \
    $$USG_ROOT/src/infrastructure/sync/syncpatientdataworker.cpp \
    $$USG_ROOT/src/infrastructure/sync/syncpatientworker.cpp \
    $$USG_ROOT/src/infrastructure/sync/syncreportworker.cpp

HEADERS += \
    $$USG_ROOT/src/infrastructure/backup/archivecreationhandler.h \
    $$USG_ROOT/src/infrastructure/database/databaseprovider.h \
    $$USG_ROOT/src/infrastructure/database/dataconstantsworker.h \
    $$USG_ROOT/src/infrastructure/email/docemailexporterworker.h \
    $$USG_ROOT/src/infrastructure/email/emailcore.h \
    $$USG_ROOT/src/infrastructure/security/cryptomanager.h \
    $$USG_ROOT/src/infrastructure/sync/docsyncworker.h \
    $$USG_ROOT/src/infrastructure/persistence/patientdatasaverworker.h \
    $$USG_ROOT/src/infrastructure/persistence/patientsaverworker.h \
    $$USG_ROOT/src/infrastructure/sync/syncdocsworker.h \
    $$USG_ROOT/src/infrastructure/sync/syncorderworker.h \
    $$USG_ROOT/src/infrastructure/sync/syncpatientdataworker.h \
    $$USG_ROOT/src/infrastructure/sync/syncpatientworker.h \
    $$USG_ROOT/src/infrastructure/sync/syncreportworker.h

FORMS += $$USG_ROOT/src/infrastructure/backup/archivecreationhandler.ui
