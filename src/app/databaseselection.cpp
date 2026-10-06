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

#include "databaseselection.h"
#include "ui_databaseselection.h"
#include "common/applicationpathscontext.h"
#include "settings/appsettingsstore.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QUrl>

DatabaseSelection::DatabaseSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseSelection)
{
    ui->setupUi(this);
    ui->txtPath->setTextFormat(Qt::PlainText);

    db = new DataBase(this);
    timer = new QTimer(this);

    setWindowTitle(tr("Alege/creează baza de date"));

    QDir file_path;
    QString txt_path_config = file_path.toNativeSeparators(dirConfigPath);
    if (! QFile(txt_path_config).exists()){
        if (QDir().mkpath(txt_path_config)){
            ApplicationPathsContext::instance().setSettingsFilePath(
                file_path.toNativeSeparators(fileConfigPath));
            connect(timer, &QTimer::timeout,
                    this, &DatabaseSelection::updateTimer);
            timer->start(1000);
        } else {
            QMessageBox::warning(this,
                                 tr("Crearea directoriei"),
                                 tr("Directoria <b>'USG'</b> pentru păstrarea setărilor "
                                    "aplicației nu a fost creată !!!<br>"
                                    "Adresați-vă administratorului aplicației."),
                                 QMessageBox::Ok);
            qApp->quit();
        }
    }

    QDir dir(txt_path_config);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList listFiles = dir.entryInfoList();
    for (int n = 0; n < listFiles.size(); n++) {
        QFileInfo fileInfo = listFiles.at(n);
        if (fileInfo.suffix().compare(QStringLiteral("conf"), Qt::CaseInsensitive) == 0
            && fileInfo.completeBaseName() != QStringLiteral("report_settings")) {
            // completeBaseName() păstrează punctele din nume, de exemplu
            // „test_v4.0.1.conf” devine „test_v4.0.1”, nu „test_v4”.
            ui->listWidget->addItem(fileInfo.completeBaseName());
        }
    }

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &DatabaseSelection::onConnectToBase, Qt::UniqueConnection);
    connect(ui->btnAddDatabase, &QAbstractButton::clicked,
            this, &DatabaseSelection::onAddDatabase, Qt::UniqueConnection);
    connect(ui->btnEditSettings, &QAbstractButton::clicked,
            this, &DatabaseSelection::onEditSettings, Qt::UniqueConnection);
    connect(ui->btnRemove, &QAbstractButton::clicked,
            this, &DatabaseSelection::onRemoveRowListWidget, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &DatabaseSelection::close, Qt::UniqueConnection);
    connect(ui->listWidget, QOverload<int>::of(&QListWidget::currentRowChanged),
            this, QOverload<int>::of(&DatabaseSelection::onCurrentRowChanged), Qt::UniqueConnection);

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif

    if (globals().isSystemThemeDark)
        ui->frame->setObjectName("customFrame");

    ui->listWidget->setCurrentRow(0);
    ui->listWidget->setFocus();

}

DatabaseSelection::~DatabaseSelection()
{
    delete ui;
}

void DatabaseSelection::readFileSettings(const QString pathToFile)
{
    // Calea profilului rămâne vizibilă indiferent de tipul conexiunii și de
    // prezența cheilor MySQL goale în fișierele salvate de versiunile noi.
    ui->txtPath->setText(QDir::toNativeSeparators(pathToFile));
    QString connectionType = tr("Necunoscut");
    if (QFileInfo(pathToFile).isReadable()) {
        QSettings settings(pathToFile, QSettings::IniFormat);
        settings.beginGroup(AppSettingsStore::Key::groupIndex);
        const int databaseIndex = settings.value(AppSettingsStore::Key::indexDatabase, 0).toInt();
        if (settings.status() == QSettings::NoError) {
            if (databaseIndex == 1)
                connectionType = QStringLiteral("MySQL");
            else if (databaseIndex == 2) {
                settings.endGroup();
                settings.beginGroup(AppSettingsStore::Key::groupConnection);
                connectionType = settings.value(AppSettingsStore::Key::sqliteEncrypted, false).toBool()
                    ? QStringLiteral("SQLCipher") : QStringLiteral("SQLite");
            }
        }
    }
    ui->txtTypeConnection->setText(tr("Tipul conectării:<br><b><u>%1</u></b>")
                                       .arg(connectionType.toHtmlEscaped()));
}

void DatabaseSelection::onCurrentRowChanged(const int row)
{
    if (row == -1) {
        ui->txtPath->clear();
        ui->txtTypeConnection->clear();
        return;
    }

    QString str_name_file = ui->listWidget->item(row)->data(Qt::DisplayRole).toString();
    const QString file_name = dirConfigPath + "/" + str_name_file + ".conf";
    readFileSettings(file_name);
}

void DatabaseSelection::onConnectToBase()
{
    if (ui->listWidget->currentRow() < 0)
        return;

    QDir file_conf;
    QString str_name_file = ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString();
    QString file_name = dirConfigPath + "/" + str_name_file + ".conf";
    ApplicationPathsContext::instance().setSettingsFilePath(
        file_conf.toNativeSeparators(file_name));
    globals().unknowModeLaunch = false;
    globals().firstLaunch = false;
    globals().moveApp = -1;
    qInfo(logInfo()) << "Selection of connection to base -"
                     << ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString();
    QDialog::accept();
}

void DatabaseSelection::onAddDatabase()
{
    globals().firstLaunch = true;
    QDialog::accept();
}

void DatabaseSelection::onEditSettings()
{
    const QListWidgetItem *currentItem = ui->listWidget->currentItem();
    if (!currentItem)
        return;

    const QString profileName = currentItem->data(Qt::DisplayRole).toString();
    const QString settingsFilePath =
        QDir(dirConfigPath).filePath(profileName + QStringLiteral(".conf"));
    const QFileInfo settingsFileInfo(settingsFilePath);

    if (!settingsFileInfo.exists() || !settingsFileInfo.isFile()) {
        QMessageBox::warning(this,
                             tr("Verificarea fișierului"),
                             tr("Fișierul cu setările conexiunii nu a fost găsit:<br>%1")
                                 .arg(QDir::toNativeSeparators(settingsFilePath)),
                             QMessageBox::Ok);
        return;
    }

    bool opened = false;
#if defined(Q_OS_WIN)
    opened = QProcess::startDetached(QStringLiteral("notepad.exe"),
                                     {QDir::toNativeSeparators(settingsFileInfo.absoluteFilePath())});
#elif defined(Q_OS_LINUX)
    opened = QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFileInfo.absoluteFilePath()));
#else
    opened = QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFileInfo.absoluteFilePath()));
#endif

    if (!opened) {
        QMessageBox::warning(this,
                             tr("Deschiderea fișierului"),
                             tr("Fișierul cu setările conexiunii nu a putut fi deschis:<br>%1")
                                 .arg(QDir::toNativeSeparators(settingsFileInfo.absoluteFilePath())),
                             QMessageBox::Ok);
    }
}

void DatabaseSelection::onRemoveRowListWidget()
{
    if (ui->listWidget->currentRow() == -1)
        return;

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Eliminarea set\304\203rilor"),
                             tr("Dori\310\233i s\304\203 elimina\310\233i fi\310\231ierul:<br>%1 ?")
                                 .arg(ui->txtPath->text()),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton) {
        QString str_name_file = ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::DisplayRole).toString();
        QString file_name = dirConfigPath + "/" + str_name_file + ".conf";
        if (QFileInfo::exists(file_name) && !QFile::remove(file_name)) {
            QMessageBox::critical(this,
                                  tr("Eliminarea setărilor"),
                                  tr("Fișierul nu a putut fi eliminat:<br>%1")
                                      .arg(QDir::toNativeSeparators(file_name)),
                                  QMessageBox::Ok);
            return;
        }
    } else if (messange_box.clickedButton() == noButton) {
        return;
    }

    const int row = ui->listWidget->currentRow();
    delete ui->listWidget->takeItem(row);
}

void DatabaseSelection::updateTimer()
{
    if (this->isVisible())
        timer->stop();
    else
        return;

    QMessageBox messange_box(QMessageBox::Question,
                             tr("Crearea bazei de date"),
                             tr("Adaugarea/crearea bazei de date ?"),
                             QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
    noButton->setStyleSheet(db->getStyleForButtonMessageBox());
    messange_box.exec();

    if (messange_box.clickedButton() == yesButton)
        onAddDatabase();
    else if (messange_box.clickedButton() == noButton)
        return;

}
