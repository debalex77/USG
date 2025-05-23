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

#include "normograms.h"
#include "ui_normograms.h"
#include <QScreen>

Normograms::Normograms(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Normograms)
{
    ui->setupUi(this);

    setWindowTitle(tr("Normograme"));

    setModels();

    initConnections();
    updateStyleBtnPressed();

    // initial setam normograma NT
    ui->stackedWidget->setCurrentIndex(page_nt);
    ui->btnNT->setFocus(); // focusul pe btn NT

    // ********************************************************
    // resize and move windows
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    this->resize(720, 480);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);

    // ********************************************************
    // resize frame btn

    ui->frame_tables->resize(500, ui->frame_tables->height());

    if (globals().isSystemThemeDark){
        ui->frame_btn->setStyleSheet(
            "background-color: #2b2b2b;"
            "border: 1px solid #555;"
            "border-radius: 5px;"
            );
        ui->frame_tables->setObjectName("customFrame");
        ui->frame_tables->setStyleSheet("QFrame#customFrame "
                                       "{ "
                                       "  background-color: #2b2b2b; "
                                       "  border: 1px solid #555; /* Linie subțire gri */ "
                                       "  border-radius: 5px; "
                                       "}");
    }
}

Normograms::~Normograms()
{
    delete model_nt;
    delete model_bn;
    delete model_indexAmniotic;
    delete model_doppler_uterine;
    delete model_doppler_umbelicale;
    delete db;
    delete ui;
}

void Normograms::setModels()
{
    db = new DataBase(this);

    QSqlQuery qry;
    QVariantMap norm_data;

    // --- NT

    model_nt = new VariantMapTableModel(this);
    model_nt->setTypeNormograms(VariantMapTableModel::TypeNormograms::NT_BN);
    model_nt->registerColumn(new SimpleColumn("id"));
    model_nt->registerColumn(new SimpleColumn("crl"));
    model_nt->registerColumn(new SimpleColumn("5_centile"));
    model_nt->registerColumn(new SimpleColumn("50_centile"));
    model_nt->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "normograma_nt");
    if (qry.exec() && qry.next()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_nt->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableNT->setModel(model_nt);
    ui->tableNT->hideColumn(0);

    // --- BN
    norm_data.clear();
    model_bn = new VariantMapTableModel(this);
    model_bn->setTypeNormograms(VariantMapTableModel::TypeNormograms::NT_BN);
    model_bn->registerColumn(new SimpleColumn("id"));
    model_bn->registerColumn(new SimpleColumn("crl"));
    model_bn->registerColumn(new SimpleColumn("5_centile"));
    model_bn->registerColumn(new SimpleColumn("50_centile"));
    model_bn->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "normograma_bn");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_bn->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableBN->setModel(model_bn);
    ui->tableBN->hideColumn(0);

    // --- index amniotic
    norm_data.clear();
    model_indexAmniotic = new VariantMapTableModel(this);
    model_indexAmniotic->setTypeNormograms(VariantMapTableModel::TypeNormograms::INDEX_AMNIOTIC);
    model_indexAmniotic->registerColumn(new SimpleColumn("id"));
    model_indexAmniotic->registerColumn(new SimpleColumn("crl"));
    model_indexAmniotic->registerColumn(new SimpleColumn("5_centile"));
    model_indexAmniotic->registerColumn(new SimpleColumn("50_centile"));
    model_indexAmniotic->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "index_amniotic");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_indexAmniotic->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableIndexAmniotic->setModel(model_indexAmniotic);
    ui->tableIndexAmniotic->hideColumn(0);

    // --- a.uterine
    norm_data.clear();
    model_doppler_uterine = new VariantMapTableModel(this);
    model_doppler_uterine->setTypeNormograms(VariantMapTableModel::TypeNormograms::DOPPLER);
    model_doppler_uterine->registerColumn(new SimpleColumn("id"));
    model_doppler_uterine->registerColumn(new SimpleColumn("crl"));
    model_doppler_uterine->registerColumn(new SimpleColumn("5_centile"));
    model_doppler_uterine->registerColumn(new SimpleColumn("50_centile"));
    model_doppler_uterine->registerColumn(new SimpleColumn("95_centile"));
    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "uterine_PI");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_doppler_uterine->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableUterine->setModel(model_doppler_uterine);
    ui->tableUterine->hideColumn(0);

    // --- a.umbelicale
    norm_data.clear();
    model_doppler_umbelicale = new VariantMapTableModel(this);
    model_doppler_umbelicale->setTypeNormograms(VariantMapTableModel::TypeNormograms::DOPPLER);
    model_doppler_umbelicale->registerColumn(new SimpleColumn("id"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("crl"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("5_centile"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("50_centile"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("95_centile"));
    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "umbilical_PI");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_doppler_umbelicale->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableUmbelical->setModel(model_doppler_umbelicale);
    ui->tableUmbelical->hideColumn(0);

}

// *******************************************************************
// **************** INITIEREA CONEXIUNILOR SI PROCESAREA LOR *********

void Normograms::initConnections()
{
    connect(ui->btnNT, &QAbstractButton::clicked, this, &Normograms::click_btn_NT);
    connect(ui->btnBN, &QAbstractButton::clicked, this, &Normograms::click_btn_BN);
    connect(ui->btnIndexAmniotic, &QAbstractButton::clicked, this, &Normograms::click_btn_indexAmniotic);
    connect(ui->btnDopplerUterine, &QAbstractButton::clicked, this, &Normograms::click_btn_uterine);
    connect(ui->btnDopplerUmbelical, &QAbstractButton::clicked, this, &Normograms::click_btn_umbelicale);
}

void Normograms::updateStyleBtnPressed()
{
    QString style_pressed;
    QString style_unpressed;

    if (globals().isSystemThemeDark) {

        style_pressed = "QCommandLinkButton "
                        "{"
                        "  background: #5b5b5b;"
                        "  border: 1px inset #00baff;"
                        "  border-color: #007acc;"
                        "  color: #ffffff;"
                        "}"
                        "QCommandLinkButton:hover "
                        "{"
                        "  background-color: #4b4b4b;"
                        "}"
                        "QCommandLinkButton::icon "
                        "{"
                        "  background-color: #ffffff;" // Fundal alb pentru icon
                        "  border-radius: 4px;"
                        "  border: 1px solid #cccccc;"
                        "}";

        style_unpressed = "QCommandLinkButton "
                          "{"
                          "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #4b4b4b, stop: 1 #3c3c3c);"
                          "  border: 0px;"
                          "  color: #ffffff;"
                          "}"
                          "QCommandLinkButton::icon "
                          "{"
                          "  background-color: #e0e0e0;" // Fundal mai deschis pentru pictogramă
                          "  border-radius: 4px;"
                          "  border: 1px solid #cccccc;"
                          "  padding: 2px;"
                          "}";

    } else {

        style_pressed = "background: 0px #C2C2C3; "
                        "border: 1px inset blue; "
                        "border-color: navy;";

        style_unpressed = "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                          "border: 0px #8f8f91;";
    }

    if (ui->stackedWidget->currentIndex() == page_nt){
        ui->btnNT->setStyleSheet(style_pressed);
    } else {
        ui->btnNT->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_bn){
        ui->btnBN->setStyleSheet(style_pressed);
    } else {
        ui->btnBN->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_index_amniotic){
        ui->btnIndexAmniotic->setStyleSheet(style_pressed);
    } else {
        ui->btnIndexAmniotic->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_uterine){
        ui->btnDopplerUterine->setStyleSheet(style_pressed);
    } else {
        ui->btnDopplerUterine->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_umbelicale){
        ui->btnDopplerUmbelical->setStyleSheet(style_pressed);
    } else {
        ui->btnDopplerUmbelical->setStyleSheet(style_unpressed);
    }
}

void Normograms::click_btn_NT()
{
    ui->stackedWidget->setCurrentIndex(page_nt);
    updateStyleBtnPressed();
}

void Normograms::click_btn_BN()
{
    ui->stackedWidget->setCurrentIndex(page_bn);
    updateStyleBtnPressed();
}

void Normograms::click_btn_indexAmniotic()
{
    ui->stackedWidget->setCurrentIndex(page_index_amniotic);
    updateStyleBtnPressed();
}

void Normograms::click_btn_uterine()
{
    ui->stackedWidget->setCurrentIndex(page_uterine);
    updateStyleBtnPressed();
}

void Normograms::click_btn_umbelicale()
{
    ui->stackedWidget->setCurrentIndex(page_umbelicale);
    updateStyleBtnPressed();
}
