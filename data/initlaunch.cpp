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

#include "initlaunch.h"
#include "loggingcategories.h"
#include "ui_initlaunch.h"

InitLaunch::InitLaunch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitLaunch)
{
    ui->setupUi(this);

    setWindowTitle(tr("Lansarea aplicației %1").arg("[*]"));

    nameLocale = QLocale::system().name();
//    nameLocale.replace(QString("_"), QString("-")); // schimbam simbolul

    // traducem aplicatia
    translator = new QTranslator(this);

    ui->comboLangApp->addItems(QStringList() << "ro_RO" << "ru_RU");

    int indexLangApp = (nameLocale == "ro-RO") ? 0 : 1;
    ui->comboLangApp->setCurrentIndex(indexLangApp);
    changeIndexComboLangApp(indexLangApp);

    connect(ui->comboLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InitLaunch::changeIndexComboLangApp);
    connect(ui->comboLangApp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InitLaunch::dataWasModified);

    connect(ui->radBtnFirstLaunch, &QRadioButton::clicked, this, &InitLaunch::onClickedRadBtnFirstLaunch);
    connect(ui->radBtnFirstLaunch, &QRadioButton::clicked, this, &InitLaunch::dataWasModified);
    connect(ui->radBtnAppMove, &QRadioButton::clicked, this, &InitLaunch::onClickedRadBtnAppMove);
    connect(ui->radBtnAppMove, &QRadioButton::clicked, this, &InitLaunch::dataWasModified);

    connect(ui->btnOK, &QPushButton::clicked, this, &InitLaunch::onWritingData);
    connect(ui->btnClose, &QPushButton::clicked, this, &InitLaunch::onClose);

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
#endif
}

InitLaunch::~InitLaunch()
{
    delete translator;
    delete ui;
}

void InitLaunch::dataWasModified()
{
    setWindowModified(true);
}

QString InitLaunch::getStyleForButtonMessageBox()
{
    return QString("QPushButton "
                   "{"
                   "  min-width: 60px;"
                   "  border: 1px solid rgba(0, 0, 0, 0.2);"
                   "  border-radius: 8px;"
                   "  background-color: #f5f5f5;"
                   "  color: #000000;"
                   "  font-size: 13px;"
                   "  padding: 4px 10px;"
                   "  min-width: 80px;"
                   "}"
                   "QPushButton:hover "
                   "{"
                   "  background-color: #e0e0e0;"
                   "}"
                   "QPushButton:pressed "
                   "{"
                   "  background-color: #d0d0d0;"
                   "}");
}

void InitLaunch::changeIndexComboLangApp(const int _index)
{
    nameLocale = (_index == 0) ? "ro_RO" : "ru_RU"; // setam nameLocal dupa index alex
    globals().langApp = nameLocale;                  // setam variabila globala
    if (translator->load(QLocale(globals().langApp), QLatin1String("USG"), QLatin1String("_"), QLatin1String(":/i18n"))) {
        QCoreApplication::installTranslator(translator);
    }
}

void InitLaunch::onClickedRadBtnFirstLaunch()
{
    globals().firstLaunch = true;
    globals().moveApp = 0;
}

void InitLaunch::onClickedRadBtnAppMove()
{
    globals().firstLaunch = false;
    globals().moveApp = 1;
}

void InitLaunch::onWritingData()
{
    globals().unknowModeLaunch = false;
    globals().firstLaunch = ui->radBtnFirstLaunch->isChecked();
    globals().moveApp     = (ui->radBtnAppMove->isChecked()) ? 1 : 0;
    if (globals().moveApp == 1)
        qInfo(logInfo()) << tr("Aplicatia a fost transferata.");
    else
        qInfo(logInfo()) << tr("Prima lansare a aplicatiei.");

    QDialog::accept();
}

void InitLaunch::onClose()
{
    this->close();
}

void InitLaunch::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(getStyleForButtonMessageBox());
        noButton->setStyleSheet(getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            onWritingData();
            event->accept();
        } else if (messange_box.clickedButton() == noButton) {
            event->accept();
        } else if (messange_box.clickedButton() == cancelButton) {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void InitLaunch::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Lansarea aplicației %1").arg("[*]"));
    }
}
