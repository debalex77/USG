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

#include "asistanttipapp.h"
#include "data/globals.h"
#include "ui_asistanttipapp.h"

AsistantTipApp::AsistantTipApp(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AsistantTipApp)
{
    ui->setupUi(this);

    setWindowTitle(tr("Asistentul aplicației."));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    db = new DataBase(this);

    connect(ui->btnNext, &QAbstractButton::clicked, this, &AsistantTipApp::stepNext);
    connect(ui->btnPreview, &QAbstractButton::clicked, this, &AsistantTipApp::stepPreview);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &AsistantTipApp::onClose);

    ui->btnPreview->setStyleSheet("width: 95px;");
    ui->btnNext->setStyleSheet("width: 95px;");
    ui->btnClose->setStyleSheet("width: 95px;");

    ui->show_launch->setChecked(globals().showAsistantHelper);

    setTextSteps();

    str_style = globals().isSystemThemeDark
                    ? "<div style='color:#b5b5b5; font-size:11pt;'>"
                    : "<div style='font-size:11pt;'>";

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif
    if (globals().isSystemThemeDark)
        ui->frame->setObjectName("customFrame");

}

AsistantTipApp::~AsistantTipApp()
{
    delete db;
    delete ui;
}

void AsistantTipApp::setStep(int m_step)
{
    if (m_step == 0)
        m_step += 1;

    current_step = m_step;
    setTipText();
}

void AsistantTipApp::stepNext()
{
    if ((current_step + 1) > max_step)
        return;

    current_step += 1;
    setTipText();
}

void AsistantTipApp::stepPreview()
{
    if ((current_step - 1) == 0)
        return;

    current_step -= 1;
    setTipText();
}

void AsistantTipApp::onClose()
{
    bool m_show_launch = (ui->show_launch->isChecked()) ? true : false;

    if (m_show_launch != globals().showAsistantHelper){
        QSqlQuery qry;
        qry.prepare(R"(
            UPDATE userPreferences SET
                showAsistantHelper = ?
            WHERE id_users = ?
        )");
        qry.bindValue(0, ui->show_launch->isChecked());
        qry.bindValue(1, globals().idUserApp);
        if (! qry.exec())
            qWarning(logWarning()) << this->metaObject()->className()
                                   << "[onClose]:"
                                   << tr("Nu au fost actualizate date de prezentare a asistentului de sfaturi %1")
                                          .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
        globals().showAsistantHelper = ui->show_launch->isChecked();
    }
    this->close();
}

void AsistantTipApp::setTextSteps()
{
    text_step1 =
        tr(R"(
            <u><b>Preferințele utilizatorului</b></u> - configurarea setărilor utilizatorului:
                <li>&nbsp;&nbsp; - Setările generale - setarea aplicației implicite, logotipul etc.</li>
                <li>&nbsp;&nbsp; - Lansarea/închiderea - setările la lansarea sau închiderea aplicației.</li>
                <li>&nbsp;&nbsp; - Setările documentelor - setări pentru documente.</li>
                <li>&nbsp;&nbsp; - Setările neredactabile - setările informative.</li>
        )");

    text_step2 =
        tr(R"(
            <u><b>Jurnalul de logare</b></u> - vizualizați jurnalul de logare.<br>
            Deschideți Setările aplicației -> Tabul 'Fișiere de logare' -> alegeți fișierul pentru a vizualiza
            jurnalul.
        )");

    text_step3 =
        tr(R"(
            <u><b>Logotipul organizației</b></u> - folosiți logotipul organizației.<br>
            Atașați logotipul organizației în setările utilizatorului pentru a prezenta în formele de tipar a documentelor și în rapoarte.
        )");

    text_step4 =
        tr(R"(
            <u><b>Ștampila organizației</b></u> - folosiți ștampila organizației.<br>
            Deschideți catalogul "Centre medicale" și găsiți organizația -> tabul "Ștampila" - atașați ștampila,
            pentru a prezenta în formele de tipar a documentelor și în rapoarte.
        )");

    text_step5 =
        tr(R"(
            <u><b>Ștampila și semnătura doctorului</b></u> - folosiți ștampila sau semnătura doctorului pentru a prezenta în formele
            de tipar a documentelor și în rapoarte.<br>
            Deschideți catalogul "Doctori" și alegeți persoana -> tabul "Ștampila, semnătura" - atașați ștampila sau semnărura.
        )");
}

void AsistantTipApp::setTipText()
{
    QString content;

    switch (current_step) {
    case 1:
        content = text_step1;
        break;
    case 2:
        content = text_step2;
        break;
    case 3:
        content = text_step3;
        break;
    case 4:
        content = text_step4;
        break;
    case 5:
        content = text_step5;
        break;
    default:
        content = "";
        break;
    }

    QString finalHtml = str_style + content + "</div>";
    ui->tip_text->setHtml(finalHtml);
}
