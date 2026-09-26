/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "common/sessioncontext.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSqlDatabase>
#include <QSqlError>

#include <common/globals.h>
#include <core/version.h>
#include <database/database.h>
#include <core/loggingcategories.h>
#include <models/basesqlquerymodel.h>
#include <settings/settingsservice.h>

SettingsDialog::SettingsDialog(DataBase &database, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_database(database)
    , m_repository(database)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(QStringLiteral(":/img/catalogs/settings.png")));
    setWindowTitle(tr("Setări %1").arg(QStringLiteral("[*]")));

    ui->editVersion->setText(QStringLiteral(USG_VERSION_FULL));
    ui->comboPrintMode->addItem(tr("Standard"),
                                int(Settings::PrintMenuMode::Standard));
    ui->comboPrintMode->addItem(tr("Preview și Designer"),
                                int(Settings::PrintMenuMode::PreviewAndDesigner));

    ui->checkArchiveSqlite->setEnabled(MainDatabaseConnectionContext::instance().isSqlite());
    if (!MainDatabaseConnectionContext::instance().isSqlite()) {
        ui->checkArchiveSqlite->setToolTip(tr("Arhivarea automată este disponibilă numai pentru baze SQLite."));
    }

    initializeModels();
    initializeConnections();
    updateLogoPreview();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setUserId(int userId)
{
    if (userId <= 0)
        return;

    const QSignalBlocker blocker(ui->comboUsers);
    setComboValue(ui->comboUsers, userId);
    loadUser(userId);
}

void SettingsDialog::reject()
{
    if (m_accepting || confirmDiscardChanges())
        QDialog::reject();
}

void SettingsDialog::initializeModels()
{
    // user
    QString usersQuery = QStringLiteral(R"(
        SELECT
            id,
            name
        FROM
            users
        WHERE
            deletionMark = 0
        ORDER BY
            name
    )");
    m_usersModel = new BaseSqlQueryModel(usersQuery, ui->comboUsers);
    m_usersModel->setModelParent(BaseSqlQueryModel::UserSettings);
    ui->comboUsers->setModel(m_usersModel);

    // organization
    QString organizationsQuery = QStringLiteral(R"(
        SELECT
            id,
            name
        FROM
            organizations
        WHERE
            deletionMark = 0
        ORDER BY
            name
    )");
    m_organizationsModel = new BaseSqlQueryModel(organizationsQuery,
                                                  ui->comboOrganization);
    m_organizationsModel->setModelParent(BaseSqlQueryModel::UserSettings);
    ui->comboOrganization->setModel(m_organizationsModel);

    // default organization
    QString defaultOrganizationsQuery = organizationsQuery;
    m_defaultOrganizationsModel = new BaseSqlQueryModel(
        defaultOrganizationsQuery, ui->comboDefaultOrganization);
    m_defaultOrganizationsModel->setModelParent(BaseSqlQueryModel::UserSettings);
    ui->comboDefaultOrganization->setModel(m_defaultOrganizationsModel);

    // doctor
    QString doctorsQuery = QStringLiteral(R"(
        SELECT
            doctors.id,
            fullNameDoctors.nameAbbreviated AS fullName
        FROM
            doctors
        INNER JOIN
            fullNameDoctors ON fullNameDoctors.id_doctors = doctors.id
        WHERE
            doctors.deletionMark = 0
        ORDER BY
            fullName
    )");
    m_doctorsModel = new BaseSqlQueryModel(doctorsQuery, ui->comboDoctor);
    m_doctorsModel->setModelParent(BaseSqlQueryModel::UserSettings);
    ui->comboDoctor->setModel(m_doctorsModel);

    // nurse
    QString nursesQuery = QStringLiteral(R"(
        SELECT
            nurses.id,
            fullNameNurses.nameAbbreviated AS fullName
        FROM
            nurses
        INNER JOIN
            fullNameNurses ON fullNameNurses.id_nurses = nurses.id
        WHERE
            nurses.deletionMark = 0
        ORDER BY
            fullName
    )");
    m_nursesModel = new BaseSqlQueryModel(nursesQuery, ui->comboNurse);
    m_nursesModel->setModelParent(BaseSqlQueryModel::UserSettings);
    ui->comboNurse->setModel(m_nursesModel);
}

void SettingsDialog::initializeConnections()
{
    const auto dirty = [this]() { markModified(); };

    connect(ui->checkUpdates, &QCheckBox::toggled, this, dirty);
    connect(ui->checkUserManual, &QCheckBox::toggled, this, dirty);
    connect(ui->checkAssistant, &QCheckBox::toggled, this, dirty);
    connect(ui->spinJournalRefresh, &QSpinBox::valueChanged, this, dirty);
    connect(ui->comboDoctor, &QComboBox::currentIndexChanged, this, dirty);
    connect(ui->comboNurse, &QComboBox::currentIndexChanged, this, dirty);
    connect(ui->editDevice, &QLineEdit::textChanged, this, dirty);
    connect(ui->checkMinimizeTray, &QCheckBox::toggled, this, dirty);
    connect(ui->checkConfirmExit, &QCheckBox::toggled, this, dirty);
    connect(ui->checkArchiveSqlite, &QCheckBox::toggled, this, dirty);
    connect(ui->checkSeparateWindows, &QCheckBox::toggled, this, dirty);
    connect(ui->comboPrintMode, &QComboBox::currentIndexChanged, this, dirty);

    connect(ui->comboOrganization, &QComboBox::currentIndexChanged,
            this, [this](int) {
                synchronizeOrganizationCombos(ui->comboOrganization,
                                              ui->comboDefaultOrganization);
                markModified();
            });
    connect(ui->comboDefaultOrganization, &QComboBox::currentIndexChanged,
            this, [this](int) {
                synchronizeOrganizationCombos(ui->comboDefaultOrganization,
                                              ui->comboOrganization);
                markModified();
            });

    connect(ui->buttonChooseLogo, &QPushButton::clicked, this, [this]() {
        const QString fileName = QFileDialog::getOpenFileName(
            this, tr("Alege logotipul"), QString(),
            tr("Imagini (*.png *.jpg *.jpeg *.bmp *.webp);;Toate fișierele (*)"));
        if (fileName.isEmpty())
            return;

        QImageReader reader(fileName);
        if (!reader.canRead()) {
            QMessageBox::warning(this, tr("Logotip"),
                                 tr("Fișierul selectat nu este o imagine validă."));
            return;
        }

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Logotip"), file.errorString());
            return;
        }
        m_loaded.values.organization.logoData = file.readAll();
        updateLogoPreview();
        markModified();
    });

    connect(ui->buttonClearLogo, &QPushButton::clicked, this, [this]() {
        if (m_loaded.values.organization.logoData.isEmpty())
            return;
        m_loaded.values.organization.logoData.clear();
        updateLogoPreview();
        markModified();
    });

    connect(ui->comboUsers, &QComboBox::currentIndexChanged,
            this, [this](int) {
                if (m_loading)
                    return;
                const int requestedUserId = comboValue(ui->comboUsers);
                if (requestedUserId <= 0 || requestedUserId == m_currentUserId)
                    return;

                if (isWindowModified()) {
                    const QMessageBox::StandardButton answer = QMessageBox::question(
                        this, tr("Setări modificate"),
                        tr("Doriți să salvați modificările înainte de a selecta alt utilizator?"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                        QMessageBox::Save);
                    if (answer == QMessageBox::Save && !saveSettings()) {
                        const QSignalBlocker blocker(ui->comboUsers);
                        setComboValue(ui->comboUsers, m_currentUserId);
                        return;
                    }
                    if (answer == QMessageBox::Cancel) {
                        const QSignalBlocker blocker(ui->comboUsers);
                        setComboValue(ui->comboUsers, m_currentUserId);
                        return;
                    }
                }
                loadUser(requestedUserId);
            });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply),
            &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok),
            &QPushButton::clicked, this, [this]() {
                if (!saveSettings())
                    return;
                m_accepting = true;
                accept();
            });
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel),
            &QPushButton::clicked, this, &SettingsDialog::reject);
}

void SettingsDialog::loadUser(int userId)
{
    m_loading = true;
    const SettingsRepository::LoadResult result = m_repository.loadForUser(userId);
    if (!result.isValid()) {
        QMessageBox::critical(this, tr("Citirea setărilor"), result.error);
        m_loading = false;
        return;
    }

    m_loaded = result.data;
    m_loaded.values.user.userId = userId;
    if (userId == SessionContext::instance().userId()) {
        m_loaded.values.synchronization = SettingsService::instance().synchronization();
    }
    m_currentUserId = userId;

    const Settings::Snapshot &values = m_loaded.values;
    ui->checkUpdates->setChecked(values.application.checkForUpdatesOnStartup);
    ui->checkUserManual->setChecked(values.application.showUserManualOnStartup);
    ui->checkAssistant->setChecked(values.application.showAssistantOnStartup);
    ui->spinJournalRefresh->setValue(values.application.documentJournalRefreshIntervalSeconds);

    setComboValue(ui->comboOrganization, values.organization.organizationId);
    setComboValue(ui->comboDefaultOrganization, values.user.defaultOrganizationId);
    setComboValue(ui->comboDoctor, values.organization.defaultDoctorId);
    setComboValue(ui->comboNurse, values.organization.defaultNurseId);
    ui->editDevice->setText(values.organization.ultrasoundDeviceName);
    updateLogoPreview();

    ui->checkMinimizeTray->setChecked(values.user.minimizeToTray);
    ui->checkConfirmExit->setChecked(values.user.confirmOnExit);
    ui->checkArchiveSqlite->setChecked(MainDatabaseConnectionContext::instance().isSqlite()
                                       && values.user.archiveSqliteOnExit);
    ui->checkSeparateWindows->setChecked(values.user.openDocumentsInSeparateWindows);
    const int printIndex = ui->comboPrintMode->findData(int(values.user.printMenuMode));
    ui->comboPrintMode->setCurrentIndex(qMax(0, printIndex));

    ui->checkSynchronization->setChecked(values.synchronization.enabled);
    QString synchronizationStatus;
    if (!values.synchronization.configured) {
        synchronizationStatus = tr("Sincronizarea cloud nu este configurată.");

    } else if (values.synchronization.enabled) {
        synchronizationStatus = tr("Sincronizarea cloud este configurată și activă.");

    } else {
        synchronizationStatus = tr("Sincronizarea cloud este configurată, dar nu este activă.");

    }

    ui->labelSynchronizationHint->setText(synchronizationStatus + QLatin1Char('\n')
        + tr("Conexiunea, parola și baza cloud se configurează în fereastra "
             "dedicată „Setări cloud server”."));

    setWindowModified(false);
    m_loading = false;
}

bool SettingsDialog::saveSettings()
{
    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("Salvarea setărilor"),
                             tr("Selectați un utilizator valid."));
        return false;
    }

    SettingsRepository::PersistedSettings settings = collectSettings();
    QSqlDatabase database = m_database.getDatabase();
    if (!database.transaction()) {
        QMessageBox::critical(this, tr("Salvarea setărilor"),
                              tr("Tranzacția nu a putut fi pornită: %1")
                                  .arg(database.lastError().text()));
        return false;
    }

    QStringList errors;
    if (!m_repository.saveForUser(settings, errors)) {
        database.rollback();
        const QString message = errors.isEmpty()
                                    ? tr("Setările nu au putut fi salvate.")
                                    : errors.join(QLatin1Char('\n'));
        qCritical(logCritical()) << "SettingsDialog save error:" << message;
        QMessageBox::critical(this, tr("Salvarea setărilor"), message);
        return false;
    }
    if (!database.commit()) {
        const QString error = database.lastError().text();
        database.rollback();
        QMessageBox::critical(this, tr("Salvarea setărilor"),
                              tr("Confirmarea tranzacției a eșuat: %1").arg(error));
        return false;
    }

    m_loaded = settings;
    if (m_currentUserId == SessionContext::instance().userId()) {
        SettingsService::instance().setSnapshot(settings.values);
    }

    setWindowModified(false);
    emit settingsApplied();
    qInfo(logInfo()) << "Setările utilizatorului au fost salvate. id="
                     << m_currentUserId;
    return true;
}

SettingsRepository::PersistedSettings SettingsDialog::collectSettings() const
{
    SettingsRepository::PersistedSettings settings = m_loaded;
    Settings::Snapshot &values = settings.values;

    values.application.checkForUpdatesOnStartup = ui->checkUpdates->isChecked();
    values.application.showUserManualOnStartup  = ui->checkUserManual->isChecked();
    values.application.showAssistantOnStartup   = ui->checkAssistant->isChecked();
    values.application.documentJournalRefreshIntervalSeconds =
        ui->spinJournalRefresh->value();

    values.organization.organizationId       = comboValue(ui->comboOrganization);
    values.organization.defaultDoctorId      = comboValue(ui->comboDoctor);
    values.organization.defaultNurseId       = comboValue(ui->comboNurse);
    values.organization.ultrasoundDeviceName = ui->editDevice->text().trimmed();

    values.user.userId                = m_currentUserId;
    values.user.defaultOrganizationId = comboValue(ui->comboDefaultOrganization);
    values.user.minimizeToTray        = ui->checkMinimizeTray->isChecked();
    values.user.confirmOnExit         = ui->checkConfirmExit->isChecked();
    values.user.archiveSqliteOnExit = MainDatabaseConnectionContext::instance().isSqlite()
                                      && ui->checkArchiveSqlite->isChecked();
    values.user.openDocumentsInSeparateWindows = ui->checkSeparateWindows->isChecked();
    values.user.printMenuMode = Settings::PrintMenuMode(ui->comboPrintMode->currentData().toInt());
    return settings;
}

void SettingsDialog::markModified()
{
    if (!m_loading)
        setWindowModified(true);
}

void SettingsDialog::setComboValue(QComboBox *combo, int value)
{
    const int index = combo->findData(value, Qt::UserRole);
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

int SettingsDialog::comboValue(const QComboBox *combo) const
{
    const int value = combo->currentData(Qt::UserRole).toInt();
    return value > 0 ? value : -1;
}

void SettingsDialog::updateLogoPreview()
{
    QPixmap logo;
    logo.loadFromData(m_loaded.values.organization.logoData);
    if (logo.isNull()) {
        ui->logoPreview->setPixmap(QPixmap());
        ui->logoPreview->setText(tr("Niciun logotip"));
        ui->buttonClearLogo->setEnabled(false);
        return;
    }

    ui->logoPreview->setText(QString());
    ui->logoPreview->setPixmap(logo.scaled(
        ui->logoPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->buttonClearLogo->setEnabled(true);
}

void SettingsDialog::synchronizeOrganizationCombos(QComboBox *source,
                                                    QComboBox *destination)
{
    if (m_loading)
        return;
    const QSignalBlocker blocker(destination);
    setComboValue(destination, comboValue(source));
}

bool SettingsDialog::confirmDiscardChanges()
{
    if (!isWindowModified())
        return true;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, tr("Setări modificate"),
        tr("Doriți să salvați modificările înainte de închidere?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    if (answer == QMessageBox::Save)
        return saveSettings();
    return answer == QMessageBox::Discard;
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    if (m_accepting || confirmDiscardChanges())
        event->accept();
    else
        event->ignore();
}
