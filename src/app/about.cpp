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

#include "about.h"
#include "ui_about.h"
#include "common/maindatabaseconnectioncontext.h"
#include <QAction>
#include <app/mainwindow.h>
#include <common/applicationpathscontext.h>
#include <QSqlQuery>

namespace {
// Versiunile SQLCipher și ale bibliotecii criptografice folosite de driver.
QString sqlCipherPragmaValue(const QString &pragma)
{
    QSqlQuery query(QSqlDatabase::database());
    if (!query.exec(QStringLiteral("PRAGMA %1").arg(pragma)) || !query.next())
        return {};
    return query.value(0).toString().trimmed();
}

QString sqlCipherVersionInfo()
{
    // Câte o versiune pe rând, fără sufixe ("community", data), ca să nu se lărgească fereastra.
    QString info;
    const QString cipherVersion = sqlCipherPragmaValue(QStringLiteral("cipher_version"))
                                      .section(QLatin1Char(' '), 0, 0);
    if (!cipherVersion.isEmpty())
        info += QStringLiteral("\nversion SQLCipher: ") + cipherVersion;

    // ex. "OpenSSL 3.5.9 30 Sep 2026" -> "version OpenSSL: 3.5.9"
    const QStringList provider = sqlCipherPragmaValue(QStringLiteral("cipher_provider_version"))
                                     .split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (provider.size() >= 2)
        info += QStringLiteral("\nversion %1: %2").arg(provider.at(0), provider.at(1));
    else if (provider.size() == 1)
        info += QStringLiteral("\nversion crypto: ") + provider.at(0);
    return info;
}
}

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    db = new DataBase(this);

    // determinam stilul
    QString str_style = globals().isSystemThemeDark
                            ? "style='color:#a6a6a6'"
                            : "";

    // text despre aplicatia
    QString str;
    str.append(tr(R"(
        <div %1>
            <h3 align='center'><b>%2 v%3</b></h3>
            <p>Aplicația <b>USG</b> este o soluție modernă și eficientă destinată gestionării pacienților
            supuși investigațiilor ecografice. Oferă funcționalități avansate pentru evidența completă
            a datelor pacienților, incluzând:</p>
            <ul style="margin-left: 40px;">
                <li>Memorarea informațiilor personale și a datelor de contact;</li>
                <li>Atașarea imaginilor și fișierelor video rezultate în urma investigațiilor;</li>
                <li>Gestionarea centralizată a istoricului medical etc.</li>
            </ul>
            <h3 align='center'><b>Caracteristici principale:</b></h3>
            <ul>
                <li><b>Free și open-source</b> – aplicația este gratuită și codul sursă este disponibil public pentru personalizare și contribuții din partea comunității.</li>
                <li><b>Cross-platform</b> – funcționează pe principalele sisteme de operare: Linux, macOS și Windows.</li>
                <li><b>Suport pentru baze de date</b> – compatibilă cu SQLite3, MySQL și MariaDB, oferind flexibilitate în funcție de nevoile utilizatorilor.</li>
            </ul>
            <h3 align='center'><b>Autorul aplicației:</b></h3>
            <p align='center'>%4, Alovada-Med SRL</p>
        </div>
        )").arg(str_style,
                APPLICATION_NAME,
                USG_VERSION_FULL,
                USG_COMPANY_EMAIL)
               );
    ui->textBrowser_about->setHtml(str);

    // text despre licenta
    QString str_licenses;
    str_licenses.append(tr(R"(
        <div %1>
            <br>
            <p align=center>
            <h4>Acest program este software gratuit; <br>
            îl poți redistribui și/sau modifica în concordanță cu termenii <br>
            GNU Licență Publică Generală cum sunt publicați de <br>
            Free Software Foundation; fie versiunea 3 a licenței, <br>
            sau(după alegerea ta) orice versiune mai actuală.</h4>
            </p>
        </div>
        )").arg(str_style));
    str_licenses.append(tr(R"(
        <div %1>
            <p align=center>
            Componente terțe: Qt (LGPLv3), LimeReport (LGPLv3), <br>
            SQLCipher (BSD-3-Clause), OpenSSL (Apache-2.0). <br>
            Textele licențelor se află în folderul <b>licenses</b> al aplicației.
            </p>
        </div>
        )").arg(str_style));
    ui->textBrowser_licenses->setText(str_licenses);

#if defined(Q_OS_WIN)
    ui->textBrowser_about->setStyleSheet("font-size: 15px;");
    ui->textBrowser_licenses->setStyleSheet("font-size: 15px;");
#elif defined(Q_OS_LINUX)
    ui->textBrowser_about->setStyleSheet("font-size: 14px;");
    ui->textBrowser_licenses->setStyleSheet("font-size: 15px;");
#endif

    const MainDatabaseConnectionData connection = MainDatabaseConnectionContext::instance().data();

    QDir dir;
    if (connection.backend == MainDatabaseBackend::SQLite) {
        ui->dir_app->setText(dir.toNativeSeparators(connection.sqliteDatabasePath));
        ui->dir_img->setText(dir.toNativeSeparators(connection.imageDatabasePath));
    } else {
        ui->dir_app->setVisible(false);
        ui->dir_img->setVisible(false);
        ui->label_2->setVisible(false);
        ui->label_4->setVisible(false);
    }
    ui->dir_settings->setText(dir.toNativeSeparators(ApplicationPathsContext::instance().data().settingsFilePath));
    ui->dir_templates->setText(dir.toNativeSeparators(ApplicationPathsContext::instance().data().templatesDirectory));
    ui->dir_reports->setText(dir.toNativeSeparators(ApplicationPathsContext::instance().data().reportsDirectory));
    ui->dir_logs->setText(dir.toNativeSeparators(ApplicationPathsContext::instance().data().logFilePath));

    ui->text_versionQt->setText("version Qt: " QT_VERSION_STR);
    if (connection.backend == MainDatabaseBackend::MariaDb)
        ui->text_version_SQLite->setText("version MySQL: " + db->getVersionMySQL());
    else if (connection.sqliteEncrypted)
        ui->text_version_SQLite->setText("version SQLite: " + db->getVersionSQLite()
                                         + sqlCipherVersionInfo());
    else
        ui->text_version_SQLite->setText("version SQLite: " + db->getVersionSQLite());

    connect(ui->pushButton, &QAbstractButton::clicked, this, &About::close);

    if (globals().isSystemThemeDark)
        ui->frame->setObjectName("customFrame");
}

About::~About()
{
    delete db;
    delete ui;
}
