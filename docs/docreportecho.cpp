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

#include "docreportecho.h"
#include "ui_docreportecho.h"

#include <QBuffer>
#include <QDirIterator>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QScreen>
#include <QStandardPaths>

#include <catalogs/catforsqltablemodel.h>

#include <customs/custommessage.h>

#include <threads/docsyncworker.h>

DocReportEcho::DocReportEcho(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocReportEcho)
{
    ui->setupUi(this);

    qApp->installEventFilter(this);

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        widget->installEventFilter(this);
    }

    foreach (QWidget *widget, findChildren<QWidget*>()) {
        if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QComboBox *>(widget) || qobject_cast<QPlainTextEdit *>(widget)) {
            widget->setFocusPolicy(Qt::StrongFocus);
        }
        if (QPushButton *button = qobject_cast<QPushButton *>(widget)) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
    }

    setWindowTitle(tr("Raport ecografic %1").arg("[*]"));

    if (globals().showDocumentsInSeparatWindow)
        setWindowFlags(Qt::Window);

    // ******************************************************************
    db        = new DataBase(this);
    popUp     = new PopUp(this);
    timer     = new QTimer(this);                       // alocam memoria
    completer = new QCompleter(this);
    modelPatients = new QStandardItemModel(completer);
    proxyPatient  = new BaseSortFilterProxyModel(this);
    data_percentage = new DataPercentage(this);

    modelOrganization   = new QSqlQueryModel(this);
    modelOrgansInternal = new QSqlQueryModel(this);
    modelUrinarySystem  = new QSqlQueryModel(this);
    modelProstate       = new QSqlQueryModel(this);
    modelGynecology     = new QSqlQueryModel(this);
    modelBreast         = new QSqlQueryModel(this);
    modelThyroid        = new QSqlQueryModel(this);
    modelGestationO     = new QSqlQueryModel(this);
    modelGestation1     = new QSqlQueryModel(this);
    modelGestation2     = new QSqlQueryModel(this);

    // ******************************************************************

    ui->editDocDate->setDisplayFormat("dd.MM.yyyy hh:mm:ss");
    ui->editDocDate->setCalendarPopup(true);

    initFooterDoc();    // setam numele autorului + imaginea
    initSetCompleter(); // initializam setarea completerului

    // ******************************************************************
    group_btn_prostate = new QButtonGroup(this);          // grupam btn ”RadioButtom”
    group_btn_prostate->addButton(ui->prostate_radioBtn_transabdom);
    group_btn_prostate->addButton(ui->prostate_radioBtn_transrectal);
    ui->prostate_radioBtn_transabdom->setChecked(true);

    group_btn_gynecology = new QButtonGroup(this);
    group_btn_gynecology->addButton(ui->gynecology_btn_transabdom);
    group_btn_gynecology->addButton(ui->gynecology_btn_transvaginal);
    ui->gynecology_btn_transvaginal->setChecked(true);

    group_btn_gestation0 = new QButtonGroup(this);
    group_btn_gestation0->addButton(ui->gestation0_view_good, 0);
    group_btn_gestation0->addButton(ui->gestation0_view_medium, 1);
    group_btn_gestation0->addButton(ui->gestation0_view_difficult, 2);
    ui->gestation0_view_medium->setChecked(true);

    group_btn_gestation1 = new QButtonGroup(this);
    group_btn_gestation1->addButton(ui->gestation1_view_good, 0);
    group_btn_gestation1->addButton(ui->gestation1_view_medium, 1);
    group_btn_gestation1->addButton(ui->gestation1_view_difficult, 2);
    ui->gestation1_view_medium->setChecked(true);

    group_btn_gestation2 = new QButtonGroup(this);
    group_btn_gestation2->addButton(ui->gestation2_trimestru2, 0);
    group_btn_gestation2->addButton(ui->gestation2_trimestru3, 1);
    ui->gestation2_trimestru2->setChecked(true);
    clickedGestation2Trimestru(0);

    // ******************************************************************

    ui->image1->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image1->setTextFormat(Qt::RichText);
    ui->image1->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image1, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImage1));

    ui->image2->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image2->setTextFormat(Qt::RichText);
    ui->image2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image2, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImage2));

    ui->image3->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image3->setTextFormat(Qt::RichText);
    ui->image3->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image3, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImage3));

    ui->image4->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image4->setTextFormat(Qt::RichText);
    ui->image4->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image4, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImage4));

    ui->image5->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image5->setTextFormat(Qt::RichText);
    ui->image5->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image5, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImage5));

    // ******************************************************************
    constructionFormVideo(); // forma video

    // ******************************************************************
    initConnections();  // initiem conectarile
    connections_tags();

    // ******************************************************************
    ui->gynecology_text_date_menstruation->setText("");

    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();

    this->resize(1350, 800);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);

    ui->frame_table->resize(900, ui->frame_btn->height());

    if (globals().isSystemThemeDark){
        ui->frame_btn->setStyleSheet(
            "background-color: #2b2b2b;"
            "border: 1px solid #555;"
            "border-radius: 5px;"
        );
        ui->frame_table->setObjectName("customFrame");
        ui->frame_table->setStyleSheet("QFrame#customFrame "
                                       "{ "
                                       "  background-color: #2b2b2b; "
                                       "  border: 1px solid #555; /* Linie subțire gri */ "
                                       "  border-radius: 5px; "
                                       "}");
        ui->frame_3->setObjectName("customFrame");
        ui->frame_3->setStyleSheet("QFrame#customFrame "
                                       "{ "
                                       "  background-color: #2b2b2b; "
                                       "  border: 1px solid #555; /* Linie subțire gri */ "
                                       "  border-radius: 5px; "
                                       "}");
        ui->frame_4->setObjectName("customFrame");
        ui->frame_4->setStyleSheet("QFrame#customFrame "
                                       "{ "
                                       "  background-color: #2b2b2b; "
                                       "  border: 1px solid #555; /* Linie subțire gri */ "
                                       "  border-radius: 5px; "
                                       "}");
        ui->frameVideo->setObjectName("customFrame");
        ui->frameVideo->setStyleSheet("QFrame#customFrame "
                                       "{ "
                                       "  background-color: #2b2b2b; "
                                       "  border: 1px solid #555; /* Linie subțire gri */ "
                                       "  border-radius: 5px; "
                                       "}");
    }
}

DocReportEcho::~DocReportEcho()
{
    delete popUp;
    delete timer;
    delete modelOrganization;
    delete modelOrgansInternal;
    delete modelUrinarySystem;
    delete modelProstate;
    delete modelGynecology;
    delete modelBreast;
    delete modelThyroid;
    delete modelGestationO;
    delete modelGestation1;
    delete modelGestation2;
    delete modelPatients;
    delete proxyPatient;
    delete completer;
    delete player;
    delete data_percentage;
#if defined(Q_OS_LINUX) || defined (Q_OS_WIN)
    delete videoWidget;
#elif defined(Q_OS_MACOS)
    delete scene;
    delete view;
#endif
    delete db;
    delete ui;
}

// *******************************************************************
// **************** VALIDAREA SI PRINTAREA DIN ALTE OBIECTE***********

void DocReportEcho::onPrintDocument(Enums::TYPE_PRINT _typeReport, QString &filePDF)
{
    onPrint(_typeReport, filePDF);
    this->close();
}

QString DocReportEcho::getNumberDocReport()
{
    return ui->editDocNumber->text();
}

void DocReportEcho::m_onWritingData()
{
    onWritingData();
}

void DocReportEcho::dataWasModified()
{
    setWindowModified(true);
}

void DocReportEcho::updateTimer()
{
    disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);
    disconnect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
    QDateTime time = QDateTime::currentDateTime();
    ui->editDocDate->setDateTime(time);
    connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);
    connect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
}

void DocReportEcho::onDateTimeChanged()
{
    disconnect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
}

void DocReportEcho::onDateLMPChanged(QDate m_date)
{
    QString str_vg = calculateGestationalAge(m_date);
    if (str_vg == nullptr)
        return;

    // calculam varsta gestationala si data probabila a nasterii
    if (m_gestation0) {
        ui->gestation0_gestation->setText(str_vg);
        ui->gestation0_probableDateBirth->setDate(calculateDueDate(m_date));
    } else if (m_gestation1) {
        ui->gestation1_gestation->setText(str_vg);
        ui->gestation1_probableDateBirth->setDate(calculateDueDate(m_date));
    } else if (m_gestation2) {
        ui->gestation2_gestation_age->setText(str_vg);
    }
}

// *******************************************************************
// **************** ATASAREA IMAGINELOR ******************************

bool DocReportEcho::loadFile(const QString &fileName, const int numberImage)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this,
                                 QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    // Redimensionăm și transformăm imaginea - scalarea pentru baza de date (calitatea imaginei)
    QImage scaledImage = newImage.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Conversie imagine în QByteArray (JPEG 90%)
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    scaledImage.save(&buffer, "JPEG", 90); // daca 'PNG' -> se mareste de 2-3 ori dimensiunea fisierului + 90% calitatea

    // Afișăm imaginea în interfață
    QPixmap pixmap;
    pixmap.loadFromData(imageData, "JPEG");

    // setam imaginea
    switch (numberImage) {
    case n_image1: ui->image1->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
    case n_image2: ui->image2->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
    case n_image3: ui->image3->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
    case n_image4: ui->image4->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
    case n_image5: ui->image5->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation)); break;
    default: return false;
    }

    // prezentam cate imagini sunt afisate
    setCountImages(m_count_images + 1);

    // verificam daca este setat drumul spre baza de date a imaginilor (slite)
    if (globals().thisSqlite &&
            ! QFile(globals().pathImageBaseAppSettings).exists()) {
        QMessageBox::information(this,
                                 tr("Verificarea setărilor"),
                                 tr("Imaginea nu este salvată în baza de date !!!<br>"
                                    "Nu este indicată localizarea bazei de date a imaginilor.<br>"
                                    "Deschideți setările aplicației și indicați drumul spre baza de date a imaginilor."),
                                 QMessageBox::Ok);
        qInfo(logInfo()) << tr("Imaginea nu este salvata. Nu este indicat drumul spre baza de date a imaginilor.");
        return false;
    }

    // Salvăm direct imaginea din QByteArray
    if (db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id),
                            globals().thisSqlite ? db->getDatabaseImage() : db->getDatabase())) {
        updateImageIntoTableimagesReports(imageData, numberImage);
    } else {
        insertImageIntoTableimagesReports(imageData, numberImage);
    }

    return true;
}

void DocReportEcho::loadImageOpeningDocument()
{
    disconnect(ui->comment_image1, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->comment_image2, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->comment_image3, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->comment_image4, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->comment_image5, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);

    // de modificat solicitarea -> solicitarea unica pu toate imaginile + comentarii
    QSqlQuery qry((globals().thisMySQL) ? db->getDatabase() : db->getDatabaseImage());
    qry.prepare("SELECT * FROM imagesReports WHERE id_reportEcho = ?");
    qry.addBindValue(m_id);
    if (! qry.exec()){
        qWarning(logWarning()) << tr("Eroare de executare a solicitarii 'loadImageOpeningDocument': %1").arg(qry.lastError().text());
    } else {
        if (qry.next()){
            QSqlRecord rec = qry.record();
            QByteArray outByteArray1 = QByteArray::fromBase64(qry.value(rec.indexOf("image_1")).toByteArray());
            QByteArray outByteArray2 = QByteArray::fromBase64(qry.value(rec.indexOf("image_2")).toByteArray());
            QByteArray outByteArray3 = QByteArray::fromBase64(qry.value(rec.indexOf("image_3")).toByteArray());
            QByteArray outByteArray4 = QByteArray::fromBase64(qry.value(rec.indexOf("image_4")).toByteArray());
            QByteArray outByteArray5 = QByteArray::fromBase64(qry.value(rec.indexOf("image_5")).toByteArray());

            QPixmap outPixmap1;
            if (! outByteArray1.isEmpty() && outPixmap1.loadFromData(outByteArray1, "jpeg")){
                ui->image1->setPixmap(outPixmap1.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap2 = QPixmap();
            if (! outByteArray2.isEmpty() && outPixmap2.loadFromData(outByteArray2, "jpeg")){
                ui->image2->setPixmap(outPixmap2.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap3 = QPixmap();
            if (! outByteArray3.isEmpty() && outPixmap3.loadFromData(outByteArray3)){
                ui->image3->setPixmap(outPixmap3.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap4 = QPixmap();
            if (! outByteArray4.isEmpty() && outPixmap4.loadFromData(outByteArray4)){
                ui->image4->setPixmap(outPixmap4.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap5 = QPixmap();
            if (! outByteArray5.isEmpty() && outPixmap5.loadFromData(outByteArray5)){
                ui->image5->setPixmap(outPixmap5.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                setCountImages(m_count_images + 1);
            }
            ui->comment_image1->setPlainText(qry.value(rec.indexOf("comment_1")).toString());
            ui->comment_image2->setPlainText(qry.value(rec.indexOf("comment_2")).toString());
            ui->comment_image3->setPlainText(qry.value(rec.indexOf("comment_3")).toString());
            ui->comment_image4->setPlainText(qry.value(rec.indexOf("comment_4")).toString());
            ui->comment_image5->setPlainText(qry.value(rec.indexOf("comment_5")).toString());
        }
    }

    connect(ui->comment_image1, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image2, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image3, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image4, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image5, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpeg");
}

void DocReportEcho::onLinkActivatedForOpenImage1(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca imaginea este necesar de validat documentul.<br>"
                                        "Dori\310\233i s\304\203 valida\310\233i documentul ?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (YesNo == QMessageBox::Yes)
            onWritingData();
        else
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), n_image1);
    dialog.close();
}

void DocReportEcho::onLinkActivatedForOpenImage2(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca imaginea este necesar de validat documentul.<br>"
                                        "Dori\310\233i s\304\203 valida\310\233i documentul ?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (YesNo == QMessageBox::Yes)
            onWritingData();
        else
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), n_image2);
    dialog.close();
}

void DocReportEcho::onLinkActivatedForOpenImage3(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca imaginea este necesar de validat documentul.<br>"
                                        "Dori\310\233i s\304\203 valida\310\233i documentul ?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (YesNo == QMessageBox::Yes)
            onWritingData();
        else
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), n_image3);
    dialog.close();
}

void DocReportEcho::onLinkActivatedForOpenImage4(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca imaginea este necesar de validat documentul.<br>"
                                        "Dori\310\233i s\304\203 valida\310\233i documentul ?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (YesNo == QMessageBox::Yes)
            onWritingData();
        else
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), n_image4);
    dialog.close();
}

void DocReportEcho::onLinkActivatedForOpenImage5(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca imaginea este necesar de validat documentul.<br>"
                                        "Dori\310\233i s\304\203 valida\310\233i documentul ?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (YesNo == QMessageBox::Yes)
            onWritingData();
        else
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), n_image5);
    dialog.close();
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA IMAGINELOR ***************

void DocReportEcho::insertImageIntoTableimagesReports(const QByteArray &imageData, const int numberImage)
{
    // verificam daca sunt date
    if (imageData.isEmpty()){
        qWarning(logWarning()) << tr("[insertImageIntoTableimagesReports] - nu sunt determinate date imaginei pentru salvarea in baza de date.");
        return;
    }

    // anuntam variabile
    QByteArray img1, img2, img3, img4, img5;

    // determinam date dupa nr.imaginei
    switch (numberImage) {
    case n_image1: img1 = imageData; break;
    case n_image2: img2 = imageData; break;
    case n_image3: img3 = imageData; break;
    case n_image4: img4 = imageData; break;
    case n_image5: img5 = imageData; break;
    }

    // crearea solicitarii
    QSqlQuery qry(globals().thisSqlite ? db->getDatabaseImage() : db->getDatabase());
    qry.prepare(R"(
        INSERT INTO imagesReports (
            id_reportEcho, id_orderEcho, id_patients,
            image_1, image_2, image_3, image_4, image_5,
            comment_1, comment_2, comment_3, comment_4, comment_5,
            id_user
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )");

    qry.addBindValue(m_id);
    qry.addBindValue(m_id_docOrderEcho);
    qry.addBindValue(m_idPacient);
    qry.addBindValue(img1.isEmpty() ? QVariant() : img1.toBase64());
    qry.addBindValue(img2.isEmpty() ? QVariant() : img2.toBase64());
    qry.addBindValue(img3.isEmpty() ? QVariant() : img3.toBase64());
    qry.addBindValue(img4.isEmpty() ? QVariant() : img4.toBase64());
    qry.addBindValue(img5.isEmpty() ? QVariant() : img5.toBase64());

    qry.addBindValue(ui->comment_image1->toPlainText().isEmpty() ? QVariant() : ui->comment_image1->toPlainText());
    qry.addBindValue(ui->comment_image2->toPlainText().isEmpty() ? QVariant() : ui->comment_image2->toPlainText());
    qry.addBindValue(ui->comment_image3->toPlainText().isEmpty() ? QVariant() : ui->comment_image3->toPlainText());
    qry.addBindValue(ui->comment_image4->toPlainText().isEmpty() ? QVariant() : ui->comment_image4->toPlainText());
    qry.addBindValue(ui->comment_image5->toPlainText().isEmpty() ? QVariant() : ui->comment_image5->toPlainText());
    qry.addBindValue(m_idUser);

    if (qry.exec()) {
        popUp->setPopupText(tr("Imaginea a fost salvată cu succes în baza de date."));
        popUp->show();
        qInfo(logInfo()) << QStringLiteral("Documentului 'Raport ecografic' nr.%1 a fost atasata imaginea %2.")
                            .arg(ui->editDocNumber->text(),
                                 QString::number(numberImage));
    } else {
        QMessageBox::warning(this, tr("Inserarea imaginei"),
                             tr("Imaginea nu a fost salvată în baza de date!"),
                             QMessageBox::Ok);
        qCritical(logCritical()) << QStringLiteral("Eroare de atasare a imaginei la documentul 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text())
                                 << qry.lastError().text();
    }

}

void DocReportEcho::updateImageIntoTableimagesReports(const QByteArray &imageData, const int numberImage)
{
    // verificam daca sunt date
    if (imageData.isEmpty()){
        qWarning(logWarning()) << tr("[updateImageIntoTableimagesReports] - nu sunt determinate date imaginei pentru actualizarea in baza de date.");
        return;
    }

    // determinam comentariu
    QVariant m_comment;
    switch (numberImage) {
    case n_image1:
        m_comment = ui->comment_image1->toPlainText().isEmpty() ? QVariant() : ui->comment_image1->toPlainText();
        break;
    case n_image2:
        m_comment = ui->comment_image2->toPlainText().isEmpty() ? QVariant() : ui->comment_image2->toPlainText();
        break;
    case n_image3:
        m_comment = ui->comment_image3->toPlainText().isEmpty() ? QVariant() : ui->comment_image3->toPlainText();
        break;
    case n_image4:
        m_comment = ui->comment_image4->toPlainText().isEmpty() ? QVariant() : ui->comment_image4->toPlainText();
        break;
    case n_image5:
        m_comment = ui->comment_image5->toPlainText().isEmpty() ? QVariant() : ui->comment_image5->toPlainText();
        break;
    default:;
        break;
    }

    // cream solicitarea
    QSqlQuery qry(globals().thisSqlite ? db->getDatabaseImage() : db->getDatabase());
    qry.prepare(QString("UPDATE imagesReports SET  image_%1 = ?, comment_%1 = ? WHERE id_reportEcho = ?")
                .arg(numberImage));

    qry.addBindValue(imageData.toBase64());
    qry.addBindValue(m_comment);
    qry.addBindValue(m_id);
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
        qInfo(logInfo()) << tr("A fost salvata imaginea %1 cu succes in baza de date 'BD_IMAGE'.").arg(numberImage);
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Inserarea/actualizarea imaginei."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Imaginea nu a fost salvată în baza de date !!!"));
        msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }

}

void DocReportEcho::removeImageIntoTableimagesReports(const int numberImage)
{
    // verificam ID
    if (m_id == Enums::IDX::IDX_UNKNOW)
        return;

    // determinam baza de date
    QSqlDatabase dbImg = globals().thisMySQL ? db->getDatabase() : db->getDatabaseImage();

    // verificam daca exista inregistrarea
    if (! db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id), dbImg)) {

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(tr(""));
        message->setTextTitle(tr("Imaginea nu poate fi eliminată din baza de date !!!"));
        message->setDetailedText(tr("Nu au fost gasite date documentului cu ID '%1' in baza de date '%2'.")
                                 .arg(QString::number(m_id),
                                      dbImg.databaseName()));
        message->exec();
        message->deleteLater();
        return;
    }

    // Curățăm imaginea și comentariul asociat
    QSqlQuery qry(dbImg);
    qry.prepare(QString(R"(
        UPDATE imagesReports SET
            image_%1 = ?,
            comment_%1 = ?
        WHERE
            id_reportEcho = ?
    )").arg(numberImage));
    qry.addBindValue(QVariant());  // imagine ștearsă
    qry.addBindValue(QVariant());  // comentariu șters
    qry.addBindValue(m_id);

    if (! qry.exec()) {

        // pregatim textul erorii
        err.clear();
        err << this->metaObject()->className()
            << "[removeImageIntoTableimagesReports]:"
            << tr("Imaginea nu a fost eliminată din baza de date")
               .arg(qry.lastError().text().isEmpty()
                        ? ": eroarea indisponibila"
                        : ": " + qry.lastError().text());

        CustomMessage *message = new CustomMessage(this);
        message->setWindowTitle(tr(""));
        message->setTextTitle(tr("Imaginea nu poate fi eliminată din baza de date !!!"));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();

        qWarning(logWarning()) << err;
        return;
    }

    // Verificăm dacă toate imaginile sunt goale → ștergem rândul complet
    QSqlQuery checkQry(dbImg);
    checkQry.prepare(R"(
        SELECT
            image_1, image_2, image_3, image_4, image_5
        FROM
            imagesReports
        WHERE id_reportEcho = ?
    )");
    checkQry.addBindValue(m_id);

    if (checkQry.exec() && checkQry.next()) {
        bool allEmpty = true;
        for (int i = 0; i < 5; ++i) {
            QByteArray img = checkQry.value(i).toByteArray();
            if (!img.isEmpty()) {
                allEmpty = false;
                break;
            }
        }

        // eliminam complect inregistrarea
        if (allEmpty) {
            QSqlQuery delQry(dbImg);
            delQry.prepare(R"(DELETE FROM imagesReports WHERE id_reportEcho = ?)");
            delQry.addBindValue(m_id);
            if (delQry.exec()) {
                qInfo(logInfo()) << tr("Toate imaginile au fost eliminate. Rândul complet a fost șters.");
            } else {
                qWarning(logWarning()) << tr("Eroare la ștergerea rândului gol:") << delQry.lastError().text();
            }
        }
    }

    // prezentam mesaj
    popUp->setPopupText(tr("Imaginea a fost eliminată cu succes din baza de date."));
    popUp->show();

    // logarea
    qInfo(logInfo()) << tr("Imaginea %1 a fost eliminată cu succes din baza de date.").arg(numberImage);
}

void DocReportEcho::updateCommentIntoTableimagesReports()
{
    if (m_id == Enums::IDX::IDX_UNKNOW)
        return;

    QSqlDatabase dbImg = globals().thisSqlite ? db->getDatabaseImage() : db->getDatabase();

    if (! db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id), dbImg))
        return;

    QSqlQuery qry(dbImg);
    qry.prepare(R"(
        UPDATE imagesReports SET
            comment_1 = ?,
            comment_2 = ?,
            comment_3 = ?,
            comment_4 = ?,
            comment_5 = ?
        WHERE id_reportEcho = ?
    )");

    qry.addBindValue(ui->comment_image1->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->comment_image1->toPlainText());
    qry.addBindValue(ui->comment_image2->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->comment_image2->toPlainText());
    qry.addBindValue(ui->comment_image3->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->comment_image3->toPlainText());
    qry.addBindValue(ui->comment_image4->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->comment_image4->toPlainText());
    qry.addBindValue(ui->comment_image5->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->comment_image5->toPlainText());
    qry.addBindValue(m_id);

    if (! qry.exec()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Inserarea comentariului."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Comentariul nu a fost inserat/actualizat în baza de date !!!"));
        msgBox.setDetailedText(qry.lastError().text().isEmpty() ? "unknown" : qry.lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
}

// *******************************************************************
// **************** INITIEREA CONEXIUNILOR SI PROCESAREA LOR *********

void DocReportEcho::initConnections()
{
    QString style_toolButton = db->getStyleForToolButton();

    ui->btnParameters->setMouseTracking(true);
    ui->btnParameters->installEventFilter(this);
    ui->btnHistory->setMouseTracking(true);
    ui->btnHistory->installEventFilter(this);
    ui->btnOpenCatPatient->setMouseTracking(true);
    ui->btnOpenCatPatient->installEventFilter(this);
    ui->btnOpenDocErderEcho->setMouseTracking(true);
    ui->btnOpenDocErderEcho->installEventFilter(this);

    // setam stilul
    ui->btnHistory->setStyleSheet(style_toolButton);
    ui->btnOpenCatPatient->setStyleSheet(style_toolButton);
    ui->btnOpenDocErderEcho->setStyleSheet(style_toolButton);

    // calculate gestation age
    connect(ui->gestation0_LMP, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged);
    connect(ui->gestation1_LMP, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged);
    connect(ui->gestation2_dateMenstruation, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged);
    connect(ui->gestation2_fetus_age, &QLineEdit::textEdited, this, &DocReportEcho::setDueDateGestation2);

    connect(ui->btnParameters, &QPushButton::clicked, this, &DocReportEcho::openParameters);
    connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);
    connect(ui->btnOpenCatPatient, &QPushButton::clicked, this, &DocReportEcho::openCatPatient);
    connect(ui->btnHistory, &QPushButton::clicked, this, &DocReportEcho::openHistoryPatient);
    connect(ui->btnOpenDocErderEcho, &QPushButton::clicked, this, &DocReportEcho::openDocOrderEcho);

    updateStyleBtnInvestigations();

    connect(ui->btnOrgansInternal, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnOrgansInternal);
    connect(ui->btnUrinarySystem, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnUrinarySystem);
    connect(ui->btnProstate, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnProstate);
    connect(ui->btnGynecology, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnGynecology);
    connect(ui->btnBreast, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnBreast);
    connect(ui->btnThyroid, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnThyroid);
    connect(ui->btnGestation0, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnGestation0);
    connect(ui->btnGestation1, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnGestation1);
    connect(ui->btnGestation2, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnGestation2);
    connect(ui->btnComment, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnComment);
    connect(ui->btnImages, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnImages);
    connect(ui->btnVideo, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnVideo);
    connect(ui->btnNormograms, &QAbstractButton::clicked, this, &DocReportEcho::clickBtnNormograms);

    // adaugarea descrierelor formatiunilor
    ui->btn_add_template_organsInternal->setStyleSheet(style_toolButton);
    ui->btn_add_template_urinary_system->setStyleSheet(style_toolButton);
    ui->btn_add_template_prostate->setStyleSheet(style_toolButton);
    ui->btn_add_template_gynecology->setStyleSheet(style_toolButton);
    ui->btn_add_template_breast->setStyleSheet(style_toolButton);
    ui->btn_add_template_thyroid->setStyleSheet(style_toolButton);
    ui->btn_add_template_gestation0->setStyleSheet(style_toolButton);
    ui->btn_add_template_gestation1->setStyleSheet(style_toolButton);

    connect(ui->btn_add_template_organsInternal, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_urinary_system, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_prostate, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gynecology, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_breast, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_thyroid, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gestation0, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gestation1, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);

    // selectia descrierilor formatiunilor
    ui->btn_template_organsInternal->setStyleSheet(style_toolButton);
    ui->btn_template_urinary_system->setStyleSheet(style_toolButton);
    ui->btn_template_prostate->setStyleSheet(style_toolButton);
    ui->btn_template_gynecology->setStyleSheet(style_toolButton);
    ui->btn_template_breast->setStyleSheet(style_toolButton);
    ui->btn_template_thyroid->setStyleSheet(style_toolButton);
    ui->btn_template_gestation0->setStyleSheet(style_toolButton);
    ui->btn_template_gestation1->setStyleSheet(style_toolButton);

    connect(ui->btn_template_organsInternal, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_urinary_system, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_prostate, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_gynecology, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_breast, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_thyroid, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_gestation0, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);
    connect(ui->btn_template_gestation1, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnSelectConcluzionTemplates);


    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::slot_CountImagesChanged);
    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::CountVideoChanged, this, &DocReportEcho::slot_CountVideoChanged);

    connect(this, &DocReportEcho::ItNewChanged, this, &DocReportEcho::slot_ItNewChanged);
    connect(this, &DocReportEcho::IdChanged, this, &DocReportEcho::slot_IdChanged);
    connect(this, &DocReportEcho::IdPacientChanged, this, &DocReportEcho::slot_IdPacientChanged);
    connect(this, &DocReportEcho::IdDocOrderEchoChanged, this, &DocReportEcho::slot_IdDocOrderEchoChanged);

    connect(this, &DocReportEcho::t_organs_internalChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_urinary_systemChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_prostateChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_gynecologyChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_breastChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_thyroideChanged, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_gestation0Changed, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_gestation1Changed, this, &DocReportEcho::initEnableBtn);
    connect(this, &DocReportEcho::t_gestation2Changed, this, &DocReportEcho::initEnableBtn);

    // ---------------- select tampletes formations by system
    // organs internal
    ui->btnSelectTemplatesIntestinalFormations->setStyleSheet(style_toolButton);
    connect(ui->liver_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Ficat");});
    connect(ui->cholecist_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Colecist");});
    connect(ui->pancreas_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Pancreas");});
    connect(ui->spleen_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Splina");});
    connect(ui->btnSelectTemplatesIntestinalFormations, &QToolButton::clicked, this, [this](){openTempletsBySystem("Intestine");});
    connect(ui->organsInternal_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (org.interne)");});
    // s.urynari
    ui->btnSelectTempletsAdrenalGlands->setStyleSheet(style_toolButton);
    connect(ui->kidney_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Rinichi");});
    connect(ui->bladder_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("V.urinara");});
    connect(ui->btnSelectTempletsAdrenalGlands, &QToolButton::clicked, this, [this](){openTempletsBySystem("Gl.suprarenale");});
    connect(ui->urinary_system_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (s.urinar)");});
    // prostata
    connect(ui->prostate_formations, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Prostata");});
    connect(ui->prostate_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (prostata)");});
    // tiroida
    ui->btnSelectTempletsThyroid->setStyleSheet(style_toolButton);
    connect(ui->btnSelectTempletsThyroid, &QToolButton::clicked, this, [this](){openTempletsBySystem("Tiroida");});
    connect(ui->thyroid_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (tiroida)");});
    // breast
    ui->btnSelectTempletsBreastLeft->setStyleSheet(style_toolButton);
    ui->btnSelectTempletsBreastRight->setStyleSheet(style_toolButton);
    connect(ui->btnSelectTempletsBreastLeft, &QToolButton::clicked, this, [this](){openTempletsBySystem("Gl.mamara (stanga)");});
    connect(ui->btnSelectTempletsBreastRight, &QToolButton::clicked, this, [this](){openTempletsBySystem("Gl.mamara (dreapta)");});
    connect(ui->breast_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (gl.mamare)");});
    // ginecologia
    ui->btnSelectTempletsUter->setStyleSheet(style_toolButton);
    ui->btnSelectTempletsOvarLeft->setStyleSheet(style_toolButton);
    ui->btnSelectTempletsOvarRight->setStyleSheet(style_toolButton);
    connect(ui->btnSelectTempletsUter, &QToolButton::clicked, this, [this](){openTempletsBySystem("Ginecologia (uter)");});
    connect(ui->btnSelectTempletsOvarLeft, &QToolButton::clicked, this, [this](){openTempletsBySystem("Ginecologia (ovar stang)");});
    connect(ui->btnSelectTempletsOvarRight, &QToolButton::clicked, this, [this](){openTempletsBySystem("Ginecologia (ovar drept)");});
    connect(ui->gynecology_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (ginecologia)");});
    // gestation
    connect(ui->gestation0_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (gestatation0)");});
    connect(ui->gestation1_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (gestatation1)");});
    connect(ui->gestation2_recommendation, &LineEditCustom::onClickSelect, this, [this](){openTempletsBySystem("Recomandari (gestatation2)");});

    //---------------- add description formation by system
    // organs internal
    ui->btnAddTemplatesIntestinalFormations->setStyleSheet(style_toolButton);
    connect(ui->liver_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Ficat");});
    connect(ui->cholecist_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Colecist");});
    connect(ui->pancreas_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Pancreas");});
    connect(ui->spleen_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Splina");});
    connect(ui->btnAddTemplatesIntestinalFormations, &QToolButton::clicked, this, [this](){addDescriptionFormation("Intestine");});
    connect(ui->organsInternal_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (org.interne)");});
    // s.urynari
    ui->btnAddTempletsAdrenalGlands->setStyleSheet(style_toolButton);
    connect(ui->kidney_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Rinichi");});
    connect(ui->bladder_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("V.urinara");});
    connect(ui->btnAddTempletsAdrenalGlands, &QToolButton::clicked, this, [this](){addDescriptionFormation("Gl.suprarenale");});
    connect(ui->urinary_system_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (s.urinar)");});
    // prostata
    connect(ui->prostate_formations, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Prostata");});
    connect(ui->prostate_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (prostata)");});
    // tiroida
    ui->btnAddTempletsThyroid->setStyleSheet(style_toolButton);
    connect(ui->btnAddTempletsThyroid, &QToolButton::clicked, this, [this](){addDescriptionFormation("Tiroida");});
    connect(ui->thyroid_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (tiroida)");});
    // breast
    ui->btnAddTempletsBreastLeft->setStyleSheet(style_toolButton);
    ui->btnAddTempletsBreastRight->setStyleSheet(style_toolButton);
    connect(ui->btnAddTempletsBreastLeft, &QToolButton::clicked, this, [this](){addDescriptionFormation("Gl.mamara (stanga)");});
    connect(ui->btnAddTempletsBreastRight, &QToolButton::clicked, this, [this](){addDescriptionFormation("Gl.mamara (dreapta)");});
    connect(ui->breast_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (gl.mamare)");});
    // ginecologia
    ui->btnAddTempletsUter->setStyleSheet(style_toolButton);
    ui->btnAddTempletsOvarLeft->setStyleSheet(style_toolButton);
    ui->btnAddTempletsOvarRight->setStyleSheet(style_toolButton);
    connect(ui->btnAddTempletsUter, &QToolButton::clicked, this, [this](){addDescriptionFormation("Ginecologia (uter)");});
    connect(ui->btnAddTempletsOvarLeft, &QToolButton::clicked, this, [this](){addDescriptionFormation("Ginecologia (ovar stang)");});
    connect(ui->btnAddTempletsOvarRight, &QToolButton::clicked, this, [this](){addDescriptionFormation("Ginecologia (ovar drept)");});
    connect(ui->gynecology_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (ginecologia)");});
    // gestation
    connect(ui->gestation0_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (gestatation0)");});
    connect(ui->gestation1_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (gestatation1)");});
    connect(ui->gestation2_recommendation, &LineEditCustom::onClickAddItem, this, [this](){addDescriptionFormation("Recomandari (gestatation2)");});

    createMenuPrint();

    connect(ui->btn_clear_image1, &QToolButton::clicked, this, &DocReportEcho::clickBtnClearImage1);
    connect(ui->btn_clear_image2, &QToolButton::clicked, this, &DocReportEcho::clickBtnClearImage2);
    connect(ui->btn_clear_image3, &QToolButton::clicked, this, &DocReportEcho::clickBtnClearImage3);
    connect(ui->btn_clear_image4, &QToolButton::clicked, this, &DocReportEcho::clickBtnClearImage4);
    connect(ui->btn_clear_image5, &QToolButton::clicked, this, &DocReportEcho::clickBtnClearImage5);

    connect(ui->comment_image1, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image2, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image3, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image4, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->comment_image5, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);

    // descrierea doppler
    connect(ui->gestation2_fetusMass, &QLineEdit::editingFinished, this, &DocReportEcho::updateDescriptionFetusWeight);
    connect(ui->gestation2_ombilic_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler);
    connect(ui->gestation2_uterLeft_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler);
    connect(ui->gestation2_uterRight_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler);
    connect(ui->gestation2_cerebral_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler);

    connect(ui->btnOk, &QPushButton::clicked, this, &DocReportEcho::onWritingDataClose);
    connect(ui->btnWrite, &QPushButton::clicked, this, &DocReportEcho::onWritingData);
    connect(ui->btnClose, &QPushButton::clicked, this, &DocReportEcho::onClose);
}

void DocReportEcho::openParameters()
{
    CustomDialogInvestig* dialogInvestig = new CustomDialogInvestig(this);
    dialogInvestig->set_t_organs_internal(m_organs_internal);
    dialogInvestig->set_t_urinary_system(m_urinary_system);
    dialogInvestig->set_t_prostate(m_prostate);
    dialogInvestig->set_t_gynecology(m_gynecology);
    dialogInvestig->set_t_breast(m_breast);
    dialogInvestig->set_t_thyroide(m_thyroide);
    dialogInvestig->set_t_gestation0(m_gestation0);
    dialogInvestig->set_t_gestation1(m_gestation1);
    dialogInvestig->set_t_gestation2(m_gestation2);

    if (dialogInvestig->exec() != QDialog::Accepted)
        return;

    set_t_organs_internal(dialogInvestig->get_t_organs_internal());
    set_t_urinary_system(dialogInvestig->get_t_urinary_system());
    set_t_prostate(dialogInvestig->get_t_prostate());
    set_t_gynecology(dialogInvestig->get_t_gynecology());
    set_t_breast(dialogInvestig->get_t_breast());
    set_t_thyroide(dialogInvestig->get_t_thyroide());
    set_t_gestation0(dialogInvestig->get_t_gestation0());
    set_t_gestation1(dialogInvestig->get_t_gestation1());
    set_t_gestation2(dialogInvestig->get_t_gestation2());

    initEnableBtn();  // modificam vizualizarea btn
    updateStyleBtnInvestigations();
}

void DocReportEcho::openCatPatient()
{
    if (m_idPacient == Enums::IDX::IDX_UNKNOW)
        return;
    CatGeneral* cat_patients = new CatGeneral(this);
    cat_patients->setAttribute(Qt::WA_DeleteOnClose);
    cat_patients->setProperty("typeCatalog", CatGeneral::TypeCatalog::Pacients);
    cat_patients->setProperty("itNew", false);
    cat_patients->setProperty("Id", m_idPacient);
    cat_patients->show();
}

void DocReportEcho::openHistoryPatient()
{
    history_patient =  new PatientHistory(this);
    history_patient->setAttribute(Qt::WA_DeleteOnClose);
    history_patient->setProperty("IdPatient", m_idPacient);
    this->hide();
    history_patient->exec();
    this->show();
}

void DocReportEcho::openDocOrderEcho()
{
    if (m_id_docOrderEcho == Enums::IDX::IDX_UNKNOW)
        return;
    DocOrderEcho* docOrder = new DocOrderEcho(this);
    docOrder->setAttribute(Qt::WA_DeleteOnClose);
    docOrder->setProperty("itNew", false);
    docOrder->setProperty("Id", m_id_docOrderEcho);
    docOrder->setGeometry(250, 150, 1400, 800);
    docOrder->show();
}

void DocReportEcho::openTempletsBySystem(const QString name_system)
{
    if (name_system.isEmpty())
        return;
    openCatalogWithSystemTamplets(name_system);
}

void DocReportEcho::openCatalogWithSystemTamplets(const QString name_system)
{
    if (name_system.isEmpty())
        return;

    CatForSqlTableModel* template_concluzion = new CatForSqlTableModel(this);
    template_concluzion->setWindowIcon(QIcon(":/img/templates.png"));
    template_concluzion->setAttribute(Qt::WA_DeleteOnClose);
    template_concluzion->m_filter_templates = name_system;
    template_concluzion->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::SystemTemplates);
    template_concluzion->setProperty("typeForm",    CatForSqlTableModel::TypeForm::SelectForm);
    template_concluzion->show();

    connect(template_concluzion, &CatForSqlTableModel::mSelectData, this, [=, this]()
    {
        if (name_system == "Ficat")
            ui->liver_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Colecist")
            ui->cholecist_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Pancreas")
            ui->pancreas_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Splina")
            ui->spleen_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Intestine")
            ui->intestinalHandles->setText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (org.interne)")
            ui->organsInternal_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Rinichi")
            ui->kidney_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "V.urinara")
            ui->bladder_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Gl.suprarenale")
            ui->adrenalGlands->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (s.urinar)")
            ui->urinary_system_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Prostata")
            ui->prostate_formations->setText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (prostata)")
            ui->prostate_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Tiroida")
            ui->thyroid_formations->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (tiroida)")
            ui->thyroid_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Gl.mamara (stanga)")
            ui->breast_left_formations->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Gl.mamara (dreapta)")
            ui->breast_right_formations->setPlainText(template_concluzion->getSelectName());
         else if (name_system == "Recomandari (gl.mamare")
            ui->breast_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Ginecologia (uter)")
            ui->gynecology_uterus_formations->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Ginecologia (ovar stang)")
            ui->gynecology_ovary_formations_left->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Ginecologia (ovar drept)")
            ui->gynecology_ovary_formations_right->setPlainText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (ginecologia)")
            ui->gynecology_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (gestatation0)")
            ui->gestation0_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (gestatation1)")
            ui->gestation1_recommendation->setText(template_concluzion->getSelectName());
        else if (name_system == "Recomandari (gestatation2)")
            ui->gestation2_recommendation->setText(template_concluzion->getSelectName());

        template_concluzion->close();

    });
}

void DocReportEcho::addDescriptionFormation(const QString name_system)
{
    QString str;
    if (name_system == "Ficat")
        str = ui->liver_formations->text();
    else if (name_system == "Colecist")
        str = ui->cholecist_formations->text();
    else if (name_system == "Pancreas")
        str = ui->pancreas_formations->text();
    else if (name_system == "Splina")
        str = ui->spleen_formations->text();
    else if (name_system == "Intestine")
        str = ui->intestinalHandles->toPlainText();
    else if (name_system == "Recomandari (org.interne)")
        str = ui->organsInternal_recommendation->text();
    else if (name_system == "Rinichi")
        str = ui->kidney_formations->text();
    else if (name_system == "V.urinara")
        str = ui->bladder_formations->text();
    else if (name_system == "Gl.suprarenale")
        str = ui->adrenalGlands->toPlainText();
    else if (name_system == "Recomandari (s.urinar)")
        str = ui->urinary_system_recommendation->text();
    else if (name_system == "Prostata")
        str = ui->prostate_formations->text();
    else if (name_system == "Recomandari (prostata)")
        str = ui->prostate_recommendation->text();
    else if (name_system == "Tiroida")
        str = ui->thyroid_formations->toPlainText();
    else if (name_system == "Recomandari (tiroida)")
        str = ui->thyroid_recommendation->text();
    else if (name_system == "Gl.mamara (stanga)")
        str = ui->breast_left_formations->toPlainText();
    else if (name_system == "Gl.mamara (dreapta)")
        str = ui->breast_right_formations->toPlainText();
    else if (name_system == "Recomandari (gl.mamare)")
        str = ui->breast_recommendation->text();
    else if (name_system == "Ginecologia (uter)")
        str = ui->gynecology_uterus_formations->toPlainText();
    else if (name_system == "Ginecologia (ovar stang)")
        str = ui->gynecology_ovary_formations_left->toPlainText();
    else if (name_system == "Ginecologia (ovar drept)")
        str = ui->gynecology_ovary_formations_right->toPlainText();
    else if (name_system == "Recomandari (ginecologia)")
        str = ui->gynecology_recommendation->text();
    else if (name_system == "Recomandari (gestatation0)")
        str = ui->gestation0_recommendation->text();
    else if (name_system == "Recomandari (gestatation1)")
        str = ui->gestation1_recommendation->text();
    else if (name_system == "Recomandari (gestatation2)")
        str = ui->gestation2_recommendation->text();
    else
        qDebug() << this->metaObject()->className() << ": eroare - nu este determinat sistemul pentru insertia sablonului !!!";

    if (str.isEmpty())
        return;

    // verificam dublajul
    QSqlQuery qry;
    qry.prepare(QString("SELECT name FROM formationsSystemTemplates WHERE name = '%1' AND typeSystem = '%2';").arg(str, name_system));
    if (qry.exec() && qry.next()) {
        if (qry.value(0).toString() == str) {
            QMessageBox msgBox(QMessageBox::Question,
                               tr("Verificarea dublajului"),
                               tr("Descrierea <b><u>'%1'</u></b> exist\304\203 ca \310\231ablon.<br>"
                                  "Dori\310\233i s\304\203 prelungi\310\233i validarea ?").arg(str),
                               QMessageBox::Yes | QMessageBox::No, this);

            if (msgBox.exec() == QMessageBox::No){
                return;
            }
        }
    }

    qry.prepare(QString("INSERT INTO formationsSystemTemplates (id, deletionMark, name, typeSystem) VALUES (?, ?, ?, ?);"));
    qry.addBindValue(db->getLastIdForTable("formationsSystemTemplates") + 1);
    qry.addBindValue(0);
    qry.addBindValue(str);
    qry.addBindValue(name_system);
    if (qry.exec()) {
        popUp->setPopupText(tr("\310\230ablonul ad\304\203ugat cu succes<br>"
                               "\303\256n baza de date."));
        popUp->show();
    }

}

void DocReportEcho::clickBtnOrgansInternal()
{
    ui->stackedWidget->setCurrentIndex(page_organs_internal);
    str_concluzion_organs_internal.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnUrinarySystem()
{
    ui->stackedWidget->setCurrentIndex(page_urinary_system);
    str_concluzion_urinary_system.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnProstate()
{
    ui->stackedWidget->setCurrentIndex(page_prostate);
    str_concluzion_prostate.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnGynecology()
{
    ui->stackedWidget->setCurrentIndex(page_gynecology);
    str_concluzion_gynecology.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnBreast()
{
    ui->stackedWidget->setCurrentIndex(page_breast);
    str_concluzion_brest.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnThyroid()
{
    ui->stackedWidget->setCurrentIndex(page_thyroid);
    str_concluzion_thyroid.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnGestation0()
{
    ui->stackedWidget->setCurrentIndex(page_gestation0);
    str_concluzion_gestation0.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnGestation1()
{
    ui->stackedWidget->setCurrentIndex(page_gestation1);
    str_concluzion_gestation1.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnGestation2()
{
    ui->stackedWidget->setCurrentIndex(page_gestation2);
    str_concluzion_gestation2.clear();
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnComment()
{
    if (ui->comment->isVisible())
        ui->comment->setHidden(true);
    else
        ui->comment->setHidden(false);

    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnImages()
{
    ui->stackedWidget->setCurrentIndex(page_images);
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnVideo()
{
    ui->stackedWidget->setCurrentIndex(page_video);
    updateStyleBtnInvestigations();
}

void DocReportEcho::clickBtnNormograms()
{
    normograms = new Normograms(this);
    normograms->setAttribute(Qt::WA_DeleteOnClose);
    normograms->show();
}

void DocReportEcho::createMenuPrint()
{
    if (globals().showDesignerMenuPrint) {
        QMenu *setUpMenu = new QMenu(this);
        QAction *openDesignerReport = new QAction(setUpMenu);
        openDesignerReport->setIcon(QIcon(":/images/design.png"));
        openDesignerReport->setText(tr("Deschide designer"));

        QAction *openPreviewReport  = new QAction(setUpMenu);
        openPreviewReport->setIcon(QIcon(":/images/Print.png"));
        openPreviewReport->setText(tr("Deschide preview"));

        setUpMenu->addAction(openDesignerReport);
        setUpMenu->addAction(openPreviewReport);
        setUpMenu->setWindowFlags(setUpMenu->windowFlags() | Qt::FramelessWindowHint);
        setUpMenu->setAttribute(Qt::WA_TranslucentBackground);
        setUpMenu->setStyleSheet(" QMenu {border-radius: 5px; font-family: Arial; font-size: 14px;}"
                                 " QMenu::item {height: 25px; width: 150px; border: 1px solid none;}");
        ui->btnPrint->setMenu(setUpMenu);
        ui->btnPrint->setStyleSheet("width:110px");
        connect(openDesignerReport, &QAction::triggered, this, &DocReportEcho::clickOpenDesignerReport);
        connect(openPreviewReport, &QAction::triggered, this, &DocReportEcho::clickOpenPreviewReport);
    } else {
        connect(ui->btnPrint, &QPushButton::clicked, this, &DocReportEcho::clickOpenPreviewReport);
    }
}

void DocReportEcho::clickOpenDesignerReport()
{
    QString filePDF;
    onPrint(Enums::TYPE_PRINT::OPEN_DESIGNER, filePDF);
}

void DocReportEcho::clickOpenPreviewReport()
{
        QString filePDF;
    onPrint(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);
}

void DocReportEcho::clickBtnClearImage1()
{
    ui->image1->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image1->setTextFormat(Qt::RichText);
    ui->image1->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->comment_image1->clear();
    removeImageIntoTableimagesReports(n_image1);
    if (m_count_images > 0)
        setCountImages(m_count_images - 1);
}

void DocReportEcho::clickBtnClearImage2()
{
    ui->image2->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image2->setTextFormat(Qt::RichText);
    ui->image2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->comment_image2->clear();
    removeImageIntoTableimagesReports(n_image2);
    if (m_count_images > 0)
        setCountImages(m_count_images - 1);
}

void DocReportEcho::clickBtnClearImage3()
{
    ui->image3->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image3->setTextFormat(Qt::RichText);
    ui->image3->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->comment_image3->clear();
    removeImageIntoTableimagesReports(n_image3);
    if (m_count_images > 0)
        setCountImages(m_count_images - 1);
}

void DocReportEcho::clickBtnClearImage4()
{
    ui->image4->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image4->setTextFormat(Qt::RichText);
    ui->image4->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->comment_image4->clear();
    removeImageIntoTableimagesReports(n_image4);
    if (m_count_images > 0)
        setCountImages(m_count_images - 1);
}

void DocReportEcho::clickBtnClearImage5()
{
    ui->image5->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
    ui->image5->setTextFormat(Qt::RichText);
    ui->image5->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->comment_image5->clear();
    removeImageIntoTableimagesReports(n_image5);
    if (m_count_images > 0)
        setCountImages(m_count_images - 1);
}

void DocReportEcho::clickBtnAddConcluzionTemplates()
{
    QString str = nullptr;
    QString str_system = nullptr;

    if (ui->stackedWidget->currentIndex() == page_organs_internal){
        str = ui->organsInternal_concluzion->toPlainText();
        str_system = tr("Organe interne");
    } else if (ui->stackedWidget->currentIndex() == page_urinary_system){
        str = ui->urinary_system_concluzion->toPlainText();
        str_system = tr("Sistemul urinar");
    } else if (ui->stackedWidget->currentIndex() == page_prostate){
        str = ui->prostate_concluzion->toPlainText();
        str_system = tr("Prostata");
    } else if (ui->stackedWidget->currentIndex() == page_gynecology){
        str = ui->gynecology_concluzion->toPlainText();
        str_system = tr("Ginecologia");
    } else if (ui->stackedWidget->currentIndex() == page_breast){
        str = ui->breast_concluzion->toPlainText();
        str_system = tr("Gl.mamare");
    } else if (ui->stackedWidget->currentIndex() == page_thyroid){
        str = ui->thyroid_concluzion->toPlainText();
        str_system = tr("Tiroida");
    } else if (ui->stackedWidget->currentIndex() == page_gestation0){
        str = ui->gestation0_concluzion->toPlainText();
        str_system = tr("Sarcina până la 11 săptămâni");
    } else if (ui->stackedWidget->currentIndex() == page_gestation1){
        str = ui->gestation1_concluzion->toPlainText();
        str_system = tr("Sarcina 11-14 săptămâni");
    }

    if (str == nullptr)
        return;

    QSqlQuery qry;
    qry.prepare("SELECT name FROM conclusionTemplates WHERE name = :name;");
    qry.bindValue(":name", str);
    if (qry.exec()){
        if (qry.next() && qry.value(0).toString() == str) {

            QMessageBox msgBox(QMessageBox::Question,
                tr("Verificarea dublajului"),
                tr("Concluzia <b>%1</b> exist\304\203 ca \310\231ablon.<br>"
                   "Dori\310\233i s\304\203 prelungi\310\233i validarea ?").arg(str),
                    QMessageBox::Yes | QMessageBox::No, this);

                if (msgBox.exec() == QMessageBox::No){
                    return;
                }
        }
    } else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Verificarea dublajului"));
        msgBox.setText(tr("Solicitarea incorecta de determinare dublajului in tabela 'conclusionTemplates'."));
        msgBox.setDetailedText(tr((qry.lastError().text().isEmpty()) ? "eroarea indisponibila" : qry.lastError().text().toStdString().c_str()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }

    qry.prepare(QString("INSERT INTO conclusionTemplates ("
                        "id,"
                        "deletionMark, "
                        "cod, "
                        "name, "
                        "%1) VALUES (?, ?, ?, ?, ?);").arg((globals().thisMySQL) ? "`system`" : "system"));
    qry.addBindValue(db->getLastIdForTable("conclusionTemplates") + 1);
    qry.addBindValue(0);
    qry.addBindValue(db->getLastIdForTable("conclusionTemplates") + 1);
    qry.addBindValue(str);
    qry.addBindValue(str_system);
    if (qry.exec()){
        popUp->setPopupText(tr("\310\230ablonul ad\304\203ugat cu succes<br>"
                               "\303\256n baza de date."));
            popUp->show();
    } else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Adaugarea \310\231ablonului"));
        msgBox.setText(tr("\310\230ablonul - <b>%1</b> - nu a fost adaugat.").arg(str));
        msgBox.setDetailedText(tr((qry.lastError().text().isEmpty()) ? "eroarea indisponibila" : qry.lastError().text().toStdString().c_str()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
}

void DocReportEcho::clickBtnSelectConcluzionTemplates()
{
    CatForSqlTableModel* template_concluzion = new CatForSqlTableModel(this);
    template_concluzion->setWindowIcon(QIcon(":/img/templates.png"));
    template_concluzion->setAttribute(Qt::WA_DeleteOnClose);
    if (ui->stackedWidget->currentIndex() == page_organs_internal)
        template_concluzion->m_filter_templates = tr("Organe interne");
    else if (ui->stackedWidget->currentIndex() == page_urinary_system)
        template_concluzion->m_filter_templates = tr("Sistemul urinar");
    else if (ui->stackedWidget->currentIndex() == page_prostate)
        template_concluzion->m_filter_templates = tr("Prostata");
    else if (ui->stackedWidget->currentIndex() == page_gynecology)
        template_concluzion->m_filter_templates = tr("Ginecologia");
    else if (ui->stackedWidget->currentIndex() == page_breast)
        template_concluzion->m_filter_templates = tr("Gl.mamare");
    else if (ui->stackedWidget->currentIndex() == page_thyroid)
        template_concluzion->m_filter_templates = tr("Tiroida");
    else if (ui->stackedWidget->currentIndex() == page_gestation0)
        template_concluzion->m_filter_templates = tr("Sarcina până la 11 săptămâni");
    else if (ui->stackedWidget->currentIndex() == page_gestation1)
        template_concluzion->m_filter_templates = tr("Sarcina 11-14 săptămâni");
    template_concluzion->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::ConclusionTemplates);
    template_concluzion->setProperty("typeForm", CatForSqlTableModel::TypeForm::SelectForm);
    template_concluzion->show();
    connect(template_concluzion, &CatForSqlTableModel::mSelectData, this, [=, this]()
    {
        QString str = template_concluzion->getSelectName();

        if (ui->stackedWidget->currentIndex() == page_organs_internal)
            ui->organsInternal_concluzion->setPlainText((ui->organsInternal_concluzion->toPlainText().isEmpty()) ?
                                                            str : ui->organsInternal_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_urinary_system)
            ui->urinary_system_concluzion->setPlainText((ui->urinary_system_concluzion->toPlainText().isEmpty()) ?
                                                            str : ui->urinary_system_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_prostate)
            ui->prostate_concluzion->setPlainText((ui->prostate_concluzion->toPlainText().isEmpty()) ?
                                                      str : ui->prostate_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_gynecology)
            ui->gynecology_concluzion->setPlainText((ui->gynecology_concluzion->toPlainText().isEmpty()) ?
                                                        str : ui->gynecology_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_breast)
            ui->breast_concluzion->setPlainText((ui->breast_concluzion->toPlainText().isEmpty()) ?
                                                    str : ui->breast_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_thyroid)
            ui->thyroid_concluzion->setPlainText((ui->thyroid_concluzion->toPlainText().isEmpty()) ?
                                                     str : ui->thyroid_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_gestation0)
            ui->gestation0_concluzion->setPlainText((ui->gestation0_concluzion->toPlainText().isEmpty()) ?
                                                        str : ui->gestation0_concluzion->toPlainText() + " " + str);
        else if (ui->stackedWidget->currentIndex() == page_gestation1)
            ui->gestation1_concluzion->setPlainText((ui->gestation1_concluzion->toPlainText().isEmpty()) ?
                                                        str : ui->gestation1_concluzion->toPlainText() + " " + str);
    });
}

// *******************************************************************
// **************** CONEXIUNI SI PROCESAREA VIDEO ********************

void DocReportEcho::clickAddVideo()
{
    if (globals().show_content_info_video){
        info_win = new InfoWindow(this);
        info_win->setTypeInfo(InfoWindow::TypeInfo::INFO_VIDEO);
        info_win->setTitle(tr("Lucru cu fi\310\231ierele video."));
        info_win->setTex(globals().str_content_message_video);
        info_win->exec();
    }

    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Alege video"));
    fileDialog.setNameFilters({
                                  "Video Files (*.mp4 *.mkv *.avi *.mpeg *.webm *.mov)",
                                  "All Files (*)"
                              });
    QStringList supportedMimeTypes = QStringList({
                                                     "video/mp4",
                                                     "video/x-matroska",
                                                     "video/avi",
                                                     "video/mpeg",
                                                     "video/webm",
                                                     "video/quicktime",
                                                     "video/x-msvideo"
                                                 });
    if (! supportedMimeTypes.isEmpty())
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        setUrl(fileDialog.selectedUrls().constFirst());
}

void DocReportEcho::clickRemoveVideo()
{
    QDir dir;
    QMessageBox::StandardButton YesNo;
    YesNo = QMessageBox::question(this,
                                  tr("Eliminarea fi\310\231ierului."),
                                  tr("Dori\310\233i s\304\203 elimina\310\233i fi\310\231ierul:<br><u>%1</u> ?")
                                  .arg(dir.toNativeSeparators(globals().pathDirectoryVideo + "/" + list_play->currentItem()->text())),
                                  QMessageBox::Yes | QMessageBox::No);

    if (YesNo == QMessageBox::Yes){
        QFile file_video(dir.toNativeSeparators(globals().pathDirectoryVideo + "/" + list_play->currentItem()->text()));
        if (file_video.remove()) {
            popUp->setPopupText(tr("Fișierul eliminat cu succes."));
            popUp->show();
        }
    }

    QListWidgetItem *itm = list_play->takeItem(list_play->currentRow());
    delete itm;

    list_play->setCurrentRow(-1);

    setCountVideo(m_count_video - 1);

    qInfo(logInfo()) << tr("Eliminat fisierul video - %1").arg(QString::number(m_id) + "_" + QString::number(m_count_video + 1) + ".mp4");
}

void DocReportEcho::setUrl(const QUrl &url)
{
    QDir dir;
    QString str = url.toString();
#if defined(Q_OS_LINUX)
    str.remove(0,7); // eliminam -> file://
#elif defined(Q_OS_MACOS)
    str.remove(0,7); // eliminam -> file://
#elif defined(Q_OS_WIN)
    str.remove(0,8); // eliminam -> file:///
    QFile file(str);
    str = dir.toNativeSeparators(str);
    qDebug() << str;
#endif
    QFile::copy(str, dir.toNativeSeparators(globals().pathDirectoryVideo + "/" + QString::number(m_id) + "_" + QString::number(list_play->count()) + ".mp4"));

    list_play->addItem(QString::number(m_id) + "_" + QString::number(list_play->count()) + ".mp4");
    setCountVideo(m_count_video + 1);
    list_play->setCurrentRow(list_play->count());

    qInfo(logInfo()) << tr("Atasat fisierul video - %1").arg(QString::number(m_id) + "_" + QString::number(list_play->count()) + ".mp4");
}

void DocReportEcho::clickPlayVideo()
{
    switch (player->playbackState()) {
    case QMediaPlayer::PlayingState:
        player->pause();
        break;
    default:
        player->play();
        break;
    }
}

void DocReportEcho::setPositionSlider(int seconds)
{
    // player->setPosition(seconds * 1000);
    player->setPosition(seconds);
}

void DocReportEcho::mediaStateChanged(QMediaPlayer::PlaybackState state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void DocReportEcho::positionChanged(qint64 progress)
{
    // if (! m_positionSlider->isSliderDown())
    //     m_positionSlider->setValue(progress / 1000);
    m_positionSlider->setValue(progress);
    updateDurationInfo(progress / 1000);
}

void DocReportEcho::durationChanged(qint64 duration)
{
    m_duration = duration / 1000;
    // m_positionSlider->setMaximum(m_duration);

    // m_positionSlider->setRange(0, duration);
    m_positionSlider->setRange(0, duration);
}

void DocReportEcho::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || m_duration) {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
            currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60,
            m_duration % 60, (m_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (m_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    m_labelDuration->setText(tStr);
}

void DocReportEcho::handleError()
{
    m_playButton->setEnabled(false);
    const QString errorString = player->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(player->error()));
    else
        message += errorString;
    m_errorLabel->setText(message);
}

void DocReportEcho::changedCurrentRowPlayList(int row)
{
    if (row == -1)
        return;

    player->stop();

    if (! m_playButton->isEnabled())
        m_playButton->setEnabled(true);
    player->setSource(QUrl("file:///" + globals().pathDirectoryVideo + "/" + QString::number(m_id) + "_" + QString::number(row) + ".mp4"));
    txt_title_player->setText("Se rulează - " + list_play->currentItem()->text());
}

// *******************************************************************
// **************** CONEXIUNILOR TAG-URILOR **************************

void DocReportEcho::connections_tags()
{
    connect(this, &DocReportEcho::t_organs_internalChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_urinary_systemChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_prostateChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_gynecologyChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_breastChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_thyroideChanged, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_gestation0Changed, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_gestation1Changed, this, &DocReportEcho::dataWasModified);
    connect(this, &DocReportEcho::t_gestation2Changed, this, &DocReportEcho::dataWasModified);

}

void DocReportEcho::disconnections_tags()
{
    disconnect(this, &DocReportEcho::t_organs_internalChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_urinary_systemChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_prostateChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_gynecologyChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_breastChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_thyroideChanged, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_gestation0Changed, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_gestation1Changed, this, &DocReportEcho::dataWasModified);
    disconnect(this, &DocReportEcho::t_gestation2Changed, this, &DocReportEcho::dataWasModified);
}

void DocReportEcho::connections_organs_internal()
{
    // table liver
    ui->liver_left->setMaxLength(5);
    ui->liver_right->setMaxLength(5);
    ui->liver_contour->setMaxLength(20);
    ui->liver_parenchyma->setMaxLength(20);
    ui->liver_ecogenity->setMaxLength(30);
    ui->liver_formations->setMaxLength(300);
    ui->liver_duct_hepatic->setMaxLength(50);
    ui->liver_porta->setMaxLength(5);
    ui->liver_lienalis->setMaxLength(5);

    // table cholecist
    ui->cholecist_dimens->setMaxLength(15);
    ui->cholecist_form->setMaxLength(150);
    ui->cholecist_formations->setMaxLength(300);
    ui->cholecist_walls->setMaxLength(5);
    ui->cholecist_coledoc->setMaxLength(5);

    // table pancreas
    ui->pancreas_cefal->setMaxLength(5);
    ui->pancreas_corp->setMaxLength(5);
    ui->pancreas_tail->setMaxLength(5);
    ui->pancreas_parenchyma->setMaxLength(20);
    ui->pancreas_ecogenity->setMaxLength(30);
    ui->pancreas_formations->setMaxLength(300);

    // table spleen
    ui->spleen_dimens->setMaxLength(15);
    ui->spleen_contour->setMaxLength(20);
    ui->spleen_parenchyma->setMaxLength(30);
    ui->spleen_formations->setMaxLength(300);

    // table intestinal loop
    ui->intestinalHandles->setPlaceholderText(tr("...maximum 300 caractere"));
    connect(ui->intestinalHandles, &QTextEdit::textChanged, this, [=, this]()
    {
       if (ui->intestinalHandles->toPlainText().length() > 300)
           ui->intestinalHandles->textCursor().deletePreviousChar();
    });

    QList<QLineEdit*> list = ui->stackedWidget->widget(page_organs_internal)->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(page_organs_internal)->findChildren<QTextEdit*>();
    for (int n = 0; n < list_text_edit.count(); n++) {
        connect(list_text_edit[n], &QTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->organsInternal_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->organsInternal_concluzion->toPlainText().length() > 500)
                    ui->organsInternal_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->organsInternal_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->organsInternal_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_organs_internal()
{
    QList<QLineEdit*> list = ui->stackedWidget->widget(page_organs_internal)->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(page_organs_internal)->findChildren<QTextEdit*>();
    for (int n = 0; n < list_text_edit.count(); n++) {
        disconnect(list_text_edit[n], &QTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    disconnect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
}

void DocReportEcho::connections_urinary_system()
{
    // table kidney
    ui->kidney_right->setMaxLength(15);
    ui->kidney_left->setMaxLength(15);
    ui->kidney_corticomed_left->setMaxLength(5);
    ui->kidney_corticomed_right->setMaxLength(5);
    ui->kidney_pielocaliceal_left->setMaxLength(30);
    ui->kidney_pielocaliceal_right->setMaxLength(30);
    ui->kidney_formations->setMaxLength(500);

    ui->adrenalGlands->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->adrenalGlands, &QTextEdit::textChanged, this, [=, this]()
    {
        if (ui->adrenalGlands->toPlainText().length() > 500)
            ui->adrenalGlands->textCursor().deletePreviousChar();
    });
    QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(page_urinary_system)->findChildren<QTextEdit*>();
    for (int n = 0; n < list_text_edit.count(); n++) {
        connect(list_text_edit[n], &QTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    // table bladder
    ui->bladder_volum->setMaxLength(5);
    ui->bladder_walls->setMaxLength(5);
    ui->bladder_formations->setMaxLength(300);

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_urinary_system)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_urinary_system)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        connect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->urinary_system_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->urinary_system_concluzion->toPlainText().length() > 500)
                    ui->urinary_system_concluzion->textCursor().deletePreviousChar();
            });

    connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->urinary_system_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->urinary_system_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_urinary_system()
{
    QList<QTextEdit*> list_text_edit = ui->stackedWidget->widget(page_urinary_system)->findChildren<QTextEdit*>();
    for (int n = 0; n < list_text_edit.count(); n++) {
        disconnect(list_text_edit[n], &QTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_urinary_system)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_urinary_system)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        disconnect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_prostate()
{
    ui->prostate_dimens->setMaxLength(25);
    ui->prostate_volum->setMaxLength(5);
    ui->prostate_ecostructure->setMaxLength(30);
    ui->prostate_contur->setMaxLength(20);
    ui->prostate_ecogency->setMaxLength(30);
    ui->prostate_formations->setMaxLength(300);

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_prostate)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_prostate)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        connect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    ui->prostate_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->prostate_concluzion->toPlainText().length() > 500)
                    ui->prostate_concluzion->textCursor().deleteChar();
            });
    connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->prostate_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->prostate_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_prostate()
{
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_prostate)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_prostate)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        disconnect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_gynecology()
{
    ui->gynecology_antecedent->setMaxLength(150);
    ui->gynecology_uterus_dimens->setMaxLength(25);
    ui->gynecology_uterus_pozition->setMaxLength(30);
    ui->gynecology_uterus_ecostructure->setMaxLength(30);
    ui->gynecology_uterus_formations->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->gynecology_uterus_formations, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gynecology_uterus_formations->toPlainText().length() > 500)
                    ui->gynecology_uterus_formations->textCursor().deletePreviousChar();
            });
    ui->gynecology_jonctional_zone_description->setMaxLength(256);
    ui->gynecology_jonctional_zone_description->setPlaceholderText(tr("... maximum 256 caractere"));
    ui->gynecology_canal_cervical_formations->setMaxLength(256);
    ui->gynecology_canal_cervical_formations->setPlaceholderText(tr("... maximum 256 caractere"));
    ui->gynecology_ecou_dimens->setMaxLength(5);
    ui->gynecology_ecou_ecostructure->setMaxLength(100);
    ui->gynecology_douglas->setMaxLength(100);
    ui->gynecology_plex_venos->setMaxLength(150);
    ui->gynecology_ovary_left_dimens->setMaxLength(25);
    ui->gynecology_ovary_right_dimens->setMaxLength(25);
    ui->gynecology_ovary_left_volum->setMaxLength(5);
    ui->gynecology_ovary_right_volum->setMaxLength(5);
    ui->gynecology_follicule_left->setMaxLength(100);
    ui->gynecology_follicule_right->setMaxLength(100);
    ui->gynecology_fallopian_tubes_formations->setMaxLength(256);
    ui->gynecology_fallopian_tubes_formations->setPlaceholderText(tr("... maximum 256 caractere"));
    ui->gynecology_ovary_formations_left->setPlaceholderText(tr("... maximum 300 caractere"));
    connect(ui->gynecology_ovary_formations_left, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gynecology_ovary_formations_left->toPlainText().length() > 300)
                    ui->gynecology_ovary_formations_left->textCursor().deletePreviousChar();
            });
    ui->gynecology_ovary_formations_right->setPlaceholderText(tr("... maximum 300 caractere"));
    connect(ui->gynecology_ovary_formations_right, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gynecology_ovary_formations_right->toPlainText().length() > 300)
                    ui->gynecology_ovary_formations_right->textCursor().deletePreviousChar();
            });

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gynecology)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_gynecology)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        connect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    connect(ui->gynecology_btn_transabdom, &QRadioButton::clicked, this, &DocReportEcho::dataWasModified);
    connect(ui->gynecology_btn_transvaginal, &QRadioButton::clicked, this, &DocReportEcho::dataWasModified);
    connect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged, this, &DocReportEcho::updateTextDateMenstruation);
    ui->gynecology_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gynecology_concluzion->toPlainText().length() > 500)
                    ui->gynecology_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->gynecology_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gynecology_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_gynecology()
{
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gynecology)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_gynecology)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        disconnect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    disconnect(ui->gynecology_btn_transabdom, &QRadioButton::clicked, this, &DocReportEcho::dataWasModified);
    disconnect(ui->gynecology_btn_transvaginal, &QRadioButton::clicked, this, &DocReportEcho::dataWasModified);
    disconnect(ui->gynecology_dateMenstruation, &QDateEdit::dateChanged, this, &DocReportEcho::dataWasModified);
}

void DocReportEcho::connections_breast()
{
    // left
    ui->breast_left_ecostructure->setMaxLength(255);
    ui->breast_left_duct->setMaxLength(20);
    ui->breast_left_ligament->setMaxLength(20);
    ui->breast_left_formations->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->breast_left_formations, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->breast_left_formations->toPlainText().length() > 500)
                    ui->breast_left_formations->textCursor().deletePreviousChar();
            });
    ui->breast_left_ganglions->setMaxLength(300);

    // right
    ui->breast_right_ecostructure->setMaxLength(255);
    ui->breast_right_duct->setMaxLength(20);
    ui->breast_right_ligament->setMaxLength(20);
    ui->breast_right_formations->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->breast_right_formations, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->breast_right_formations->toPlainText().length() > 500)
                    ui->breast_right_formations->textCursor().deletePreviousChar();
            });
    ui->breast_right_ganglions->setMaxLength(300);

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_breast)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_breast)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        connect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    ui->breast_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->breast_concluzion, &QPlainTextEdit::textChanged, this, [this]()
            {
                if (ui->breast_concluzion->toPlainText().length() > 500)
                    ui->breast_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->breast_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->breast_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->breast_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_breast()
{
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_breast)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text = ui->stackedWidget->widget(page_breast)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text.count(); n++) {
        disconnect(list_plain_text[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_thyroid()
{
    ui->thyroid_left_dimens->setMaxLength(20);
    ui->thyroid_left_volum->setMaxLength(5);
    ui->thyroid_right_dimens->setMaxLength(20);
    ui->thyroid_right_volum->setMaxLength(5);
    ui->thyroid_istm->setMaxLength(4);
    ui->thyroid_ecostructure->setMaxLength(20);
    ui->thyroid_formations->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->thyroid_formations, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->thyroid_formations->toPlainText().length() > 500)
                    ui->thyroid_formations->textCursor().deletePreviousChar();
            });
    ui->thyroid_ganglions->setPlaceholderText(tr("... maximum 300 caractere"));
    ui->thyroid_ganglions->setMaxLength(300);

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_thyroid)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_thyroid)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->thyroid_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->thyroid_concluzion->toPlainText().length() > 500)
                    ui->thyroid_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);
    ui->thyroid_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->thyroid_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_thyroid()
{
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_thyroid)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_thyroid)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_gestation0()
{
    ui->gestation0_gestation->setInputMask("99s. 9z.");
    ui->gestation0_GS_age->setInputMask("99s. 9z.");
    ui->gestation0_CRL_age->setInputMask("99s. 9z.");

    ui->gestation0_antecedent->setMaxLength(150);
    ui->gestation0_gestation->setMaxLength(20);
    ui->gestation0_GS_dimens->setMaxLength(5);
    ui->gestation0_GS_age->setMaxLength(20);
    ui->gestation0_CRL_dimens->setMaxLength(5);
    ui->gestation0_CRL_age->setMaxLength(20);
    ui->gestation0_BCF->setMaxLength(30);
    ui->gestation0_liquid_amniotic->setMaxLength(40);
    ui->gestation0_miometer->setMaxLength(200);
    ui->gestation0_cervix->setMaxLength(200);
    ui->gestation0_ovary->setMaxLength(200);

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation0)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation0)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->gestation0_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gestation0_concluzion->toPlainText().length() > 500)
                    ui->gestation0_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);

    ui->gestation0_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gestation0_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_gestation0()
{
        QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation0)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation0)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_gestation1()
{
    ui->gestation1_gestation->setInputMask("99s. 9z.");
    ui->gestation1_CRL_age->setInputMask("99s. 9z.");
    ui->gestation1_BPD_age->setInputMask("99s. 9z.");
    ui->gestation1_FL_age->setInputMask("99s. 9z.");

    ui->gestation1_antecedent->setMaxLength(150);
    ui->gestation1_gestation->setMaxLength(20);
    ui->gestation1_CRL_dimens->setMaxLength(5);
    ui->gestation1_CRL_age->setMaxLength(20);
    ui->gestation1_BPD_dimens->setMaxLength(5);
    ui->gestation1_BPD_age->setMaxLength(20);
    ui->gestation1_NT_dimens->setMaxLength(5);
    ui->gestation1_NT_percent->setMaxLength(20);
    ui->gestation1_BN_dimens->setMaxLength(5);
    ui->gestation1_BN_percent->setMaxLength(20);
    ui->gestation1_BCF->setMaxLength(30);
    ui->gestation1_FL_dimens->setMaxLength(5);
    ui->gestation1_FL_age->setMaxLength(20);
    ui->gestation1_callote_cranium->setMaxLength(50);
    ui->gestation1_plex_choroid->setMaxLength(50);
    ui->gestation1_vertebral_column->setMaxLength(50);
    ui->gestation1_stomach->setMaxLength(50);
    ui->gestation1_diaphragm->setMaxLength(50);
    ui->gestation1_bladder->setMaxLength(50);
    ui->gestation1_abdominal_wall->setMaxLength(50);
    ui->gestation1_location_placenta->setMaxLength(50);
    ui->gestation1_sac_vitelin->setMaxLength(50);
    ui->gestation1_amniotic_liquid->setMaxLength(40);
    ui->gestation1_miometer->setMaxLength(200);
    ui->gestation1_cervix->setMaxLength(200);
    ui->gestation1_ovary->setMaxLength(200);
    ui->gestation1_sac_vitelin->setMaxLength(50);
    ui->gestation1_sac_vitelin->setPlaceholderText(tr("... maximum 50 caractere"));

    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation1)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation1)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->gestation1_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged, this, [=, this]()
            {
                if (ui->gestation1_concluzion->toPlainText().length() > 500)
                    ui->gestation1_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);

    ui->gestation1_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gestation1_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_gestation1()
{
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation1)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation1)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::connections_gestation2()
{
    ui->gestation2_gestation_age->setMaxLength(20);
    ui->gestation2_gestation_age->setPlaceholderText(tr("... maximum 20 caractere"));
    ui->gestation2_pregnancy_description->setMaxLength(250);
    ui->gestation2_pregnancy_description->setPlaceholderText(tr("... maximum 250 caractere"));
    ui->gestation2_eyeball_desciption->setMaxLength(100);
    ui->gestation2_eyeball_desciption->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_nasolabialTriangle_description->setMaxLength(100);
    ui->gestation2_nasolabialTriangle_description->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_ventricularSystem_description->setMaxLength(70);
    ui->gestation2_ventricularSystem_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_choroidalPlex_description->setMaxLength(70);
    ui->gestation2_choroidalPlex_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_cerebellum_description->setMaxLength(70);
    ui->gestation2_cerebellum_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_vertebralColumn_description->setMaxLength(100);
    ui->gestation2_vertebralColumn_description->setPlaceholderText(tr("... maximum 100 caractere"));
    ui->gestation2_heartPosition->setMaxLength(50);
    ui->gestation2_heartPosition->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_planPatruCamere_description->setMaxLength(70);
    ui->gestation2_planPatruCamere_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ventricularEjectionPathLeft_description->setMaxLength(70);
    ui->gestation2_ventricularEjectionPathLeft_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ventricularEjectionPathRight_description->setMaxLength(70);
    ui->gestation2_ventricularEjectionPathRight_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_intersectionVesselMagistral_description->setMaxLength(70);
    ui->gestation2_intersectionVesselMagistral_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_planTreiVase_description->setMaxLength(70);
    ui->gestation2_planTreiVase_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_pulmonaryAreas_description->setMaxLength(70);
    ui->gestation2_pulmonaryAreas_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_diaphragm_description->setMaxLength(70);
    ui->gestation2_diaphragm_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_stomach_description->setMaxLength(50);
    ui->gestation2_stomach_description->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_cholecist_description->setMaxLength(50);
    ui->gestation2_cholecist_description->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_intestine_description->setMaxLength(70);
    ui->gestation2_intestine_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_kidneys_description->setMaxLength(70);
    ui->gestation2_kidneys_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_ureter_description->setMaxLength(70);
    ui->gestation2_ureter_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_extremities_description->setMaxLength(150);
    ui->gestation2_extremities_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_placenta_localization->setMaxLength(50);
    ui->gestation2_placenta_localization->setPlaceholderText(tr("... maximum 50 caractere"));
    ui->gestation2_placentaStructure_description->setMaxLength(150);
    ui->gestation2_placentaStructure_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_umbilicalCordon_description->setMaxLength(70);
    ui->gestation2_umbilicalCordon_description->setPlaceholderText(tr("... maximum 70 caractere"));
    ui->gestation2_cervix_description->setMaxLength(150);
    ui->gestation2_cervix_description->setPlaceholderText(tr("... maximum 150 caractere"));
    ui->gestation2_comment->setPlaceholderText(tr("... maximum 250 caractere"));
;
    ui->gestation2_gestation_age->setInputMask("99s. 9z.");
    ui->gestation2_bpd_age->setInputMask("99s. 9z.");
    ui->gestation2_hc_age->setInputMask("99s. 9z.");
    ui->gestation2_ac_age->setInputMask("99s. 9z.");
    ui->gestation2_fl_age->setInputMask("99s. 9z.");
    ui->gestation2_fetus_age->setInputMask("99s. 9z.");

    // line edit
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation2)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        connect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    //plain text edit
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation2)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        connect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    //combobox
    QList<QComboBox*> list_combo = ui->stackedWidget->widget(page_gestation2)->findChildren<QComboBox*>();
    for (int n = 0; n < list_combo.count(); n++) {
        connect(list_combo[n], &QComboBox::currentTextChanged, this, &DocReportEcho::dataWasModified);
    }

    ui->gestation2_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    ui->gestation2_recommendation->setPlaceholderText(tr("... maximum 255 caractere"));
    ui->gestation2_recommendation->setMaxLength(255);

    connect(ui->gestation2_comment, &QPlainTextEdit::textChanged, this, [=, this](){
        if (ui->gestation2_comment->toPlainText().length() > 250)
            ui->gestation2_comment->textCursor().deletePreviousChar();
    });

    connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged, this, [=, this](){
        if (ui->gestation2_concluzion->toPlainText().length() > 500)
            ui->gestation2_concluzion->textCursor().deletePreviousChar();
    });

    connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::updateTextConcluzionBySystem);

    connect(group_btn_gestation2, QOverload<int>::of(&QButtonGroup::idClicked),
            this, QOverload<int>::of(&DocReportEcho::clickedGestation2Trimestru));

    connect(ui->gestation2_nasalFold, &QLineEdit::editingFinished, this, [=, this]() {
        ui->tabWidget_morfology->setCurrentIndex(1); // SNC
    });

    connect(ui->gestation2_vertebralColumn_description, &QLineEdit::editingFinished, this, [=, this](){
        ui->tabWidget_morfology->setCurrentIndex(2); // heart
    });

    connect(ui->gestation2_diaphragm_description, &QLineEdit::editingFinished, this, [=, this](){
        ui->tabWidget_morfology->setCurrentIndex(4); // abdomen
    });

    connect(ui->gestation2_intestine_description, &QLineEdit::editingFinished, this, [=, this](){
        ui->tabWidget_morfology->setCurrentIndex(5); // s.urinar
    });
}

void DocReportEcho::disconnections_gestation2()
{
    // line edit
    QList<QLineEdit*> list_line_edit = ui->stackedWidget->widget(page_gestation2)->findChildren<QLineEdit*>();
    for (int n = 0; n < list_line_edit.count(); n++) {
        disconnect(list_line_edit[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    //plain text edit
    QList<QPlainTextEdit*> list_plain_text_edit = ui->stackedWidget->widget(page_gestation2)->findChildren<QPlainTextEdit*>();
    for (int n = 0; n < list_plain_text_edit.count(); n++) {
        disconnect(list_plain_text_edit[n], &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    //combobox
    QList<QComboBox*> list_combo = ui->stackedWidget->widget(page_gestation2)->findChildren<QComboBox*>();
    for (int n = 0; n < list_combo.count(); n++) {
        disconnect(list_combo[n], &QComboBox::currentTextChanged, this, &DocReportEcho::dataWasModified);
    }
}

void DocReportEcho::clickedGestation2Trimestru(const int id_button)
{
    if (id_button == 0){ // trimestru II
        ui->gestation2_nasalBones->setEnabled(true);
        ui->gestation2_nasalBones_dimens->setEnabled(true);
        ui->gestation2_nasalFold->setEnabled(true);
        ui->label_gestation2_nasalBones->setEnabled(true);
        ui->label_gestation2_nasalBones_dimens->setEnabled(true);
        ui->label_gestation2_nasalBones_mm->setEnabled(true);
        ui->label_gestation2_nasalFold->setEnabled(true);
        ui->label_gestation2_nasalFold_mm->setEnabled(true);

        ui->label_gestation2_fissureSilvius->setEnabled(true);
        ui->gestation2_fissureSilvius->setEnabled(true);
        ui->label_gestation2_ventricularEjectionPathLeft->setEnabled(true);
        ui->gestation2_ventricularEjectionPathLeft->setEnabled(true);
        ui->gestation2_ventricularEjectionPathLeft_description->setEnabled(true);
        ui->label_gestation2_ventricularEjectionPathRight->setEnabled(true);
        ui->gestation2_ventricularEjectionPathRight->setEnabled(true);
        ui->gestation2_ventricularEjectionPathRight_description->setEnabled(true);
        ui->label_gestation2_planBicav->setEnabled(true);
        ui->gestation2_planBicav->setEnabled(true);
    } else { // trimestru III
        ui->gestation2_nasalBones->setEnabled(false);
        ui->gestation2_nasalBones_dimens->setEnabled(false);
        ui->gestation2_nasalFold->setEnabled(false);
        ui->label_gestation2_nasalBones->setEnabled(false);
        ui->label_gestation2_nasalBones_dimens->setEnabled(false);
        ui->label_gestation2_nasalBones_mm->setEnabled(false);
        ui->label_gestation2_nasalFold->setEnabled(false);
        ui->label_gestation2_nasalFold_mm->setEnabled(false);

        ui->label_gestation2_fissureSilvius->setEnabled(false);
        ui->gestation2_fissureSilvius->setEnabled(false);
        ui->label_gestation2_ventricularEjectionPathLeft->setEnabled(false);
        ui->gestation2_ventricularEjectionPathLeft->setEnabled(false);
        ui->gestation2_ventricularEjectionPathLeft_description->setEnabled(false);
        ui->label_gestation2_ventricularEjectionPathRight->setEnabled(false);
        ui->gestation2_ventricularEjectionPathRight->setEnabled(false);
        ui->gestation2_ventricularEjectionPathRight_description->setEnabled(false);
        ui->label_gestation2_planBicav->setEnabled(false);
        ui->gestation2_planBicav->setEnabled(false);
    }
}

// *******************************************************************
// **************** PROCESAREA DOPPLER SI MASA FATULUI ***************

void DocReportEcho::updateTextDescriptionDoppler()
{
    QString txt_umbelicalArtery    = getPercentageByDopplerUmbelicalArtery();
    QString txt_uterineArteryLeft  = getPercentageByDopplerUterineArteryLeft();
    QString txt_uterineArteryRight = getPercentageByDopplerUterineArteryRight();
    QString txt_CMA                = getPercentageByDopplerCMA();

    ui->gestation2_infoDoppler->clear();

    if (txt_umbelicalArtery != nullptr)
        ui->gestation2_infoDoppler->append(txt_umbelicalArtery);

    if (txt_uterineArteryLeft != nullptr)
        ui->gestation2_infoDoppler->append(txt_uterineArteryLeft);

    if (txt_uterineArteryRight != nullptr)
        ui->gestation2_infoDoppler->append(txt_uterineArteryRight);

    if (txt_CMA != nullptr)
        ui->gestation2_infoDoppler->append(txt_CMA);

    ui->gestation2_infoDoppler->append("\nDevierea de calcul cu 'Fetal Medicine Foundation' este de până la 0.10 percentile.");
}

void DocReportEcho::updateDescriptionFetusWeight()
{
    if (ui->gestation2_fetusMass->text().isEmpty()) {
        ui->gestation2_descriptionFetusWeight->setText("");
        return;
    }

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_weight = ui->gestation2_fetusMass->text().toDouble();

    data_percentage->setTypePercentile(DataPercentage::TYPE_PERCENTILES::P_FETAL_WEIGHT);
    ui->gestation2_descriptionFetusWeight->setText("Masa fătului: " +
                                                   data_percentage->determinePIPercentile_FMF(current_weight, weeks));
}

QString DocReportEcho::getPercentageByDopplerUmbelicalArtery()
{
    if (ui->gestation2_ombilic_PI->text().isEmpty())
        return nullptr;

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_ombilic_PI->text().replace(",", ".").toDouble();

    data_percentage->setTypePercentile(DataPercentage::TYPE_PERCENTILES::P_UMBILICAL_ARTERY);
    data_percentage->resetBloodFlow();
    QString txt = data_percentage->determinePIPercentile_FMF(current_PI, weeks);
    ui->gestation2_ombilic_flux->setCurrentIndex(data_percentage->getBloodFlow());

    return "Doppler a.ombelicale: " + txt;
}

QString DocReportEcho::getPercentageByDopplerUterineArteryLeft()
{
    if (ui->gestation2_uterLeft_PI->text().isEmpty())
        return nullptr;

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_uterLeft_PI->text().replace(",", ".").toDouble();

    data_percentage->setTypePercentile(DataPercentage::TYPE_PERCENTILES::P_UTERINE_ARTERY);
    data_percentage->resetBloodFlow();
    QString txt = data_percentage->determinePIPercentile_FMF(current_PI, weeks);
    ui->gestation2_uterLeft_flux->setCurrentIndex(data_percentage->getBloodFlow());

    return "Doppler a.uterină stânga: " + txt;
}

QString DocReportEcho::getPercentageByDopplerUterineArteryRight()
{
    if (ui->gestation2_uterRight_PI->text().isEmpty())
        return nullptr;

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_uterRight_PI->text().replace(",", ".").toDouble();

    data_percentage->setTypePercentile(DataPercentage::TYPE_PERCENTILES::P_UTERINE_ARTERY);
    data_percentage->resetBloodFlow();
    QString txt = data_percentage->determinePIPercentile_FMF(current_PI, weeks);
    ui->gestation2_uterRight_flux->setCurrentIndex(data_percentage->getBloodFlow());

    return "Doppler a.uterină dreapta: " + txt;
}

QString DocReportEcho::getPercentageByDopplerCMA()
{
    if (ui->gestation2_cerebral_PI->text().isEmpty())
        return nullptr;

    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    double current_PI = ui->gestation2_cerebral_PI->text().replace(",", ".").toDouble();

    data_percentage->setTypePercentile(DataPercentage::TYPE_PERCENTILES::P_CMA);
    data_percentage->resetBloodFlow();
    QString txt = data_percentage->determinePIPercentile_FMF(current_PI, weeks);
    ui->gestation2_cerebral_flux->setCurrentIndex(data_percentage->getBloodFlow());

    return "Doppler a.cerebrală medie: " + txt;
}

// *******************************************************************
// **************** PROCESAREA CONCLUZIILOR **************************

void DocReportEcho::updateTextConcluzionBySystem()
{
    ui->concluzion->clear();

    if (m_organs_internal && ! ui->organsInternal_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->organsInternal_concluzion->toPlainText());
    if (m_urinary_system && ! ui->urinary_system_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->urinary_system_concluzion->toPlainText());
    if (m_prostate && ! ui->prostate_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->prostate_concluzion->toPlainText());
    if (m_gynecology && ! ui->gynecology_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->gynecology_concluzion->toPlainText());
    if (m_breast && ! ui->breast_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->breast_concluzion->toPlainText());
    if (m_thyroide && ! ui->thyroid_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->thyroid_concluzion->toPlainText());
    if (m_gestation0 && ! ui->gestation0_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->gestation0_concluzion->toPlainText());
    if (m_gestation1 && ! ui->gestation1_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->gestation1_concluzion->toPlainText());
    if (m_gestation2 && ! ui->gestation2_concluzion->toPlainText().isEmpty())
        ui->concluzion->appendPlainText(ui->gestation2_concluzion->toPlainText());
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void DocReportEcho::slot_ItNewChanged()
{
    if (m_itNew){

        setWindowTitle(tr("Raport ecografic (crearea) %1").arg("[*]"));

        connect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
        timer->start(1000);                    // inițierea timerului pu data docum.

        setIdUser(globals().idUserApp);

        if (m_organs_internal)
            connections_organs_internal();
        if (m_urinary_system)
            connections_urinary_system();
        if (m_prostate)
            connections_prostate();
        if (m_gynecology)
            connections_gynecology();
        if (m_breast)
            connections_breast();
        if (m_thyroide)
            connections_thyroid();

        if (m_gestation0) {
            ui->gestation0_LMP->setDate(QDate::currentDate());
            connections_gestation0();
        }

        if (m_gestation1) {
            ui->gestation1_LMP->setDate(QDate::currentDate());
            connections_gestation1();
        }

        if (m_gestation2) {
            ui->gestation2_dateMenstruation->setDate(QDate::currentDate());
            updateDescriptionFetusWeight();
            connections_gestation2();
        }

        ui->comment->setHidden(true);
    }
    initEnableBtn();
    updateStyleBtnInvestigations();
}

void DocReportEcho::slot_IdChanged()
{
    if (m_id == Enums::IDX::IDX_UNKNOW)
        return;

    //----------------------------------------------------------------------------
    // deconectam conexiunile
    disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::dataWasModified);
    disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);
    disconnections_tags();

    //----------------------------------------------------------------------------
    // extragem datele principale ale documentului
    QMap<QString, QString> items;

    if (db->getObjectDataById("reportEcho", m_id, items)){

        ui->editDocNumber->setText(items.constFind("numberDoc").value());
        ui->editDocNumber->setDisabled(!ui->editDocNumber->text().isEmpty());
        if (globals().thisMySQL){
            QString str_date = items.constFind("dateDoc").value();
            static const QRegularExpression replaceT("T");
            static const QRegularExpression removeMilliseconds("\\.000");
            str_date = str_date.replace(replaceT, " ").replace(removeMilliseconds,"");
            ui->editDocDate->setDateTime(QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss"));
        } else
            ui->editDocDate->setDateTime(QDateTime::fromString(items.constFind("dateDoc").value(), "yyyy-MM-dd hh:mm:ss"));

        int id_pacient = items.constFind("id_pacients").value().toInt();
        setIdPacient(id_pacient);

        int id_orderEcho = items.constFind("id_orderEcho").value().toInt();
        setIdDocOrderEcho(id_orderEcho);

        set_t_organs_internal(items.constFind("t_organs_internal").value().toInt());
        set_t_urinary_system(items.constFind("t_urinary_system").value().toInt());
        set_t_prostate(items.constFind("t_prostate").value().toInt());
        set_t_gynecology(items.constFind("t_gynecology").value().toInt());
        set_t_breast(items.constFind("t_breast").value().toInt());
        set_t_thyroide(items.constFind("t_thyroid").value().toInt());
        set_t_gestation0(items.constFind("t_gestation0").value().toInt());
        set_t_gestation1(items.constFind("t_gestation1").value().toInt());
        set_t_gestation2(items.constFind("t_gestation2").value().toInt());
        initEnableBtn();

        if (m_organs_internal){
            disconnections_organs_internal(); // deconectam modificarea formei la modificarea textului
            setDataFromSystemOrgansInternal();
            connections_organs_internal();    // conectam modificarea formei la modificarea textului
        }

        if (m_urinary_system){
            disconnections_urinary_system();
            setDataFromSystemUrinary();
            connections_urinary_system();
        }

        if (m_prostate){
            disconnections_prostate();
            setDataFromTableProstate();
            connections_prostate();
        }

        if (m_gynecology){
            disconnections_gynecology();
            setDataFromTableGynecology();
            connections_gynecology();
            updateTextDateMenstruation();
        }

        if (m_breast){
            disconnections_breast();
            setDataFromTableBreast();
            connections_breast();
        }

        if (m_thyroide){
            disconnections_thyroid();
            setDataFromTableThyroid();
            connections_thyroid();
        }

        if (m_gestation0){
            disconnections_gestation0();
            setDataFromTableGestation0();
            connections_gestation0();
        }

        if (m_gestation1){
            disconnections_gestation1();
            setDataFromTableGestation1();
            connections_gestation1();
        }

        if (m_gestation2){
            disconnections_gestation2();
            setDataFromTableGestation2();
            connections_gestation2();
        }

        int id_user = items.constFind("id_users").value().toInt();
        setIdUser(id_user);

        ui->concluzion->setPlainText(items.constFind("concluzion").value());
        ui->comment->setPlainText(items.constFind("comment").value());
        ui->comment->setHidden(ui->comment->toPlainText().isEmpty());

        setWindowTitle(tr("Raport ecografic (validat) %1 %2").arg("nr." + ui->editDocNumber->text() + " din " + ui->editDocDate->dateTime().toString("dd.MM.yyyy hh:mm:ss"), "[*]"));
    }

    //----------------------------------------------------------------------------
    // video

     /** verificam daca este indicata directoriu de stocare a
      ** fisierelor video.
      ** Daca directoriu nu este indicat deschiderea documentului
      ** poate dura foarte mult din cauza cautarii prin tot
      ** hard discul a fisierelor video */
    if (globals().pathDirectoryVideo != nullptr) {
        findVideoFiles();
    };

    //----------------------------------------------------------------------------
    // logarea
    if (items.count() == 0)
        qWarning(logInfo()) << tr("Nu au fost determinate datele documentului 'Raport ecografic' cu id='%1'")
                               .arg(QString::number(m_id));

    //----------------------------------------------------------------------------
    // extragem imaginile documentului
    disconnect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified);

    loadImageOpeningDocument();
    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified);
    //----------------------------------------------------------------------------
    // conectam conexiunile
    connections_tags();
    updateStyleBtnInvestigations();
}

void DocReportEcho::slot_IdPacientChanged()
{
    if (m_idPacient == Enums::IDX::IDX_UNKNOW)
        return;

    auto indexPatient = modelPatients->match(modelPatients->index(0,0), Qt::UserRole, m_idPacient, 1, Qt::MatchExactly);
    if (!indexPatient.isEmpty()){
        ui->comboPatient->setCurrentIndex(indexPatient.first().row());
        completer->setCurrentRow(indexPatient.first().row());
        ui->comboPatient->setCurrentText(completer->currentCompletion());

        // gasim antecedente pacientului
        QString str_qry= "SELECT reportEcho.id, "
                         "tableGynecology.antecedent "
                         "FROM reportEcho "
                         "INNER JOIN tableGynecology ON tableGynecology.id_reportEcho = reportEcho.id  "
                         "WHERE reportEcho.id_pacients = :id_pacients; ";
        QSqlQuery qry;
        qry.prepare(str_qry);
        qry.bindValue(":id_pacients", QString::number(m_idPacient));
        if (qry.exec() && qry.next()){
            ui->gynecology_antecedent->setText(qry.value(1).toString());
        }
    }
    ui->comboPatient->setDisabled(!ui->comboPatient->currentText().isEmpty());
}

void DocReportEcho::slot_IdDocOrderEchoChanged()
{
    if (m_id_docOrderEcho == Enums::IDX::IDX_UNKNOW)
        return;

    QMap<QString, QString> items;
    if (db->getObjectDataById("orderEcho", m_id_docOrderEcho, items)){
        if (globals().thisMySQL){
            QString str_date = items.constFind("dateDoc").value();
            static const QRegularExpression replaceT("T");
            static const QRegularExpression removeMilliseconds("\\.000");
            str_date = str_date.replace(replaceT, " ").replace(removeMilliseconds,"");
            ui->labelOrderEcho->setText(tr("Comanda ecografica nr.%1 din %2")
                                            .arg(items.constFind("numberDoc").value(),
                                                 QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss").toString("dd-MM-yyyy hh:mm:ss")));
        } else {
            ui->labelOrderEcho->setText(tr("Comanda ecografica nr.%1 din %2")
                                            .arg(items.constFind("numberDoc").value(),
                                                 QDateTime::fromString(items.constFind("dateDoc").value(), "yyyy-MM-dd hh:mm:ss").toString("dd-MM-yyyy hh:mm:ss")));
        }
        ui->editDocNumber->setText(items.constFind("numberDoc").value());     // setam nr.documentului
        ui->editDocNumber->setDisabled(!ui->editDocNumber->text().isEmpty()); // setam editarea nr.
        if (m_idPacient == Enums::IDX::IDX_UNKNOW)
            setIdPacient(items.constFind("id_pacients").value().toInt());
    }
}

void DocReportEcho::slot_IdUserChanged()
{
    if (m_idUser == Enums::IDX::IDX_UNKNOW)
        setIdUser(globals().idUserApp);
}

void DocReportEcho::slot_CountImagesChanged()
{
    if (m_count_images > 0)
        ui->btnImages->setText(tr("Imagini (ata\310\231ate %1)").arg(QString::number(m_count_images)));
}

void DocReportEcho::slot_CountVideoChanged()
{
    if (m_count_video > 0)
        ui->btnVideo->setText(tr("Video (ata\310\231ate %1)").arg(QString::number(m_count_video)));
    else
        ui->btnVideo->setText(tr("Video"));
}

void DocReportEcho::updateTextDateMenstruation()
{
    QDate d1(ui->editDocDate->date().toString("yyyy").toInt(),
             ui->editDocDate->date().toString("MM").toInt(),
             ui->editDocDate->date().toString("dd").toInt());

    QDate d2(ui->gynecology_dateMenstruation->date().toString("yyyy").toInt(),
             ui->gynecology_dateMenstruation->date().toString("MM").toInt(),
             ui->gynecology_dateMenstruation->date().toString("dd").toInt());

    QDate birthdayPatient = QDate(1970, 01, 01);
    QDate d3;
    QMap<QString, QString> _items;
    if (db->getObjectDataById("pacients", m_idPacient, _items)){
        birthdayPatient = QDate::fromString(_items.constFind("birthday").value(), "yyyy-MM-dd");
        d3 = QDate(birthdayPatient.toString("yyyy").toInt(),
                   birthdayPatient.toString("MM").toInt(),
                   birthdayPatient.toString("dd").toInt());
    }

    QString strText;
    int resultDay = d2.daysTo(d1);
    if (resultDay > 90 && birthdayPatient != QDate(1970, 01, 01) && d3.daysTo(d1) > 48)
        if (resultDay / 365 == 0)
            strText = QString(tr("menopauza < 1 an."));
        else
            strText = QString(tr("menopauza %1 ani.").arg((resultDay / 365)));
    else
        strText = QString(tr(" a %1 zi de la ciclul menstrual").arg(resultDay));
    ui->gynecology_text_date_menstruation->setText(strText);
}

void DocReportEcho::activatedItemCompleter(const QModelIndex &index)
{
//    qDebug() << index.data(Qt::UserRole).toString()
//             << index.data(Qt::DisplayRole).toString(); // returneaza 'id'

    int _id = index.data(Qt::UserRole).toInt();    // determinam 'id'
    if (_id == Enums::IDX::IDX_UNKNOW || _id == Enums::IDX::IDX_WRITE)     // verificarea id
        return;

    setIdPacient(index.data(Qt::UserRole).toInt()); // setam 'id' pacientului
}

// *******************************************************************
// **************** VALIDAREA, PRINTAREA DOCUMENTULUI ****************

bool DocReportEcho::controlRequiredObjects()
{
    if (m_id == Enums::IDX::IDX_UNKNOW){
        QMessageBox::warning(this, tr("Controlul complet\304\203rii obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> documentului !!!<br>Adresa\310\233i-v\304\203 administratorului aplica\310\233iei"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_idPacient == Enums::IDX::IDX_UNKNOW || m_idPacient == Enums::IDX::IDX_WRITE){
        QMessageBox::warning(this, tr("Controlul complet\304\203rii obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> pacientului !!!<br>Adresa\310\233i-v\304\203 administratorului aplica\310\233iei"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_idUser == Enums::IDX::IDX_UNKNOW || m_idUser == Enums::IDX::IDX_WRITE){
        QMessageBox::warning(this, tr("Controlul complet\304\203rii obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> autorului documentului !!!<br>Adresa\310\233i-v\304\203 administratorului aplica\310\233iei"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (ui->editDocNumber->text().isEmpty()){
        QMessageBox::warning(this, tr("Controlul complet\304\203rii obiectelor"),
                             tr("Nu este indicat <b>'Num\304\203rul'</b> documentului !!!<br>Adresa\310\233i-v\304\203 administratorului aplica\310\233iei"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (ui->concluzion->toPlainText().isEmpty()){
        QMessageBox::warning(this, tr("Controlul complet\304\203rii obiectelor"),
                             tr("Nu este indicat\304\203 <b>'Concluzia'</b> raportului !!!<br>Validarea nu este posibil\304\203."),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_organs_internal && ui->organsInternal_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (organelor interne)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_urinary_system && ui->urinary_system_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (sistemului urinar)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_prostate && ui->prostate_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (prostatei)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gynecology && ui->gynecology_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (ginecologic\304\203)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_breast && ui->breast_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (gl.mamare)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_thyroide && ui->thyroid_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (gl.tiroide)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gestation0 && ui->gestation0_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (sarcina p\303\242n\304\203 la 11 s\304\203pt\304\203m\303\242ni)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gestation1 && ui->gestation1_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (sarcina 11-14 s\304\203pt\304\203m\303\242ni)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    return true;
}

void DocReportEcho::onPrint(Enums::TYPE_PRINT _typeReport, QString &filePDF)
{
    if (globals().pathTemplatesDocs.isEmpty()){

        QMessageBox::warning(this,
                             tr("Verificarea set\304\203rilor"),
                             tr("Nu este indicat directoriul cu \310\231abloanele de tipar.<br>"
                                "Tip\304\203rirea documentului nu este posibil\304\203."),
                             QMessageBox::Ok);
        return;
    }

    static int exist_logo = 0;
    static int exist_stamp_doctor = 0;
    static int exist_signature_doctor = 0;
    static int exist_stam_organization = 0;

    // *************************************************************************************
    // alocam memoria
    m_report = new LimeReport::ReportEngine(this);
    modelPatient_print = new QSqlQueryModel(this);

    // *************************************************************************************
    // titlu raportului
    m_report->setPreviewWindowTitle(tr("Raport ecografic nr.") + ui->editDocNumber->text() + tr(" din ") +
                                    ui->editDocDate->dateTime().toString("dd.MM.yyyy hh:mm:ss") + tr(" (printare)"));

    //------------------------------------------------------------------------------------------------------
    // ----- 1. logotipul
    QPixmap pix_logo = QPixmap();
    QStandardItem *img_item_logo = new QStandardItem();
    QString name_key_logo = "logo_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_logo, &pix_logo)){
        if (! globals().c_logo_byteArray.isEmpty() && pix_logo.loadFromData(globals().c_logo_byteArray)){
            globals().cache_img.insert(name_key_logo, pix_logo);
        }
    }

    // --- setam logotipul
    if (! pix_logo.isNull()) {
        img_item_logo->setData(pix_logo.scaled(300,50, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_logo = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 2. stampila organizatiei
    QPixmap pix_stamp_organization = QPixmap();
    QStandardItem* img_item_stamp_organization = new QStandardItem();
    QString name_key_stamp_organization = "stamp_organization_id-" + QString::number(globals().c_id_organizations) + "_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_stamp_organization, &pix_stamp_organization)) {
        if (! globals().main_stamp_organization.isEmpty() && pix_stamp_organization.loadFromData(globals().main_stamp_organization)){
            globals().cache_img.insert(name_key_stamp_organization, pix_stamp_organization);
        }
    }

    // --- setam stampila
    if (! pix_stamp_organization.isNull()) {
        img_item_stamp_organization->setData(pix_stamp_organization.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stam_organization = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 3. stampila doctorului
    QPixmap pix_stamp_doctor = QPixmap();
    QStandardItem* img_item_stamp_doctor = new QStandardItem();
    QString name_key_stamp_doctor = "stamp_doctor_id-" + QString::number(globals().c_id_doctor) + "_" + globals().nameUserApp;

    // --- verifiam cache
    if (! globals().cache_img.find(name_key_stamp_doctor, &pix_stamp_doctor)) {
        if (! globals().stamp_main_doctor.isEmpty() && pix_stamp_doctor.loadFromData(globals().stamp_main_doctor)){
            globals().cache_img.insert(name_key_stamp_doctor, pix_stamp_doctor);
        }
    }

    // --- setam stampila
    if (! pix_stamp_doctor.isNull()) {
        img_item_stamp_doctor->setData(pix_stamp_doctor.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_stamp_doctor = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // ----- 4. semnatura doctorului
    QPixmap pix_signature = QPixmap();
    QStandardItem* img_item_signature = new QStandardItem();
    QString name_key_signature = "signature_doctor_id-" + QString::number(globals().c_id_doctor) + "_" + globals().nameUserApp;

    // --- verificam cache
    if (! globals().cache_img.find(name_key_signature, &pix_signature)) {
        if(! globals().signature_main_doctor.isEmpty() && pix_signature.loadFromData(globals().signature_main_doctor)) {
            globals().cache_img.insert(name_key_signature, pix_signature);
        }
    }

    // --- setam semnatura
    if (! pix_signature.isNull()) {
        img_item_signature->setData(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage(), Qt::DisplayRole);
        exist_signature_doctor = 1;
    }

    //------------------------------------------------------------------------------------------------------
    // setam imaginile in model
    QList<QStandardItem *> items_img;
    items_img.append(img_item_logo);
    items_img.append(img_item_stamp_organization);
    items_img.append(img_item_stamp_doctor);
    items_img.append(img_item_signature);

    model_img = new QStandardItemModel(this);
    model_img->setColumnCount(4);
    model_img->appendRow(items_img);

    m_report->dataManager()->addModel("table_logo", model_img, true);

    // ************************************************************************************
    // setam datele organizatiei, pacientului in model
    if (modelOrganization->rowCount() > 0)
        modelOrganization->clear();
    modelOrganization->setQuery(db->getQryFromTableConstantById(globals().idUserApp));
    m_report->dataManager()->addModel("main_organization", modelOrganization, false);

    if (modelPatient_print->rowCount() > 0)
        modelPatient_print->clear();
    modelPatient_print->setQuery(db->getQryFromTablePatientById(m_idPacient));
    m_report->dataManager()->addModel("table_patient", modelPatient_print, false);

    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("v_export_pdf", (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF) ? 1 : 0);
    m_report->dataManager()->setReportVariable("v_exist_stam_organization", exist_stam_organization);
    m_report->dataManager()->setReportVariable("v_exist_stamp_doctor", exist_stamp_doctor);
    m_report->dataManager()->setReportVariable("v_exist_signature_doctor", exist_signature_doctor);
    m_report->dataManager()->setReportVariable("unitMeasure", (globals().unitMeasure == "milimetru") ? "mm" : "cm");

    this->hide();

    // ************************************************************************************
    // setam datele examinarilor in model
    if (m_organs_internal && m_urinary_system){

        // extragem datele si introducem in model + setam variabile
        modelOrgansInternal->setQuery(db->getQryForTableOrgansInternalById(m_id));
        modelUrinarySystem->setQuery(db->getQryForTableUrinarySystemById(m_id));
        m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);
        m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Complex.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Complex.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - complex: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - complex: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - complex: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_complex.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - complex: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_organs_internal && !m_urinary_system){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        modelOrgansInternal->setQuery(db->getQryForTableOrgansInternalById(m_id));
        m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Organs internal.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Organs internal.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - organs internal: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - organs internal: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - organs internal: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "organs_internal.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - organs internal: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_urinary_system && !m_organs_internal){

        // alocam memoria
        modelUrinarySystem = new QSqlQueryModel(this);

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        modelUrinarySystem->setQuery(db->getQryForTableUrinarySystemById(m_id));
        m_report->dataManager()->addModel("table_urinary_system", modelUrinarySystem, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Urinary system.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Urinary system.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - urinary system: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - urinary system: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - urinary system: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_urinary_sistem.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - urinary system: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_prostate){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("method_examination", (ui->prostate_radioBtn_transrectal->isChecked()) ? "transrectal" : "transabdominal");
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        modelProstate->setQuery(db->getQryForTableProstateById(m_id));
        m_report->dataManager()->addModel("table_prostate", modelProstate, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Prostate.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Prostate.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - prostate: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - prostate: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - prostate: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_prostate.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - prostate: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_gynecology){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("method_examination", (ui->gynecology_btn_transvaginal->isChecked()) ? "transvaginal" : "transabdominal");
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        modelGynecology->setQuery(db->getQryForTableGynecologyById(m_id));
        m_report->dataManager()->addModel("table_gynecology", modelGynecology, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gynecology.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gynecology.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - gynecology: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - gynecology: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - gynecology: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_gynecology.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gynecology: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_breast){

        // extragem datele si introducem in model + setam variabile
        modelBreast->setQuery(db->getQryForTableBreastById(m_id));
        m_report->dataManager()->addModel("table_breast", modelBreast, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Breast.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Breast.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - breast: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - breast: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - breast: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_breast.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - breast: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_thyroide){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        modelThyroid->setQuery(db->getQryForTableThyroidById(m_id));
        m_report->dataManager()->addModel("table_thyroid", modelThyroid, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Thyroid.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Thyroid.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - thyroid: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - thyroid: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - thyroid: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_thyroid.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - thyroid: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_gestation0){

        m_report->dataManager()->setReportVariable("v_lmp", ui->gestation0_LMP->date().toString("dd.MM.yyyy"));
        m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation0_probableDateBirth->date().toString("dd.MM.yyyy"));

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("ivestigation_view", group_btn_gestation0->checkedId());
        modelGestationO->setQuery(db->getQryForTableGestation0dById(m_id));
        m_report->dataManager()->addModel("table_gestation0", modelGestationO, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gestation0.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation0.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - gestation0: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - gestation0: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - gestation0: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_gestation0.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation0: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_gestation1){

        m_report->dataManager()->setReportVariable("v_lmp", ui->gestation1_LMP->date().toString("dd.MM.yyyy"));
        m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation1_probableDateBirth->date().toString("dd.MM.yyyy"));

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("ivestigation_view", group_btn_gestation1->checkedId());
        modelGestation1->setQuery(db->getQryForTableGestation1dById(m_id));
        m_report->dataManager()->addModel("table_gestation1", modelGestation1, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals().pathTemplatesDocs + "/Gestation1.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation1.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - gestation1: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW){
            qInfo(logInfo()) << tr("Printare (preview) - gestation1: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - gestation1: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_gestation1.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation1: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    if (m_gestation2){

        m_report->dataManager()->setReportVariable("v_lmp", ui->gestation2_dateMenstruation->date().toString("dd.MM.yyyy"));
        m_report->dataManager()->setReportVariable("v_probable_date_birth", ui->gestation2_probabilDateBirth->date().toString("dd.MM.yyyy"));

        modelGestation2->setQuery(db->getQryForTableGestation2(m_id));
        m_report->dataManager()->addModel("table_gestation2", modelGestation2, false);

        // sablonul
        if (! m_report->loadFromFile((ui->gestation2_trimestru2->isChecked()) ?
                                     globals().pathTemplatesDocs + "/Gestation2.lrxml" : globals().pathTemplatesDocs + "/Gestation3.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals().pathTemplatesDocs + "/Gestation2.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item_logo;
            delete img_item_signature;
            delete img_item_stamp_doctor;
            delete model_img;
            delete modelPatient_print;
            m_report->deleteLater();

            this->show();

            return;
        }

        // prezentam
        m_report->setShowProgressDialog(true);
        if (_typeReport == Enums::TYPE_PRINT::OPEN_DESIGNER){
            qInfo(logInfo()) << tr("Printare (designer) - gestation2: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        } else if (_typeReport == Enums::TYPE_PRINT::OPEN_PREVIEW) {
            qInfo(logInfo()) << tr("Printare (preview) - gestation2: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->previewReport();
        } else if (_typeReport == Enums::TYPE_PRINT::PRINT_TO_PDF){
            qInfo(logInfo()) << tr("Printare (export PDF) - gestation2: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->printToPDF(filePDF + "_gestation2.pdf");
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation2: document 'Raport ecografic' nr.%1")
                                    .arg(ui->editDocNumber->text());
            m_report->designReport();
        }
    }

    this->show();

    // *************************************************************************************
    // elibiram memoria
    model_img->deleteLater();
    modelPatient_print->deleteLater();
    m_report->deleteLater();
}

bool DocReportEcho::onWritingData()
{
    if (m_itNew && m_id == Enums::IDX::IDX_UNKNOW)
        m_id = db->getLastIdForTable("reportEcho") + 1; // incercare de a seta 'id' documentului

    if (! controlRequiredObjects())
        return false;

    if (m_post == Enums::IDX::IDX_UNKNOW)  // daca a fost apasat btnOk = propritatea trebuia sa fie m_post == idx_post
        setPost(Enums::IDX::IDX_WRITE);    // setam post = idx_write

    QString details_error;

    if (m_itNew){    
        if (! insertingDocumentDataIntoTables(details_error)){
            CustomMessage *msgBox = new CustomMessage(this);
            msgBox->setWindowTitle(tr("Validarea documentului"));
            msgBox->setTextTitle(tr("Documentul nu este validat."));
            msgBox->setDetailedText(details_error);
            msgBox->exec();
            msgBox->deleteLater();
            return false;
        }     
        if (m_post == Enums::IDX::IDX_WRITE || m_post == Enums::IDX::IDX_POST){
            popUp->setPopupText(tr("Documentul a fost %1 cu succes<br> in baza de date.")
                                .arg((m_post == Enums::IDX::IDX_WRITE) ? tr("salvat") : tr("validat")));
            popUp->show();
            setItNew(false);
        }
        qInfo(logInfo()) << tr("Documentul 'Raport ecografic' cu nr.%1 a fost creat cu succes in baza de date.")
                            .arg(ui->editDocNumber->text());
    } else {

        if (! updatingDocumentDataIntoTables(details_error)){
            CustomMessage *msgBox = new CustomMessage(this);
            msgBox->setWindowTitle(tr("Actualizarea documentului"));
            msgBox->setTextTitle(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msgBox->setDetailedText(details_error);
            msgBox->exec();
            msgBox->deleteLater();
            return false;
        }

        if (m_post == Enums::IDX::IDX_WRITE || m_post == Enums::IDX::IDX_POST){
            popUp->setPopupText(tr("Datele documentului au fost actualizate<br> cu succes."));
            popUp->show();
            setItNew(false);
        }
        qInfo(logInfo()) << tr("Documentul 'Raport ecografic' nr.%1 a fost actualizat cu succes in baza de date.")
                            .arg(QString::number(m_id));
    }

    updateDataDocOrderEcho(); // actualizarea datelor doc.Comanda ecografica - inserarea valorii atasarii imaginei

    // imaginea se inscrie in BD - vezi functia loadFile()
    // iar comentariile la imagini la validarea documentului
    updateCommentIntoTableimagesReports();

    emit PostDocument(); // pu actualizarea listei documentelor

    // initierea syncronizarii
    if (globals().cloud_srv_exist)
        initSyncDocs();

    return true;
}

void DocReportEcho::onWritingDataClose()
{
    setPost(Enums::IDX::IDX_POST); // setam proprietatea 'post'

    if (onWritingData()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Printarea documentului"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("Dori\310\233i s\304\203 printa\310\233i documentul ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        QString filePDF;
        if (msgBox.exec() == QMessageBox::Yes)
            onPrint(Enums::TYPE_PRINT::OPEN_PREVIEW, filePDF);

        QDialog::accept();
        emit mCloseThisForm();
    }
}

void DocReportEcho::onClose()
{
    this->close();
    emit mCloseThisForm();
}

// *******************************************************************
// **************** CALCULAREA VARSTEI GESTATIONLE *******************

QString DocReportEcho::calculateGestationalAge(const QDate &lmp)
{
    QDate today = QDate::currentDate();
    int daysDifference = lmp.daysTo(today);

    if (daysDifference < 0) {
        return nullptr;
    }

    int weeks = daysDifference / 7;
    int remainingDays = daysDifference % 7;

    return QString("%1s. %2z.").arg(weeks).arg(remainingDays);
}

QDate DocReportEcho::calculateDueDate(const QDate &lmp)
{
    return lmp.addDays(280);  // Adăugăm 280 de zile (40 săptămâni)
}

QDate DocReportEcho::calculateDueDateFromFetalAge(int fetalAgeWeeks, int fetalAgeDays)
{
    int fetalAgeTotalDays = (fetalAgeWeeks * 7) + fetalAgeDays;
    int remainingDaysToDueDate = 280 - fetalAgeTotalDays; // Restul până la 40 săptămâni

    return QDate::currentDate().addDays(remainingDaysToDueDate);
}

void DocReportEcho::setDueDateGestation2()
{
    int weeks, days;
    extractWeeksAndDays(ui->gestation2_fetus_age->text(), weeks, days);
    if (weeks > 0)
        ui->gestation2_probabilDateBirth->setDate(calculateDueDateFromFetalAge(weeks, days));
}

void DocReportEcho::extractWeeksAndDays(const QString &str_vg, int &weeks, int &days)
{
    static const QRegularExpression regex(R"((\d+)s\.\s*(\d+)z\.)");
    QRegularExpressionMatch match = regex.match(str_vg);

    if (match.hasMatch()) {
        weeks = match.captured(1).toInt();  // Prima grupare (\d+) -> săptămâni
        days = match.captured(2).toInt();   // A doua grupare (\d+) -> zile
    } else {
        weeks = 0;
        days = 0;
        // qDebug() << "Format invalid!";
    }
}

// *******************************************************************
// **************** CONSTRUCTIA FORMEI VIDEO *************************

void DocReportEcho::constructionFormVideo()
{
    QString str_style_btn = "QToolButton {border: 1px solid #8f8f91; "
                            "border-radius: 4px;"
                            "width: 95px;}"
                            "QToolButton:hover {background-color: rgb(234,243,250);}"
                            "QToolButton:pressed {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa)}";

    //---------------------------------------------------------------------------
    // groupBox - play list

    QGridLayout *gridLayout = new QGridLayout(ui->frameVideo);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout_video"));

    QSplitter *splitter = new QSplitter(this);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);

    QGroupBox *groupBox_list_play = new QGroupBox(splitter);
    groupBox_list_play->setObjectName(QString::fromUtf8("groupBox_list_play"));
    groupBox_list_play->setTitle(tr("Lista cu video"));
    groupBox_list_play->setMinimumSize(QSize(240, 0));
    groupBox_list_play->setMaximumSize(QSize(240, 16777215));

    QGridLayout *gridLayout_groupBox_list_play = new QGridLayout(groupBox_list_play);
    gridLayout_groupBox_list_play->setObjectName(QString::fromUtf8("gridLayout_groupBox_list_play"));

    QToolButton *btn_remove_video = new QToolButton(groupBox_list_play);
    btn_remove_video->setObjectName(QString::fromUtf8("btn_remove_video"));
    btn_remove_video->setText(tr("Elimin\304\203"));
    btn_remove_video->setIcon(QIcon(":/img/clear_x32.png"));
    btn_remove_video->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn_remove_video->setStyleSheet(str_style_btn);
    connect(btn_remove_video, &QAbstractButton::clicked, this, &DocReportEcho::clickRemoveVideo);
    connect(btn_remove_video, &QAbstractButton::clicked, this, &DocReportEcho::dataWasModified);

    gridLayout_groupBox_list_play->addWidget(btn_remove_video, 0, 1, 1, 1);

    QToolButton *btn_add_video = new QToolButton(groupBox_list_play);
    btn_add_video->setObjectName(QString::fromUtf8("btn_add_video"));
    btn_add_video->setText(tr("Adaug\304\203"));
    btn_add_video->setIcon(QIcon(":/img/add_x32.png"));
    btn_add_video->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn_add_video->setStyleSheet(str_style_btn);
    connect(btn_add_video, &QAbstractButton::clicked, this, &DocReportEcho::clickAddVideo);
    connect(btn_add_video, &QAbstractButton::clicked, this, &DocReportEcho::dataWasModified);

    gridLayout_groupBox_list_play->addWidget(btn_add_video, 0, 0, 1, 1);

    QSpacerItem *spacer_toolBar_playList = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout_groupBox_list_play->addItem(spacer_toolBar_playList, 0, 2, 1, 1);

    list_play = new QListWidget(groupBox_list_play);
    list_play->setObjectName(QString::fromUtf8("list_play"));
    connect(list_play, QOverload<int>::of(&QListWidget::currentRowChanged), this, QOverload<int>::of(&DocReportEcho::changedCurrentRowPlayList));

    gridLayout_groupBox_list_play->addWidget(list_play, 1, 0, 1, 3);

    splitter->addWidget(groupBox_list_play);

    //---------------------------------------------------------------------------
    // groupBox - player

    QGroupBox *groupBox_player = new QGroupBox(splitter);
    groupBox_player->setObjectName(QString::fromUtf8("groupBox_player"));
    groupBox_player->setTitle(tr("Player"));

    QGridLayout *gridLayout_player = new QGridLayout(groupBox_player);
    gridLayout_player->setObjectName(QString::fromUtf8("gridLayout_player"));

    txt_title_player = new QLabel(groupBox_player);
    // txt_title_player->setFrameShape(QFrame::Panel);
    txt_title_player->setMaximumHeight(40);

    QFont font;
    font.setBold(true);
    txt_title_player->setFont(font);
    txt_title_player->setStyleSheet(QString::fromUtf8("color: rgb(198, 70, 0);"));
    txt_title_player->setAlignment(Qt::AlignCenter);

    gridLayout_player->addWidget(txt_title_player, 0, 0, 1, 3);

    player = new QMediaPlayer(groupBox_player);
    player->setObjectName(QString::fromUtf8("player"));

#if defined(Q_OS_LINUX) || defined (Q_OS_WIN)
    videoWidget = new QVideoWidget(groupBox_player);
    player->setVideoOutput(videoWidget);

    gridLayout_player->addWidget(videoWidget, 1, 0, 1, 3);
#elif defined(Q_OS_MACOS)
    scene = new QGraphicsScene(this);
    videoItem = new QGraphicsVideoItem();
    videoItem->setSize(QSizeF(640, 360)); // Set video size to 640x360 (or desired size)
    scene->addItem(videoItem);

    view = new QGraphicsView(scene, groupBox_player);
    view->setScene(scene);
    view->setMinimumSize(640, 360); // Ensure view matches video size
    player->setVideoOutput(videoItem);

    gridLayout_player->addWidget(view, 1, 0, 1, 3);
#endif

    m_playButton = new QPushButton(groupBox_player);
    m_playButton->setObjectName(QString::fromUtf8("m_playButton"));
    m_playButton->setEnabled(false);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QAbstractButton::clicked, this, &DocReportEcho::clickPlayVideo);

    gridLayout_player->addWidget(m_playButton, 2, 0, 1, 1);

    m_positionSlider = new QSlider(groupBox_player);
    m_positionSlider->setObjectName(QString::fromUtf8("m_positionSlider"));
    m_positionSlider->setOrientation(Qt::Horizontal);

    gridLayout_player->addWidget(m_positionSlider, 2, 1, 1, 1);

    m_labelDuration = new QLabel(groupBox_player);
    m_labelDuration->setObjectName(QString::fromUtf8("m_labelDuration"));
    connect(m_positionSlider, QOverload<int>::of(&QAbstractSlider::sliderMoved), this, QOverload<int>::of(&DocReportEcho::setPositionSlider));

    gridLayout_player->addWidget(m_labelDuration, 2, 2, 1, 1);

    m_errorLabel = new QLabel(groupBox_player);
    m_errorLabel->setObjectName(QString::fromUtf8("m_errorLabel"));
    // m_errorLabel->setFrameShape(QFrame::Panel);
    m_errorLabel->setMaximumHeight(40);

    gridLayout_player->addWidget(m_errorLabel, 3, 0, 1, 3);

    splitter->addWidget(groupBox_player);

    gridLayout->addWidget(splitter, 0, 0, 1, 1);

    connect(player, &QMediaPlayer::playbackStateChanged, this, &DocReportEcho::mediaStateChanged);
    connect(player, &QMediaPlayer::errorChanged, this, &DocReportEcho::handleError);
    connect(player, &QMediaPlayer::positionChanged, this, &DocReportEcho::positionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &DocReportEcho::durationChanged);

    // ui->frameVideo->setLayout(gridLayout);
#if defined(Q_OS_LINUX) || defined (Q_OS_WIN)
    videoWidget->show();
#elif defined(Q_OS_MACOS)
    view->show();
#endif

}

void DocReportEcho::findVideoFiles()
{
    if (globals().pathDirectoryVideo.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Verificarea set\304\203rilor"),
                             tr("Nu este setat directoriul pentru p\304\203strarea fi\310\231ierelor video !!!"),
                             QMessageBox::Ok);
        return;
    }

    QDirIterator it(globals().pathDirectoryVideo + "/",
                    QStringList()
                    << QString::number(m_id) + "_0.mp4"
                    << QString::number(m_id) + "_1.mp4"
                    << QString::number(m_id) + "_2.mp4"
                    << QString::number(m_id) + "_3.mp4"
                    << QString::number(m_id) + "_4.mp4"
                    << QString::number(m_id) + "_5.mp4"
                    << QString::number(m_id) + "_6.mp4",
                    QDir::NoFilter,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo file(it.next());
        list_play->addItem(file.fileName());
        setCountVideo(m_count_video + 1);
    }
}

// *******************************************************************
// **************** INSERAREA DATELOR IMPLICITE A DOCUMENTULUI *******

void DocReportEcho::setDefaultDataTableLiver()
{
    if (! m_organs_internal)
        return;

    if (ui->liver_contour->text().isEmpty())
        ui->liver_contour->setText("clar");
    if (ui->liver_parenchyma->text().isEmpty())
        ui->liver_parenchyma->setText("omogena");
    if (ui->liver_ecogenity->text().isEmpty())
        ui->liver_ecogenity->setText("medie");
    if (ui->liver_formations->text().isEmpty())
        ui->liver_formations->setText("lichidiene, solide abs.");
    if (ui->liver_duct_hepatic->text().isEmpty())
        ui->liver_duct_hepatic->setText("nu sunt dilatate");
}

void DocReportEcho::setDefaultDataTableCholecist()
{
    if (! m_organs_internal)
        return;

    if (ui->cholecist_form->text().isEmpty())
        ui->cholecist_form->setText("obisnuita");
    if (ui->cholecist_formations->text().isEmpty())
        ui->cholecist_formations->setText("abs.");
}

void DocReportEcho::setDefaultDataTablePancreas()
{
    if (! m_organs_internal)
        return;

    if (ui->pancreas_ecogenity->text().isEmpty())
        ui->pancreas_ecogenity->setText("sporita");
    if (ui->pancreas_parenchyma->text().isEmpty())
        ui->pancreas_parenchyma->setText("omogena");
    if (ui->pancreas_formations->text().isEmpty())
        ui->pancreas_formations->setText("lichidiene, solide abs.");
}

void DocReportEcho::setDefaultDataTableSpleen()
{
    if (! m_organs_internal)
        return;

    if (ui->spleen_contour->text().isEmpty())
        ui->spleen_contour->setText("clar");
    if (ui->spleen_parenchyma->text().isEmpty())
        ui->spleen_parenchyma->setText("omogena");
    if (ui->spleen_formations->text().isEmpty())
        ui->spleen_formations->setText("lichidiene, solide abs.");
}

void DocReportEcho::setDefaultDataTableIntestinalLoop()
{
    if (! m_organs_internal)
        return;

    if (ui->intestinalHandles->toPlainText().isEmpty())
        ui->intestinalHandles->setPlainText("formațiuni abs., ganglioni limfatici mezenteriali 5-10 mm fără aglomerări");
}

void DocReportEcho::setDefaultDataKidney()
{
    if (! m_urinary_system)
        return;

    if (ui->kidney_formations->text().isEmpty())
        ui->kidney_formations->setText("solide, lichide abs., sectoare hiperecogene 2-3 mm fără umbră acustică");
    if (ui->kidney_pielocaliceal_left->text().isEmpty())
        ui->kidney_pielocaliceal_left->setText("nu este dilatat");
    if (ui->kidney_pielocaliceal_right->text().isEmpty())
        ui->kidney_pielocaliceal_right->setText("nu este dilatat");
    if (ui->adrenalGlands->toPlainText().isEmpty())
        ui->adrenalGlands->setPlainText("nu sunt vizibile ecografic");
}

void DocReportEcho::setDefaultDataBladder()
{
    if (! m_urinary_system)
        return;

    if (ui->bladder_formations->text().isEmpty())
        ui->bladder_formations->setText("diverticuli, calculi abs.");
}

void DocReportEcho::setDefaultDataProstate()
{
    if (! m_prostate)
        return;

    if (ui->prostate_contur->text().isEmpty())
        ui->prostate_contur->setText("clar");
    if (ui->prostate_ecogency->text().isEmpty())
        ui->prostate_ecogency->setText("scăzută");
    if (ui->prostate_ecostructure->text().isEmpty())
        ui->prostate_ecostructure->setText("omogenă");
    if (ui->prostate_formations->text().isEmpty())
        ui->prostate_formations->setText("abs.");
    if (ui->prostate_recommendation->text().isEmpty())
        ui->prostate_recommendation->setText("consulta\310\233ia urologului");
}

void DocReportEcho::setDefaultDataGynecology()
{
    if (! m_gynecology)
        return;

    if (m_id == Enums::IDX::IDX_UNKNOW){
        ui->gynecology_btn_transvaginal->setChecked(true);
        ui->gynecology_dateMenstruation->setDate(QDate::currentDate());
    }
    if (ui->gynecology_antecedent->text().isEmpty())
        ui->gynecology_antecedent->setText("abs.");
    if (ui->gynecology_uterus_pozition->text().isEmpty())
        ui->gynecology_uterus_pozition->setText("anteflexie");
    if (ui->gynecology_uterus_ecostructure->text().isEmpty())
        ui->gynecology_uterus_ecostructure->setText("omogenă");
    if (ui->gynecology_uterus_formations->toPlainText().isEmpty())
        ui->gynecology_uterus_formations->setPlainText("abs.");
    if (ui->gynecology_ecou_ecostructure->text().isEmpty())
        ui->gynecology_ecou_ecostructure->setText("omogenă");
    if (ui->gynecology_cervix_ecostructure->text().isEmpty())
        ui->gynecology_cervix_ecostructure->setText("omogenă");
    if (ui->gynecology_douglas->text().isEmpty())
        ui->gynecology_douglas->setText("liber");
    if (ui->gynecology_plex_venos->text().isEmpty())
        ui->gynecology_plex_venos->setText("nu sunt dilatate");
    if (ui->gynecology_ovary_formations_right->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_right->setPlainText("abs.");
    if (ui->gynecology_ovary_formations_left->toPlainText().isEmpty())
        ui->gynecology_ovary_formations_left->setPlainText("abs.");
    if (ui->gynecology_recommendation->text().isEmpty())
        ui->gynecology_recommendation->setText("consulta\310\233ia ginecologului");
}

void DocReportEcho::setDefaultDataBreast()
{
    if (! m_breast)
        return;

    if (ui->breast_right_ecostructure->text().isEmpty())
        ui->breast_right_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_right_duct->text().isEmpty())
        ui->breast_right_duct->setText("norm.");
    if (ui->breast_right_ligament->text().isEmpty())
        ui->breast_right_ligament->setText("norm");
    if (ui->breast_right_formations->toPlainText().isEmpty())
        ui->breast_right_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_right_ganglions->text().isEmpty())
        ui->breast_right_ganglions->setText("fără modificări patologice");

    if (ui->breast_left_ecostructure->text().isEmpty())
        ui->breast_left_ecostructure->setText("glandulară, omogenă");
    if (ui->breast_left_duct->text().isEmpty())
        ui->breast_left_duct->setText("norm.");
    if (ui->breast_left_ligament->text().isEmpty())
        ui->breast_left_ligament->setText("norm");
    if (ui->breast_left_formations->toPlainText().isEmpty())
        ui->breast_left_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_left_ganglions->text().isEmpty())
        ui->breast_left_ganglions->setText("fără modificări patologice");
}

void DocReportEcho::setDefaultDataThyroid()
{
    if (! m_thyroide)
        return;

    if (ui->thyroid_ecostructure->text().isEmpty())
        ui->thyroid_ecostructure->setText("omogenă");
    if (ui->thyroid_formations->toPlainText().isEmpty())
        ui->thyroid_formations->setPlainText("l.drept - forma\310\233iuni lichidiene, solide abs.\nl.st\303\242ng - forma\310\233iuni lichidiene, solide abs.");
    if (ui->thyroid_ganglions->text().isEmpty())
        ui->thyroid_ganglions->setText("fara modificări patologice");
    if (ui->thyroid_recommendation->text().isEmpty())
        ui->thyroid_recommendation->setText("consulta\310\233ia endocrinologului");
}

void DocReportEcho::setDefaultDataGestation0()
{
    if (! m_gestation0)
        return;

    ui->gestation0_LMP->setDate(QDate::currentDate());

    if (ui->gestation0_antecedent->text().isEmpty())
        ui->gestation0_antecedent->setText("abs.");
    if (ui->gestation0_BCF->text().isEmpty())
        ui->gestation0_BCF->setText("prezen\310\233i, ritmici");
    if (ui->gestation0_liquid_amniotic->text().isEmpty())
        ui->gestation0_liquid_amniotic->setText("omogen, transparent");
    if (ui->gestation0_miometer->text().isEmpty())
        ui->gestation0_miometer->setText("omogen; forma\310\233iuni solide, lichidiene abs.");
    if (ui->gestation0_cervix->text().isEmpty())
        ui->gestation0_cervix->setText("omogen; forma\310\233iuni solide, lichidiene abs.; \303\256nchis, lungimea 32,9 mm");
    if (ui->gestation0_ovary->text().isEmpty())
        ui->gestation0_ovary->setText("aspect ecografic normal");
    if (ui->gestation0_recommendation->text().isEmpty())
        ui->gestation0_recommendation->setText("consulta\310\233ia ginecologului, examen ecografic la 11-12 s\304\203pt\304\203m\303\242ni a sarcinei");

}

void DocReportEcho::setDefaultDataGestation1()
{
    if (! m_gestation1)
        return;

    if (ui->gestation1_antecedent->text().isEmpty())
        ui->gestation1_antecedent->setText("abs.");
    if (ui->gestation1_BCF->text().isEmpty())
        ui->gestation1_BCF->setText("prezen\310\233i, ritmici");
    if (ui->gestation1_callote_cranium->text().isEmpty())
        ui->gestation1_callote_cranium->setText("norm.");
    if (ui->gestation1_plex_choroid->text().isEmpty())
        ui->gestation1_plex_choroid->setText("norm.");
    if (ui->gestation1_vertebral_column->text().isEmpty())
        ui->gestation1_vertebral_column->setText("integră");
    if (ui->gestation1_stomach->text().isEmpty())
        ui->gestation1_stomach->setText("norm.");
    if (ui->gestation1_bladder->text().isEmpty())
        ui->gestation1_bladder->setText("norm.");
    if (ui->gestation1_diaphragm->text().isEmpty())
        ui->gestation1_diaphragm->setText("norm.");
    if (ui->gestation1_abdominal_wall->text().isEmpty())
        ui->gestation1_abdominal_wall->setText("integru");
    if (ui->gestation1_location_placenta->text().isEmpty())
        ui->gestation1_location_placenta->setText("peretele anterior");
    if (ui->gestation1_amniotic_liquid->text().isEmpty())
        ui->gestation1_amniotic_liquid->setText("omogen, transparent");
    if (ui->gestation1_miometer->text().isEmpty())
        ui->gestation1_miometer->setText("omogen; forma\310\233iuni solide, lichidiene abs.");
    if (ui->gestation1_cervix->text().isEmpty())
        ui->gestation1_cervix->setText("omogen; forma\310\233iuni solide, lichidiene abs.; \303\256nchis, lungimea 32,9 mm");
    if (ui->gestation1_ovary->text().isEmpty())
        ui->gestation1_ovary->setText("aspect ecografic normal");
    if (ui->gestation1_recommendation->text().isEmpty())
        ui->gestation1_recommendation->setText("consulta\310\233ia ginecologului, examen ecografic la 18-20 s\304\203pt\304\203m\303\242ni a sarcinei");
}

void DocReportEcho::setDefaultDataGestation2()
{
    if (! m_gestation2)
        return;

    if (ui->gestation2_recommendation->text().isEmpty())
        ui->gestation2_recommendation->setText("consulta\310\233ia ginecologului");
}

// *******************************************************************
// **************** ACCESIBILITATEA BUTOANELOR ***********************

void DocReportEcho::initEnableBtn()
{
    int m_current_page = -1;

    if (m_id == Enums::IDX::IDX_UNKNOW &&
            ! m_organs_internal &&
            ! m_urinary_system &&
            ! m_prostate &&
            ! m_gynecology &&
            ! m_breast &&
            ! m_thyroide &&
            ! m_gestation0 &&
            ! m_gestation1 &&
            ! m_gestation2)
        ui->stackedWidget->setHidden(true);
    else
        ui->stackedWidget->setHidden(false);

    ui->btnOrgansInternal->setHidden(! m_organs_internal);
    ui->btnUrinarySystem->setHidden(! m_urinary_system);
    ui->btnProstate->setHidden(! m_prostate);
    ui->btnGynecology->setHidden(! m_gynecology);
    ui->btnBreast->setHidden(! m_breast);
    ui->btnThyroid->setHidden(! m_thyroide);
    ui->btnGestation0->setHidden(! m_gestation0);
    ui->btnGestation1->setHidden(! m_gestation1);
    ui->btnGestation2->setHidden(! m_gestation2);
    if (m_gestation0 || m_gestation1 || m_gestation2)
        ui->btnNormograms->setHidden(false);
    else
        ui->btnNormograms->setHidden(true);

    ui->btnOrgansInternal->setEnabled(m_organs_internal);
    if (m_organs_internal && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_organs_internal);
        ui->btnOrgansInternal->setFocus();
        m_current_page = page_organs_internal;
    }

    ui->btnUrinarySystem->setEnabled(m_urinary_system);
    if (m_urinary_system && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_urinary_system);
        ui->btnUrinarySystem->setFocus();
        m_current_page = page_urinary_system;
    }

    ui->btnProstate->setEnabled(m_prostate);
    if (m_prostate && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_prostate);
        ui->btnProstate->setFocus();
        m_current_page = page_prostate;
    }

    ui->btnGynecology->setEnabled(m_gynecology);
    if (m_gynecology && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_gynecology);
        ui->btnGynecology->setFocus();
        m_current_page = page_gynecology;
    }

    ui->btnBreast->setEnabled(m_breast);
    if (m_breast && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_breast);
        ui->btnBreast->setFocus();
        m_current_page = page_breast;
    }

    ui->btnThyroid->setEnabled(m_thyroide);
    if (m_thyroide && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_thyroid);
        ui->btnThyroid->setFocus();
        m_current_page = page_thyroid;
    }

    ui->btnGestation0->setEnabled(m_gestation0);
    if (m_gestation0 && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_gestation0);
        ui->btnGestation0->setFocus();
        m_current_page = page_gestation0;
    }

    ui->btnGestation1->setEnabled(m_gestation1);
    if (m_gestation1 && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_gestation1);
        ui->btnGestation1->setFocus();
        m_current_page = page_gestation1;
    }

    ui->btnGestation2->setEnabled(m_gestation2);
    if (m_gestation2 && m_current_page == -1){
        ui->stackedWidget->setCurrentIndex(page_gestation2);
        ui->btnGestation2->setFocus();
        m_current_page = page_gestation2;
    }
}

void DocReportEcho::updateStyleBtnInvestigations()
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

    if (ui->stackedWidget->currentIndex() == page_organs_internal)
        ui->btnOrgansInternal->setStyleSheet(style_pressed);
    else
        ui->btnOrgansInternal->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_urinary_system)
        ui->btnUrinarySystem->setStyleSheet(style_pressed);
    else
        ui->btnUrinarySystem->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_prostate)
        ui->btnProstate->setStyleSheet(style_pressed);
    else
        ui->btnProstate->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_gynecology)
        ui->btnGynecology->setStyleSheet(style_pressed);
    else
        ui->btnGynecology->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_breast)
        ui->btnBreast->setStyleSheet(style_pressed);
    else
        ui->btnBreast->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_thyroid)
        ui->btnThyroid->setStyleSheet(style_pressed);
    else
        ui->btnThyroid->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_gestation0)
        ui->btnGestation0->setStyleSheet(style_pressed);
    else
        ui->btnGestation0->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_gestation1)
        ui->btnGestation1->setStyleSheet(style_pressed);
    else
        ui->btnGestation1->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_gestation2)
        ui->btnGestation2->setStyleSheet(style_pressed);
    else
        ui->btnGestation2->setStyleSheet(style_unpressed);

    if (ui->comment->isVisible())
        ui->btnComment->setStyleSheet(style_pressed);
    else
        ui->btnComment->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_images)
        ui->btnImages->setStyleSheet(style_pressed);
    else
        ui->btnImages->setStyleSheet(style_unpressed);

    if (ui->stackedWidget->currentIndex() == page_video)
        ui->btnVideo->setStyleSheet(style_pressed);
    else
        ui->btnVideo->setStyleSheet(style_unpressed);
}

// *******************************************************************
// **************** INITIEREA - COMPLETER & FOOTER DOCUMENTULUI ******

void DocReportEcho::initSetCompleter()
{
    if (modelPatients->rowCount() > 0)
        modelPatients->clear();

    const QString strQuery =
        globals().thisMySQL ?
        QStringLiteral(R"(
            SELECT
                pacients.id,
                CONCAT(pacients.name, ' ', pacients.fName, ', ',
                DATE_FORMAT(pacients.birthday, '%d.%m.%Y'), ', idnp: ',
                IFNULL(pacients.IDNP, '')) AS FullName
            FROM pacients
            WHERE pacients.deletionMark = 0
            ORDER BY FullName ASC;
        )")
        :
        QStringLiteral(R"(
            SELECT
                pacients.id,
                pacients.name || ' ' || pacients.fName || ', ' ||
                strftime('%d.%m.%Y', pacients.birthday) || ', idnp: ' ||
                IFNULL(pacients.IDNP, '') AS FullName
            FROM pacients
            WHERE pacients.deletionMark = 0
            ORDER BY FullName ASC;
        )");
    QSqlQuery qry;
    if (! qry.exec(strQuery)) {
        qWarning() << "Eroare exec query:" << qry.lastError().text();
        return;
    }

    // pu performanta cream container
    QList<QStandardItem*> items;

    // prelucrarea solicitarii si completarea containerului 'items'
    if (qry.exec()) {
        while (qry.next()) {
            QStandardItem *item = new QStandardItem;
            item->setData(qry.value(0).toInt(), Qt::UserRole);
            item->setData(qry.value(1).toString(), Qt::DisplayRole);
            items.append(item);
        }
    }

    // adaugam toate randurile printr-o tranzactie/simultan (eficient si rapid)
    modelPatients->invisibleRootItem()->appendRows(items);

    proxyPatient->setSourceModel(modelPatients); // setam modelul in proxyPacient

    completer->setModel(proxyPatient);                                  // setam model
    completer->setCaseSensitivity(Qt::CaseInsensitive);                 // cautare insenseibila la registru
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel); // modelul este sortat în mod sensibil.
    completer->setFilterMode(Qt::MatchContains);                        // contine elementul
    completer->setCompletionMode(QCompleter::PopupCompletion);

    ui->comboPatient->setEditable(true);
    ui->comboPatient->setCompleter(completer);

    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&DocReportEcho::activatedItemCompleter));
}

void DocReportEcho::initFooterDoc()
{
    QPixmap pixAutor = QIcon(":/img/user_x32.png").pixmap(18,18);
    QLabel* labelPix = new QLabel(this);
    labelPix->setPixmap(pixAutor);
    labelPix->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelPix->setMinimumHeight(2);

    auto labelAuthor = new QLabel(this);
    labelAuthor->setText(globals().nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

// *******************************************************************
// **************** INSERAREA DATELOR IN BD **************************

bool DocReportEcho::insertingDocumentDataIntoTables(QString &details_error)
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    auto rollbackAndFail = [&](const QString &err) {
        details_error = err;
        db->getDatabase().rollback();
        return false;
    };

    try {
        if (! insertMainDocument(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_organs_internal && ! insertOrgansInternal(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_urinary_system && ! insertUrinarySystem(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_prostate && ! insertProstate(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_gynecology && ! insertGynecology(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_breast && ! insertBreast(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_thyroide && ! insertThyroid(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_gestation0 && ! insertGestation0(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_gestation1 && ! insertGestation1(qry, details_error))
            return rollbackAndFail(details_error);
        if (m_gestation2 && ! insertGestation2(qry, details_error))
            return rollbackAndFail(details_error);

        return db->getDatabase().commit();

    } catch (...) {
        return rollbackAndFail(tr("Excepție neprevăzută la inserarea datelor documentului."));
    }
}

bool DocReportEcho::insertMainDocument(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO reportEcho (
            id, deletionMark, numberDoc,
            dateDoc, id_pacients, id_orderEcho,
            t_organs_internal, t_urinary_system,
            t_prostate, t_gynecology,
            t_breast, t_thyroid, t_gestation0,
            t_gestation1, t_gestation2, t_gestation3,
            id_users, concluzion, comment,
            attachedImages) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(m_post);
    qry.addBindValue(ui->editDocNumber->text());
    qry.addBindValue(ui->editDocDate->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    qry.addBindValue(m_idPacient);
    qry.addBindValue(m_id_docOrderEcho);

    auto b = [=](bool val) { return globals().thisMySQL ? val : int(val); };
    qry.addBindValue(b(m_organs_internal));
    qry.addBindValue(b(m_urinary_system));
    qry.addBindValue(b(m_prostate));
    qry.addBindValue(b(m_gynecology));
    qry.addBindValue(b(m_breast));
    qry.addBindValue(b(m_thyroide));
    qry.addBindValue(b(m_gestation0));
    qry.addBindValue(b(m_gestation1));
    qry.addBindValue(b(m_gestation2));
    qry.addBindValue(b(m_gestation3));

    qry.addBindValue(m_idUser);
    qry.addBindValue(ui->concluzion->toPlainText());
    qry.addBindValue((ui->comment->toPlainText().isEmpty()) ? QVariant() : ui->comment->toPlainText());
    qry.addBindValue((m_count_images == 0) ? QVariant() : 1);

    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'reportEcho' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    } else {
        // introducem date in structura pu syncronizare

    }
    return true;
}

bool DocReportEcho::insertOrgansInternal(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableLiver (
            id_reportEcho, `left`, `right`,
            contur, parenchim, ecogenity,
            formations, ductsIntrahepatic,
            porta, lienalis, concluzion,
            recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->liver_left->text());
    qry.addBindValue(ui->liver_right->text());
    qry.addBindValue(ui->liver_contour->text());
    qry.addBindValue(ui->liver_parenchyma->text());
    qry.addBindValue(ui->liver_ecogenity->text());
    qry.addBindValue(ui->liver_formations->text());
    qry.addBindValue(ui->liver_duct_hepatic->text());
    qry.addBindValue(ui->liver_porta->text());
    qry.addBindValue(ui->liver_lienalis->text());
    qry.addBindValue(ui->organsInternal_concluzion->toPlainText());
    qry.addBindValue((ui->organsInternal_recommendation->text().isEmpty()) ? QVariant() : ui->organsInternal_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableLiver' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableCholecist
    qry.prepare(R"(
        INSERT INTO tableCholecist (
            id_reportEcho,
            form,
            dimens,
            walls,
            choledoc,
            formations)
        VALUES (?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->cholecist_form->text());
    qry.addBindValue(ui->cholecist_dimens->text());
    qry.addBindValue(ui->cholecist_walls->text());
    qry.addBindValue(ui->cholecist_coledoc->text());
    qry.addBindValue(ui->cholecist_formations->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableCholecist' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tablePancreas
    qry.prepare(R"(
        INSERT INTO tablePancreas (
            id_reportEcho,
            cefal,
            corp,
            tail,
            texture,
            ecogency,
            formations)
        VALUES (?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->pancreas_cefal->text());
    qry.addBindValue(ui->pancreas_corp->text());
    qry.addBindValue(ui->pancreas_tail->text());
    qry.addBindValue(ui->pancreas_parenchyma->text());
    qry.addBindValue(ui->pancreas_ecogenity->text());
    qry.addBindValue(ui->pancreas_formations->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tablePancreas' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableSpleen
    qry.prepare(R"(
        INSERT INTO tableSpleen (
            id_reportEcho,
            dimens,
            contur,
            parenchim,
            formations)
        VALUES (?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->spleen_dimens->text());
    qry.addBindValue(ui->spleen_contour->text());
    qry.addBindValue(ui->spleen_parenchyma->text());
    qry.addBindValue(ui->spleen_formations->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableSpleen' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableIntestinalLoop
    qry.prepare(R"(
        INSERT INTO tableIntestinalLoop (
            id_reportEcho,
            formations) VALUES (?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->intestinalHandles->toPlainText());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableIntestinalLoop' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;

}

bool DocReportEcho::insertUrinarySystem(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableKidney (
            id_reportEcho,
            contour_right,
            contour_left,
            dimens_right,
            dimens_left,
            corticomed_right,
            corticomed_left,
            pielocaliceal_right,
            pielocaliceal_left,
            formations,
            suprarenal_formations,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->kidney_contur_right->currentText());
    qry.addBindValue(ui->kidney_contur_left->currentText());
    qry.addBindValue(ui->kidney_right->text());
    qry.addBindValue(ui->kidney_left->text());
    qry.addBindValue(ui->kidney_corticomed_right->text());
    qry.addBindValue(ui->kidney_corticomed_left->text());
    qry.addBindValue(ui->kidney_pielocaliceal_right->text());
    qry.addBindValue(ui->kidney_pielocaliceal_left->text());
    qry.addBindValue(ui->kidney_formations->text());
    qry.addBindValue(ui->adrenalGlands->toPlainText());
    qry.addBindValue(ui->urinary_system_concluzion->toPlainText());
    qry.addBindValue((ui->urinary_system_recommendation->text().isEmpty()) ? QVariant() : ui->urinary_system_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableKidney' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableBladder

    qry.prepare(R"(
        INSERT INTO tableBladder (
            id_reportEcho,
            volum,
            walls,
            formations)
        VALUES (?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->bladder_volum->text());
    qry.addBindValue(ui->bladder_walls->text());
    qry.addBindValue(ui->bladder_formations->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableBladder' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertProstate(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableProstate (
            id_reportEcho,
            dimens,
            volume,
            ecostructure,
            contour,
            ecogency,
            formations,
            transrectal,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->prostate_dimens->text());
    qry.addBindValue(ui->prostate_volum->text());
    qry.addBindValue(ui->prostate_ecostructure->text());
    qry.addBindValue(ui->prostate_contur->text());
    qry.addBindValue(ui->prostate_ecogency->text());
    qry.addBindValue(ui->prostate_formations->text());
    qry.addBindValue(globals().thisMySQL
                     ? QVariant(ui->prostate_radioBtn_transrectal->isChecked())
                     : QVariant(int(ui->prostate_radioBtn_transrectal->isChecked())));
    qry.addBindValue(ui->prostate_concluzion->toPlainText());
    qry.addBindValue((ui->prostate_recommendation->text().isEmpty()) ? QVariant() : ui->prostate_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableProstate' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertGynecology(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableGynecology (
            id_reportEcho,
            transvaginal,
            dateMenstruation,
            antecedent,
            uterus_dimens,
            uterus_pozition,
            uterus_ecostructure,
            uterus_formations,
            junctional_zone,
            junctional_zone_description,
            ecou_dimens,
            ecou_ecostructure,
            cervix_dimens,
            cervix_ecostructure,
            cervical_canal,
            cervical_canal_formations,
            douglas,
            plex_venos,
            ovary_right_dimens,
            ovary_left_dimens,
            ovary_right_volum,
            ovary_left_volum,
            ovary_right_follicle,
            ovary_left_follicle,
            ovary_right_formations,
            ovary_left_formations,
            fallopian_tubes,
            fallopian_tubes_formations,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(globals().thisMySQL
                     ? QVariant(ui->gynecology_btn_transvaginal->isChecked())
                     : QVariant(int(ui->gynecology_btn_transvaginal->isChecked())));
    qry.addBindValue(ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue((ui->gynecology_antecedent->text().isEmpty()) ? QVariant() : ui->gynecology_antecedent->text());
    qry.addBindValue(ui->gynecology_uterus_dimens->text());
    qry.addBindValue(ui->gynecology_uterus_pozition->text());
    qry.addBindValue(ui->gynecology_uterus_ecostructure->text());
    qry.addBindValue(ui->gynecology_uterus_formations->toPlainText());
    qry.addBindValue(ui->gynecology_combo_jonctional_zone->currentText());
    qry.addBindValue((ui->gynecology_jonctional_zone_description->text().isEmpty()) ? QVariant() : ui->gynecology_jonctional_zone_description->text());
    qry.addBindValue(ui->gynecology_ecou_dimens->text());
    qry.addBindValue(ui->gynecology_ecou_ecostructure->text());
    qry.addBindValue(ui->gynecology_cervix_dimens->text());
    qry.addBindValue(ui->gynecology_cervix_ecostructure->text());
    qry.addBindValue(ui->gynecology_combo_canal_cervical->currentText());
    qry.addBindValue((ui->gynecology_canal_cervical_formations->text().isEmpty()) ? QVariant() : ui->gynecology_canal_cervical_formations->text());
    qry.addBindValue(ui->gynecology_douglas->text());
    qry.addBindValue(ui->gynecology_plex_venos->text());
    qry.addBindValue(ui->gynecology_ovary_right_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_left_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_right_volum->text());
    qry.addBindValue(ui->gynecology_ovary_left_volum->text());
    qry.addBindValue(ui->gynecology_follicule_right->text());
    qry.addBindValue(ui->gynecology_follicule_left->text());
    qry.addBindValue(ui->gynecology_ovary_formations_right->toPlainText());
    qry.addBindValue(ui->gynecology_ovary_formations_left->toPlainText());
    qry.addBindValue(ui->gynecology_combo_fallopian_tubes->currentText());
    qry.addBindValue((ui->gynecology_fallopian_tubes_formations->text().isEmpty()) ? QVariant() : ui->gynecology_fallopian_tubes_formations->text());
    qry.addBindValue(ui->gynecology_concluzion->toPlainText());
    qry.addBindValue((ui->gynecology_recommendation->text().isEmpty()) ? QVariant() : ui->gynecology_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGynecology' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertBreast(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableBreast (
            id_reportEcho,
            breast_right_ecostrcture,
            breast_right_duct,
            breast_right_ligament,
            breast_right_formations,
            breast_right_ganglions,
            breast_left_ecostrcture,
            breast_left_duct,
            breast_left_ligament,
            breast_left_formations,
            breast_left_ganglions,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->breast_right_ecostructure->text());
    qry.addBindValue(ui->breast_right_duct->text());
    qry.addBindValue(ui->breast_right_ligament->text());
    qry.addBindValue(ui->breast_right_formations->toPlainText());
    qry.addBindValue(ui->breast_right_ganglions->text());
    qry.addBindValue(ui->breast_left_ecostructure->text());
    qry.addBindValue(ui->breast_left_duct->text());
    qry.addBindValue(ui->breast_left_ligament->text());
    qry.addBindValue(ui->breast_left_formations->toPlainText());
    qry.addBindValue(ui->breast_left_ganglions->text());
    qry.addBindValue(ui->breast_concluzion->toPlainText());
    qry.addBindValue((ui->breast_recommendation->text().isEmpty()) ? QVariant() : ui->breast_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableBreast' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertThyroid(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableThyroid (
            id_reportEcho,
            thyroid_right_dimens,
            thyroid_right_volum,
            thyroid_left_dimens,
            thyroid_left_volum,
            thyroid_istm,
            thyroid_ecostructure,
            thyroid_formations,
            thyroid_ganglions,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->thyroid_right_dimens->text());
    qry.addBindValue(ui->thyroid_right_volum->text());
    qry.addBindValue(ui->thyroid_left_dimens->text());
    qry.addBindValue(ui->thyroid_left_volum->text());
    qry.addBindValue(ui->thyroid_istm->text());
    qry.addBindValue(ui->thyroid_ecostructure->text());
    qry.addBindValue(ui->thyroid_formations->toPlainText());
    qry.addBindValue(ui->thyroid_ganglions->text());
    qry.addBindValue(ui->thyroid_concluzion->toPlainText());
    qry.addBindValue((ui->thyroid_recommendation->text().isEmpty()) ? QVariant() : ui->thyroid_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableThyroid' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertGestation0(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableGestation0 (
            id_reportEcho,
            view_examination,
            antecedent,
            gestation_age,
            GS,
            GS_age,
            CRL,
            CRL_age,
            BCF,
            liquid_amniotic,
            miometer,
            cervix,
            ovary,
            concluzion,
            recommendation,
            lmp)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(group_btn_gestation0->checkedId());
    qry.addBindValue((ui->gestation0_antecedent->text().isEmpty()) ? QVariant() : ui->gestation0_antecedent->text());
    qry.addBindValue(ui->gestation0_gestation->text());
    qry.addBindValue(ui->gestation0_GS_dimens->text());
    qry.addBindValue(ui->gestation0_GS_age->text());
    qry.addBindValue(ui->gestation0_CRL_dimens->text());
    qry.addBindValue(ui->gestation0_CRL_age->text());
    qry.addBindValue(ui->gestation0_BCF->text());
    qry.addBindValue(ui->gestation0_liquid_amniotic->text());
    qry.addBindValue(ui->gestation0_miometer->text());
    qry.addBindValue(ui->gestation0_cervix->text());
    qry.addBindValue(ui->gestation0_ovary->text());
    qry.addBindValue(ui->gestation0_concluzion->toPlainText());
    qry.addBindValue((ui->gestation0_recommendation->text().isEmpty()) ? QVariant() : ui->gestation0_recommendation->text());
    qry.addBindValue(ui->gestation0_LMP->date().toString("yyyy-MM-dd"));
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation0' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertGestation1(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        INSERT INTO tableGestation1 (
            id_reportEcho,
            view_examination,
            antecedent,
            gestation_age,
            CRL,
            CRL_age,
            BPD,
            BPD_age,
            NT,
            NT_percent,
            BN,
            BN_percent,
            BCF,
            FL,
            FL_age,
            callote_cranium,
            plex_choroid,
            vertebral_column,
            stomach,
            bladder,
            diaphragm,
            abdominal_wall,
            location_placenta,
            sac_vitelin,
            amniotic_liquid,
            miometer,
            cervix,
            ovary,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(group_btn_gestation1->checkedId());
    qry.addBindValue((ui->gestation1_antecedent->text().isEmpty()) ? QVariant() : ui->gestation1_antecedent->text());
    qry.addBindValue(ui->gestation1_gestation->text());
    qry.addBindValue(ui->gestation1_CRL_dimens->text());
    qry.addBindValue(ui->gestation1_CRL_age->text());
    qry.addBindValue(ui->gestation1_BPD_dimens->text());
    qry.addBindValue(ui->gestation1_BPD_age->text());
    qry.addBindValue(ui->gestation1_NT_dimens->text());
    qry.addBindValue(ui->gestation1_NT_percent->text());
    qry.addBindValue(ui->gestation1_BN_dimens->text());
    qry.addBindValue(ui->gestation1_BN_percent->text());
    qry.addBindValue(ui->gestation1_BCF->text());
    qry.addBindValue(ui->gestation1_FL_dimens->text());
    qry.addBindValue(ui->gestation1_FL_age->text());
    qry.addBindValue(ui->gestation1_callote_cranium->text());
    qry.addBindValue(ui->gestation1_plex_choroid->text());
    qry.addBindValue(ui->gestation1_vertebral_column->text());
    qry.addBindValue(ui->gestation1_stomach->text());
    qry.addBindValue(ui->gestation1_bladder->text());
    qry.addBindValue(ui->gestation1_diaphragm->text());
    qry.addBindValue(ui->gestation1_abdominal_wall->text());
    qry.addBindValue(ui->gestation1_location_placenta->text());
    qry.addBindValue(ui->gestation1_sac_vitelin->text());
    qry.addBindValue(ui->gestation1_amniotic_liquid->text());
    qry.addBindValue(ui->gestation1_miometer->text());
    qry.addBindValue(ui->gestation1_cervix->text());
    qry.addBindValue(ui->gestation1_ovary->text());
    qry.addBindValue(ui->gestation1_concluzion->toPlainText());
    qry.addBindValue((ui->gestation1_recommendation->text().isEmpty()) ? QVariant() : ui->gestation1_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation1' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::insertGestation2(QSqlQuery &qry, QString &err)
{
    // main table
    qry.prepare(R"(
        INSERT INTO tableGestation2 (
            id_reportEcho,
            gestation_age,
            trimestru,
            dateMenstruation,
            view_examination,
            single_multiple_pregnancy,
            single_multiple_pregnancy_description,
            antecedent,
            comment,
            concluzion,
            recommendation)
        VALUES (?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue((ui->gestation2_gestation_age->text().isEmpty()) ? QVariant() : ui->gestation2_gestation_age->text());
    qry.addBindValue((ui->gestation2_trimestru2->isChecked()) ? 2 : 3);
    qry.addBindValue(ui->gestation2_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->gestation2_view_examination->currentIndex());
    qry.addBindValue(ui->gestation2_pregnancy->currentIndex());
    qry.addBindValue((ui->gestation2_pregnancy_description->text().isEmpty()) ? QVariant() : ui->gestation2_pregnancy_description->text());
    qry.addBindValue(QVariant());
    qry.addBindValue((ui->gestation2_comment->toPlainText().isEmpty()) ? QVariant() : ui->gestation2_comment->toPlainText());
    qry.addBindValue(ui->gestation2_concluzion->toPlainText());
    qry.addBindValue((ui->gestation2_recommendation->text().isEmpty()) ? QVariant() : ui->gestation2_recommendation->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // biometry
    qry.prepare(R"(
        INSERT INTO tableGestation2_biometry (
            id_reportEcho,
            BPD,
            BPD_age,
            HC,
            HC_age,
            AC,
            AC_age,
            FL,
            FL_age,
            FetusCorresponds)
        VALUES (?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_bpd->text());
    qry.addBindValue(ui->gestation2_bpd_age->text());
    qry.addBindValue(ui->gestation2_hc->text());
    qry.addBindValue(ui->gestation2_hc_age->text());
    qry.addBindValue(ui->gestation2_ac->text());
    qry.addBindValue(ui->gestation2_ac_age->text());
    qry.addBindValue(ui->gestation2_fl->text());
    qry.addBindValue(ui->gestation2_fl_age->text());
    qry.addBindValue(ui->gestation2_fetus_age->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_biometry' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // cranium
    qry.prepare(R"(
        INSERT INTO tableGestation2_cranium (
            id_reportEcho,
            calloteCranium,
            facialeProfile,
            nasalBones,
            nasalBones_dimens,
            eyeball,
            eyeball_desciption,
            nasolabialTriangle,
            nasolabialTriangle_description,
            nasalFold)
        VALUES (?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_calloteCranium->currentIndex());
    qry.addBindValue(ui->gestation2_facialeProfile->currentIndex());
    qry.addBindValue(ui->gestation2_nasalBones->currentIndex());
    qry.addBindValue((ui->gestation2_nasalBones_dimens->text().isEmpty()) ? QVariant() : ui->gestation2_nasalBones_dimens->text());
    qry.addBindValue(ui->gestation2_eyeball->currentIndex());
    qry.addBindValue((ui->gestation2_eyeball_desciption->text().isEmpty()) ? QVariant() : ui->gestation2_eyeball_desciption->text());
    qry.addBindValue(ui->gestation2_nasolabialTriangle->currentIndex());
    qry.addBindValue((ui->gestation2_nasolabialTriangle_description->text().isEmpty()) ? QVariant() : ui->gestation2_nasolabialTriangle_description->text());
    qry.addBindValue((ui->gestation2_nasalFold->text().isEmpty()) ? QVariant() : ui->gestation2_nasalFold->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_cranium' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // SNC
    qry.prepare(R"(
        INSERT INTO tableGestation2_SNC (
            id_reportEcho,
            hemispheres,
            fissureSilvius,
            corpCalos,
            ventricularSystem,
            ventricularSystem_description,
            cavityPellucidSeptum,
            choroidalPlex,
            choroidalPlex_description,
            cerebellum,
            cerebellum_description,
            vertebralColumn,
            vertebralColumn_description)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_hemispheres->currentIndex());
    qry.addBindValue(ui->gestation2_fissureSilvius->currentIndex());
    qry.addBindValue(ui->gestation2_corpCalos->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularSystem->currentIndex());
    qry.addBindValue((ui->gestation2_ventricularSystem_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularSystem_description->text());
    qry.addBindValue(ui->gestation2_cavityPellucidSeptum->currentIndex());
    qry.addBindValue(ui->gestation2_choroidalPlex->currentIndex());
    qry.addBindValue((ui->gestation2_choroidalPlex_description->text().isEmpty()) ? QVariant() : ui->gestation2_choroidalPlex_description->text());
    qry.addBindValue(ui->gestation2_cerebellum->currentIndex());
    qry.addBindValue((ui->gestation2_cerebellum_description->text().isEmpty()) ? QVariant() : ui->gestation2_cerebellum_description->text());
    qry.addBindValue(ui->gestation2_vertebralColumn->currentIndex());
    qry.addBindValue((ui->gestation2_vertebralColumn_description->text().isEmpty()) ? QVariant() : ui->gestation2_vertebralColumn_description->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_SNC' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // heart
    qry.prepare(R"(
        INSERT INTO tableGestation2_heart (
            id_reportEcho,
            `position`,
            heartBeat,
            heartBeat_frequency,
            heartBeat_rhythm,
            pericordialCollections,
            planPatruCamere,
            planPatruCamere_description,
            ventricularEjectionPathLeft,
            ventricularEjectionPathLeft_description,
            ventricularEjectionPathRight,
            ventricularEjectionPathRight_description,
            intersectionVesselMagistral,
            intersectionVesselMagistral_description,
            planTreiVase,
            planTreiVase_description,
            archAorta,
            planBicav)
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue((ui->gestation2_heartPosition->text().isEmpty()) ? QVariant() : ui->gestation2_heartPosition->text());
    qry.addBindValue(ui->gestation2_heartBeat->currentIndex());
    qry.addBindValue((ui->gestation2_heartBeat_frequency->text().isEmpty()) ? QVariant() : ui->gestation2_heartBeat_frequency->text());
    qry.addBindValue(ui->gestation2_heartBeat_rhythm->currentIndex());
    qry.addBindValue(ui->gestation2_pericordialCollections->currentIndex());
    qry.addBindValue(ui->gestation2_planPatruCamere->currentIndex());
    qry.addBindValue((ui->gestation2_planPatruCamere_description->text().isEmpty()) ? QVariant() : ui->gestation2_planPatruCamere_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathLeft->currentIndex());
    qry.addBindValue((ui->gestation2_ventricularEjectionPathLeft_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularEjectionPathLeft_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathRight->currentIndex());
    qry.addBindValue((ui->gestation2_ventricularEjectionPathRight_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularEjectionPathRight_description->text());
    qry.addBindValue(ui->gestation2_intersectionVesselMagistral->currentIndex());
    qry.addBindValue((ui->gestation2_intersectionVesselMagistral_description->text().isEmpty()) ? QVariant() : ui->gestation2_intersectionVesselMagistral_description->text());
    qry.addBindValue(ui->gestation2_planTreiVase->currentIndex());
    qry.addBindValue((ui->gestation2_planTreiVase_description->text().isEmpty()) ? QVariant() : ui->gestation2_planTreiVase_description->text());
    qry.addBindValue(ui->gestation2_archAorta->currentIndex());
    qry.addBindValue(ui->gestation2_planBicav->currentIndex());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_heart' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // thorax
    qry.prepare(R"(
        INSERT INTO tableGestation2_thorax (
            id_reportEcho,
            pulmonaryAreas,
            pulmonaryAreas_description,
            pleuralCollections,
            diaphragm)
        VALUES (?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_pulmonaryAreas->currentIndex());
    qry.addBindValue((ui->gestation2_pulmonaryAreas_description->text().isEmpty()) ? QVariant() : ui->gestation2_pulmonaryAreas_description->text());
    qry.addBindValue(ui->gestation2_pleuralCollections->currentIndex());
    qry.addBindValue(ui->gestation2_diaphragm->currentIndex());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_thorax' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // abdomen
    qry.prepare(R"(
        INSERT INTO tableGestation2_abdomen (
            id_reportEcho,
            abdominalWall,
            abdominalCollections,
            stomach,
            stomach_description,
            abdominalOrgans,
            cholecist,
            cholecist_description,
            intestine,
            intestine_description)
        VALUES (?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_abdominalWall->currentIndex());
    qry.addBindValue(ui->gestation2_abdominalCollections->currentIndex());
    qry.addBindValue(ui->gestation2_stomach->currentIndex());
    qry.addBindValue((ui->gestation2_stomach_description->text().isEmpty()) ? QVariant() : ui->gestation2_stomach_description->text());
    qry.addBindValue(ui->gestation2_abdominalOrgans->currentIndex());
    qry.addBindValue(ui->gestation2_cholecist->currentIndex());
    qry.addBindValue((ui->gestation2_cholecist_description->text().isEmpty()) ? QVariant() : ui->gestation2_cholecist_description->text());
    qry.addBindValue(ui->gestation2_intestine->currentIndex());
    qry.addBindValue((ui->gestation2_intestine_description->text().isEmpty()) ? QVariant() : ui->gestation2_intestine_description->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_abdomen' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // s.urinar
    qry.prepare(R"(
        INSERT INTO tableGestation2_urinarySystem (
            id_reportEcho,
            kidneys,
            kidneys_descriptions,
            ureter,
            ureter_descriptions,
            bladder)
        VALUES (?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_kidneys->currentIndex());
    qry.addBindValue((ui->gestation2_kidneys_description->text().isEmpty()) ? QVariant() : ui->gestation2_kidneys_description->text());
    qry.addBindValue(ui->gestation2_ureter->currentIndex());
    qry.addBindValue((ui->gestation2_ureter_description->text().isEmpty()) ? QVariant() : ui->gestation2_ureter_description->text());
    qry.addBindValue(ui->gestation2_bladder->currentIndex());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_urinarySystem' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // other
    qry.prepare(R"(
        INSERT INTO tableGestation2_other (
            id_reportEcho,
            externalGenitalOrgans,
            externalGenitalOrgans_aspect,
            extremities,
            extremities_descriptions,
            fetusMass,
            placenta,
            placentaLocalization,
            placentaDegreeMaturation,
            placentaDepth,
            placentaStructure,
            placentaStructure_descriptions,
            umbilicalCordon,
            umbilicalCordon_description,
            insertionPlacenta,
            amnioticIndex,
            amnioticIndexAspect,
            amnioticBedDepth,
            cervix,
            cervix_description)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue(ui->gestation2_fetusSex->currentIndex());
    qry.addBindValue(QVariant());
    qry.addBindValue(ui->gestation2_extremities->currentIndex());
    qry.addBindValue((ui->gestation2_extremities_description->text().isEmpty()) ? QVariant() : ui->gestation2_extremities_description->text());
    qry.addBindValue((ui->gestation2_fetusMass->text().isEmpty()) ? QVariant() : ui->gestation2_fetusMass->text());
    qry.addBindValue(ui->gestation2_placenta->currentIndex());
    qry.addBindValue((ui->gestation2_placenta_localization->text().isEmpty()) ? QVariant() : ui->gestation1_location_placenta->text());
    qry.addBindValue((ui->gestation2_placentaDegreeMaturation->text().isEmpty()) ? QVariant() : ui->gestation2_placentaDegreeMaturation->text());
    qry.addBindValue((ui->gestation2_placentaDepth->text().isEmpty()) ? QVariant() : ui->gestation2_placentaDepth->text());
    qry.addBindValue(ui->gestation2_placentaStructure->currentIndex());
    qry.addBindValue((ui->gestation2_placentaStructure_description->text().isEmpty()) ? QVariant() : ui->gestation2_placentaStructure_description->text());
    qry.addBindValue(ui->gestation2_umbilicalCordon->currentIndex());
    qry.addBindValue((ui->gestation2_umbilicalCordon_description->text().isEmpty()) ? QVariant() : ui->gestation2_umbilicalCordon_description->text());
    qry.addBindValue(ui->gestation2_insertionPlacenta->currentIndex());
    qry.addBindValue((ui->gestation2_amnioticIndex->text().isEmpty()) ? QVariant() : ui->gestation2_amnioticIndex->text());
    qry.addBindValue(ui->gestation2_amnioticIndexAspect->currentIndex());
    qry.addBindValue((ui->gestation2_amnioticBedDepth->text().isEmpty()) ? QVariant() : ui->gestation2_amnioticBedDepth->text());
    qry.addBindValue((ui->gestation2_cervix->text().isEmpty()) ? QVariant() : ui->gestation2_cervix->text());
    qry.addBindValue((ui->gestation2_cervix_description->text().isEmpty()) ? QVariant() : ui->gestation2_cervix_description->text());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_other' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // doppler
    qry.prepare(R"(
        INSERT INTO tableGestation2_doppler (
            id_reportEcho,
            ombilic_PI,
            ombilic_RI,
            ombilic_SD,
            ombilic_flux,
            cerebral_PI,
            cerebral_RI,
            cerebral_SD,
            cerebral_flux,
            uterRight_PI,
            uterRight_RI,
            uterRight_SD,
            uterRight_flux,
            uterLeft_PI,
            uterLeft_RI,
            uterLeft_SD,
            uterLeft_flux,
            ductVenos)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);
    )");
    qry.addBindValue(m_id);
    qry.addBindValue((ui->gestation2_ombilic_PI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_PI->text());
    qry.addBindValue((ui->gestation2_ombilic_RI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_RI->text());
    qry.addBindValue((ui->gestation2_ombilic_SD->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_SD->text());
    qry.addBindValue(ui->gestation2_ombilic_flux->currentIndex());
    qry.addBindValue((ui->gestation2_cerebral_PI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_PI->text());
    qry.addBindValue((ui->gestation2_cerebral_RI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_RI->text());
    qry.addBindValue((ui->gestation2_cerebral_SD->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_SD->text());
    qry.addBindValue(ui->gestation2_cerebral_flux->currentIndex());
    qry.addBindValue((ui->gestation2_uterRight_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_PI->text());
    qry.addBindValue((ui->gestation2_uterRight_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_RI->text());
    qry.addBindValue((ui->gestation2_uterRight_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_SD->text());
    qry.addBindValue(ui->gestation2_uterRight_flux->currentIndex());
    qry.addBindValue((ui->gestation2_uterLeft_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_PI->text());
    qry.addBindValue((ui->gestation2_uterLeft_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_RI->text());
    qry.addBindValue((ui->gestation2_uterLeft_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_SD->text());
    qry.addBindValue(ui->gestation2_uterLeft_flux->currentIndex());
    qry.addBindValue(ui->gestation2_ductVenos->currentIndex());
    if (! qry.exec()) {
        err = tr("Eroarea inserării datelor documentului nr.%1 în tabela 'tableGestation2_doppler' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

// *******************************************************************
// **************** ACTUALIZAREA DATELOR IN BD ***********************

bool DocReportEcho::updatingDocumentDataIntoTables(QString &details_error)
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    auto rollbackAndFail = [&](const QString &err) {
        details_error = err;
        db->getDatabase().rollback();
        return false;
    };

    try {

        if (! updateMainDocument(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateOrgansInternal(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateUrinarySystem(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateProstate(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateGynecology(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateBreast(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateThyroid(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateGestation0(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateGestation1(qry, details_error))
            return rollbackAndFail(details_error);
        if (! updateGestation2(qry, details_error))
            return rollbackAndFail(details_error);

        return db->getDatabase().commit();

    } catch (...) {
        return rollbackAndFail(tr("Excepție neprevăzută la actualizarea datelor documentului."));
    }
}

bool DocReportEcho::updateMainDocument(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE reportEcho SET
            deletionMark = ?,
            numberDoc    = ?,
            dateDoc      = ?,
            id_pacients  = ?,
            id_orderEcho = ?,
            t_organs_internal = ?,
            t_urinary_system  = ?,
            t_prostate   = ?,
            t_gynecology = ?,
            t_breast     = ?,
            t_thyroid    = ?,
            t_gestation0 = ?,
            t_gestation1 = ?,
            t_gestation2 = ?,
            t_gestation3 = ?,
            id_users     = ?,
            concluzion   = ?,
            comment      = ?,
            attachedImages = ?
        WHERE
            id = ?;
    )");

    qry.addBindValue(m_post);
    qry.addBindValue(ui->editDocNumber->text());
    qry.addBindValue(ui->editDocDate->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    qry.addBindValue(m_idPacient);
    qry.addBindValue(m_id_docOrderEcho);

    auto b = [=](bool val) { return globals().thisMySQL ? val : int(val); };
    qry.addBindValue(b(m_organs_internal));
    qry.addBindValue(b(m_urinary_system));
    qry.addBindValue(b(m_prostate));
    qry.addBindValue(b(m_gynecology));
    qry.addBindValue(b(m_breast));
    qry.addBindValue(b(m_thyroide));
    qry.addBindValue(b(m_gestation0));
    qry.addBindValue(b(m_gestation1));
    qry.addBindValue(b(m_gestation2));
    qry.addBindValue(b(m_gestation3));

    qry.addBindValue(m_idUser);
    qry.addBindValue(ui->concluzion->toPlainText());
    qry.addBindValue((ui->comment->toPlainText().isEmpty())
                     ? QVariant()
                     : ui->comment->toPlainText());
    qry.addBindValue((m_count_images == 0) ? QVariant() : 1);
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'reportEcho' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateOrgansInternal(QSqlQuery &qry, QString &err)
{
    //*********************************************
    // -- tableLiver
    qry.prepare(R"(
        UPDATE
            tableLiver
        SET
            `left` = ?,
            `right` = ?,
            contur = ?,
            parenchim = ?,
            ecogenity = ?,
            formations = ?,
            ductsIntrahepatic = ?,
            porta = ?,
            lienalis = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");

    qry.addBindValue(ui->liver_left->text());
    qry.addBindValue(ui->liver_right->text());
    qry.addBindValue(ui->liver_contour->text());
    qry.addBindValue(ui->liver_parenchyma->text());
    qry.addBindValue(ui->liver_ecogenity->text());
    qry.addBindValue(ui->liver_formations->text());
    qry.addBindValue(ui->liver_duct_hepatic->text());
    qry.addBindValue(ui->liver_porta->text());
    qry.addBindValue(ui->liver_lienalis->text());
    qry.addBindValue(ui->organsInternal_concluzion->toPlainText());
    qry.addBindValue((ui->organsInternal_recommendation->text().isEmpty())
                     ? QVariant()
                     : ui->organsInternal_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableLiver' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableCholecist
    qry.prepare(R"(
        UPDATE
            tableCholecist
        SET
            form = ?,
            dimens = ?,
            walls = ?,
            choledoc = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->cholecist_form->text());
    qry.addBindValue(ui->cholecist_dimens->text());
    qry.addBindValue(ui->cholecist_walls->text());
    qry.addBindValue(ui->cholecist_coledoc->text());
    qry.addBindValue(ui->cholecist_formations->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableCholecist' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tablePancreas
    qry.prepare(R"(
        UPDATE
            tablePancreas
        SET
            cefal = ?,
            corp = ?,
            tail = ?,
            texture = ?,
            ecogency = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->pancreas_cefal->text());
    qry.addBindValue(ui->pancreas_corp->text());
    qry.addBindValue(ui->pancreas_tail->text());
    qry.addBindValue(ui->pancreas_parenchyma->text());
    qry.addBindValue(ui->pancreas_ecogenity->text());
    qry.addBindValue(ui->pancreas_formations->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tablePancreas' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableSpleen
    qry.prepare(R"(
        UPDATE
            tableSpleen
        SET
            dimens = ?,
            contur = ?,
            parenchim = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->spleen_dimens->text());
    qry.addBindValue(ui->spleen_contour->text());
    qry.addBindValue(ui->spleen_parenchyma->text());
    qry.addBindValue(ui->spleen_formations->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableSpleen' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableIntestinalLoop
    qry.prepare(R"(
        UPDATE
            tableIntestinalLoop
        SET
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->intestinalHandles->toPlainText());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableIntestinalLoop' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateUrinarySystem(QSqlQuery &qry, QString &err)
{
    //*********************************************
    // -- tableKidney
    qry.prepare(R"(
        UPDATE
            tableKidney
        SET
            contour_right = ?,
            contour_left = ?,
            dimens_right = ?,
            dimens_left = ?,
            corticomed_right = ?,
            corticomed_left = ?,
            pielocaliceal_right = ?,
            pielocaliceal_left = ?,
            formations = ?,
            suprarenal_formations = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");

    qry.addBindValue(ui->kidney_contur_right->currentText());
    qry.addBindValue(ui->kidney_contur_left->currentText());
    qry.addBindValue(ui->kidney_right->text());
    qry.addBindValue(ui->kidney_left->text());
    qry.addBindValue(ui->kidney_corticomed_right->text());
    qry.addBindValue(ui->kidney_corticomed_left->text());
    qry.addBindValue(ui->kidney_pielocaliceal_right->text());
    qry.addBindValue(ui->kidney_pielocaliceal_left->text());
    qry.addBindValue(ui->kidney_formations->text());
    qry.addBindValue(ui->adrenalGlands->toPlainText());
    qry.addBindValue(ui->urinary_system_concluzion->toPlainText());
    qry.addBindValue((ui->urinary_system_recommendation->text().isEmpty())
                     ? QVariant()
                     : ui->urinary_system_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableKidney' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    //*********************************************
    // -- tableBladder
    qry.prepare(R"(
        UPDATE
            tableBladder
        SET
            volum = ?,
            walls = ?,
            formations = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->bladder_volum->text());
    qry.addBindValue(ui->bladder_walls->text());
    qry.addBindValue(ui->bladder_formations->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableBladder' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateProstate(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableProstate
        SET
            dimens = ?,
            volume = ?,
            ecostructure = ?,
            contour = ?,
            ecogency = ?,
            formations = ?,
            transrectal = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->prostate_dimens->text());
    qry.addBindValue(ui->prostate_volum->text());
    qry.addBindValue(ui->prostate_ecostructure->text());
    qry.addBindValue(ui->prostate_contur->text());
    qry.addBindValue(ui->prostate_ecogency->text());
    qry.addBindValue(ui->prostate_formations->text());
    qry.addBindValue(globals().thisMySQL
                     ? QVariant(ui->prostate_radioBtn_transrectal->isChecked())
                     : QVariant(int(ui->prostate_radioBtn_transrectal->isChecked())));
    qry.addBindValue(ui->prostate_concluzion->toPlainText());
    qry.addBindValue((ui->prostate_recommendation->text().isEmpty())
                     ? QVariant()
                     : ui->prostate_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableProstate' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateGynecology(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableGynecology
        SET
            transvaginal = ?,
            dateMenstruation = ?,
            antecedent = ?,
            uterus_dimens = ?,
            uterus_pozition = ?,
            uterus_ecostructure = ?,
            uterus_formations = ?,
            junctional_zone = ?,
            junctional_zone_description = ?,
            ecou_dimens = ?,
            ecou_ecostructure = ?,
            cervix_dimens = ?,
            cervix_ecostructure = ?,
            cervical_canal = ?,
            cervical_canal_formations = ?,
            douglas = ?,
            plex_venos = ?,
            ovary_right_dimens = ?,
            ovary_left_dimens = ?,
            ovary_right_volum = ?,
            ovary_left_volum = ?,
            ovary_right_follicle = ?,
            ovary_left_follicle = ?,
            ovary_right_formations = ?,
            ovary_left_formations = ?,
            fallopian_tubes = ?,
            fallopian_tubes_formations = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(globals().thisMySQL
                     ? QVariant(ui->gynecology_btn_transvaginal->isChecked())
                     : QVariant(int(ui->gynecology_btn_transvaginal->isChecked())));
    qry.addBindValue(ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->gynecology_antecedent->text().isEmpty()
                     ? QVariant()
                     : ui->gynecology_antecedent->text());
    qry.addBindValue(ui->gynecology_uterus_dimens->text());
    qry.addBindValue(ui->gynecology_uterus_pozition->text());
    qry.addBindValue(ui->gynecology_uterus_ecostructure->text());
    qry.addBindValue(ui->gynecology_uterus_formations->toPlainText());
    qry.addBindValue(ui->gynecology_combo_jonctional_zone->currentText());
    qry.addBindValue(ui->gynecology_jonctional_zone_description->text().isEmpty()
                     ? QVariant()
                     : ui->gynecology_jonctional_zone_description->text());
    qry.addBindValue(ui->gynecology_ecou_dimens->text());
    qry.addBindValue(ui->gynecology_ecou_ecostructure->text());
    qry.addBindValue(ui->gynecology_cervix_dimens->text());
    qry.addBindValue(ui->gynecology_cervix_ecostructure->text());
    qry.addBindValue(ui->gynecology_combo_canal_cervical->currentText());
    qry.addBindValue(ui->gynecology_canal_cervical_formations->text().isEmpty()
                     ? QVariant()
                     : ui->gynecology_canal_cervical_formations->text());
    qry.addBindValue(ui->gynecology_douglas->text());
    qry.addBindValue(ui->gynecology_plex_venos->text());
    qry.addBindValue(ui->gynecology_ovary_right_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_left_dimens->text());
    qry.addBindValue(ui->gynecology_ovary_right_volum->text());
    qry.addBindValue(ui->gynecology_ovary_left_volum->text());
    qry.addBindValue(ui->gynecology_follicule_right->text());
    qry.addBindValue(ui->gynecology_follicule_left->text());
    qry.addBindValue(ui->gynecology_ovary_formations_right->toPlainText());
    qry.addBindValue(ui->gynecology_ovary_formations_left->toPlainText());
    qry.addBindValue(ui->gynecology_combo_fallopian_tubes->currentText());
    qry.addBindValue(ui->gynecology_fallopian_tubes_formations->text().isEmpty()
                     ? QVariant()
                     : ui->gynecology_fallopian_tubes_formations->text());
    qry.addBindValue(ui->gynecology_concluzion->toPlainText());
    qry.addBindValue(ui->gynecology_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->gynecology_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGynecology' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateBreast(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableBreast
        SET
            breast_right_ecostrcture = ?,
            breast_right_duct = ?,
            breast_right_ligament = ?,
            breast_right_formations = ?,
            breast_right_ganglions = ?,
            breast_left_ecostrcture = ?,
            breast_left_duct = ?,
            breast_left_ligament = ?,
            breast_left_formations = ?,
            breast_left_ganglions = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->breast_right_ecostructure->text());
    qry.addBindValue(ui->breast_right_duct->text());
    qry.addBindValue(ui->breast_right_ligament->text());
    qry.addBindValue(ui->breast_right_formations->toPlainText());
    qry.addBindValue(ui->breast_right_ganglions->text());
    qry.addBindValue(ui->breast_left_ecostructure->text());
    qry.addBindValue(ui->breast_left_duct->text());
    qry.addBindValue(ui->breast_left_ligament->text());
    qry.addBindValue(ui->breast_left_formations->toPlainText());
    qry.addBindValue(ui->breast_left_ganglions->text());
    qry.addBindValue(ui->breast_concluzion->toPlainText());
    qry.addBindValue(ui->breast_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->breast_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableBreast' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateThyroid(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableThyroid
        SET
            thyroid_right_dimens = ?,
            thyroid_right_volum = ?,
            thyroid_left_dimens = ?,
            thyroid_left_volum = ?,
            thyroid_istm = ?,
            thyroid_ecostructure = ?,
            thyroid_formations = ?,
            thyroid_ganglions = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->thyroid_right_dimens->text());
    qry.addBindValue(ui->thyroid_right_volum->text());
    qry.addBindValue(ui->thyroid_left_dimens->text());
    qry.addBindValue(ui->thyroid_left_volum->text());
    qry.addBindValue(ui->thyroid_istm->text());
    qry.addBindValue(ui->thyroid_ecostructure->text());
    qry.addBindValue(ui->thyroid_formations->toPlainText());
    qry.addBindValue(ui->thyroid_ganglions->text());
    qry.addBindValue(ui->thyroid_concluzion->toPlainText());
    qry.addBindValue(ui->thyroid_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->thyroid_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableThyroid' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateGestation0(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableGestation0
        SET
            view_examination = ?,
            antecedent = ?,
            gestation_age = ?,
            GS = ?,
            GS_age = ?,
            CRL = ?,
            CRL_age = ?,
            BCF = ?,
            liquid_amniotic = ?,
            miometer = ?,
            cervix = ?,
            ovary = ?,
            concluzion = ?,
            recommendation = ?,
            lmp = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(group_btn_gestation0->checkedId()),
    qry.addBindValue(ui->gestation0_antecedent->text().isEmpty()
                     ? QVariant()
                     : ui->gestation0_antecedent->text());
    qry.addBindValue(ui->gestation0_gestation->text());
    qry.addBindValue(ui->gestation0_GS_dimens->text());
    qry.addBindValue(ui->gestation0_GS_age->text());
    qry.addBindValue(ui->gestation0_CRL_dimens->text());
    qry.addBindValue(ui->gestation0_CRL_age->text());
    qry.addBindValue(ui->gestation0_BCF->text());
    qry.addBindValue(ui->gestation0_liquid_amniotic->text());
    qry.addBindValue(ui->gestation0_miometer->text());
    qry.addBindValue(ui->gestation0_cervix->text());
    qry.addBindValue(ui->gestation0_ovary->text());
    qry.addBindValue(ui->gestation0_concluzion->toPlainText());
    qry.addBindValue(ui->gestation0_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->gestation0_recommendation->text());
    qry.addBindValue(ui->gestation0_LMP->date().toString("yyyy-MM-dd"));
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation0' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateGestation1(QSqlQuery &qry, QString &err)
{
    qry.prepare(R"(
        UPDATE
            tableGestation1
        SET
            view_examination = ?,
            antecedent = ?,
            gestation_age = ?,
            CRL = ?,
            CRL_age = ?,
            BPD = ?,
            BPD_age = ?,
            NT = ?,
            NT_percent = ?,
            BN = ?,
            BN_percent = ?,
            BCF = ?,
            FL = ?,
            FL_age = ?,
            callote_cranium = ?,
            plex_choroid = ?,
            vertebral_column = ?,
            stomach = ?,
            bladder = ?,
            diaphragm = ?,
            abdominal_wall = ?,
            location_placenta = ?,
            sac_vitelin = ?,
            amniotic_liquid = ?,
            miometer = ?,
            cervix = ?,
            ovary = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(group_btn_gestation1->checkedId()),
    qry.addBindValue(ui->gestation1_antecedent->text().isEmpty()
                     ? QVariant()
                     : ui->gestation1_antecedent->text());
    qry.addBindValue(ui->gestation1_gestation->text());
    qry.addBindValue(ui->gestation1_CRL_dimens->text());
    qry.addBindValue(ui->gestation1_CRL_age->text());
    qry.addBindValue(ui->gestation1_BPD_dimens->text());
    qry.addBindValue(ui->gestation1_BPD_age->text());
    qry.addBindValue(ui->gestation1_NT_dimens->text());
    qry.addBindValue(ui->gestation1_NT_percent->text());
    qry.addBindValue(ui->gestation1_BN_dimens->text());
    qry.addBindValue(ui->gestation1_BN_percent->text());
    qry.addBindValue(ui->gestation1_BCF->text());
    qry.addBindValue(ui->gestation1_FL_dimens->text());
    qry.addBindValue(ui->gestation1_FL_age->text());
    qry.addBindValue(ui->gestation1_callote_cranium->text());
    qry.addBindValue(ui->gestation1_plex_choroid->text());
    qry.addBindValue(ui->gestation1_vertebral_column->text());
    qry.addBindValue(ui->gestation1_stomach->text());
    qry.addBindValue(ui->gestation1_bladder->text());
    qry.addBindValue(ui->gestation1_diaphragm->text());
    qry.addBindValue(ui->gestation1_abdominal_wall->text());
    qry.addBindValue(ui->gestation1_location_placenta->text());
    qry.addBindValue(ui->gestation1_sac_vitelin->text());
    qry.addBindValue(ui->gestation1_amniotic_liquid->text());
    qry.addBindValue(ui->gestation1_miometer->text());
    qry.addBindValue(ui->gestation1_cervix->text());
    qry.addBindValue(ui->gestation1_ovary->text());
    qry.addBindValue(ui->gestation1_concluzion->toPlainText());
    qry.addBindValue(ui->gestation1_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->gestation1_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation1' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

bool DocReportEcho::updateGestation2(QSqlQuery &qry, QString &err)
{
    // main table
    qry.prepare(R"(
        UPDATE
            tableGestation2
        SET
            gestation_age = ?,
            trimestru = ?,
            dateMenstruation = ?,
            view_examination = ?,
            single_multiple_pregnancy = ?,
            single_multiple_pregnancy_description = ?,
            antecedent = ?,
            comment = ?,
            concluzion = ?,
            recommendation = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_gestation_age->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_gestation_age->text());
    qry.addBindValue(ui->gestation2_trimestru2->isChecked() ? 2 : 3);
    qry.addBindValue(ui->gestation2_dateMenstruation->date().toString("yyyy-MM-dd"));
    qry.addBindValue(ui->gestation2_view_examination->currentIndex());
    qry.addBindValue(ui->gestation2_pregnancy->currentIndex());
    qry.addBindValue(ui->gestation2_pregnancy_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_pregnancy_description->text());
    qry.addBindValue(QVariant());
    qry.addBindValue(ui->gestation2_comment->toPlainText().isEmpty()
                     ? QVariant()
                     : ui->gestation2_comment->toPlainText());
    qry.addBindValue(ui->gestation2_concluzion->toPlainText());
    qry.addBindValue(ui->gestation2_recommendation->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_recommendation->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // biometry
    qry.prepare(R"(
        UPDATE
            tableGestation2_biometry
        SET
            BPD = ?,
            BPD_age = ?,
            HC = ?,
            HC_age = ?,
            AC = ?,
            AC_age = ?,
            FL = ?,
            FL_age = ?,
            FetusCorresponds = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_bpd->text());
    qry.addBindValue(ui->gestation2_bpd_age->text());
    qry.addBindValue(ui->gestation2_hc->text());
    qry.addBindValue(ui->gestation2_hc_age->text());
    qry.addBindValue(ui->gestation2_ac->text());
    qry.addBindValue(ui->gestation2_ac_age->text());
    qry.addBindValue(ui->gestation2_fl->text());
    qry.addBindValue(ui->gestation2_fl_age->text());
    qry.addBindValue(ui->gestation2_fetus_age->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_biometry' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // cranium
    qry.prepare(R"(
        UPDATE
            tableGestation2_cranium
        SET
            calloteCranium = ?,
            facialeProfile = ?,
            nasalBones = ?,
            nasalBones_dimens = ?,
            eyeball = ?,
            eyeball_desciption = ?,
            nasolabialTriangle = ?,
            nasolabialTriangle_description = ?,
            nasalFold = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_calloteCranium->currentIndex());
    qry.addBindValue(ui->gestation2_facialeProfile->currentIndex());
    qry.addBindValue(ui->gestation2_nasalBones->currentIndex());
    qry.addBindValue(ui->gestation2_nasalBones_dimens->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_nasalBones_dimens->text());
    qry.addBindValue(ui->gestation2_eyeball->currentIndex());
    qry.addBindValue(ui->gestation2_eyeball_desciption->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_eyeball_desciption->text());
    qry.addBindValue(ui->gestation2_nasolabialTriangle->currentIndex());
    qry.addBindValue(ui->gestation2_nasolabialTriangle_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_nasolabialTriangle_description->text());
    qry.addBindValue(ui->gestation2_nasalFold->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_nasalFold->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_cranium' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // SNC
    qry.prepare(R"(
        UPDATE
            tableGestation2_SNC
        SET
            hemispheres = ?,
            fissureSilvius = ?,
            corpCalos = ?,
            ventricularSystem = ?,
            ventricularSystem_description = ?,
            cavityPellucidSeptum = ?,
            choroidalPlex = ?,
            choroidalPlex_description = ?,
            cerebellum = ?,
            cerebellum_description = ?,
            vertebralColumn = ?,
            vertebralColumn_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_hemispheres->currentIndex());
    qry.addBindValue(ui->gestation2_fissureSilvius->currentIndex());
    qry.addBindValue(ui->gestation2_corpCalos->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularSystem->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularSystem_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ventricularSystem_description->text());
    qry.addBindValue(ui->gestation2_cavityPellucidSeptum->currentIndex());
    qry.addBindValue(ui->gestation2_choroidalPlex->currentIndex());
    qry.addBindValue(ui->gestation2_choroidalPlex_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_choroidalPlex_description->text());
    qry.addBindValue(ui->gestation2_cerebellum->currentIndex());
    qry.addBindValue(ui->gestation2_cerebellum_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cerebellum_description->text());
    qry.addBindValue(ui->gestation2_vertebralColumn->currentIndex());
    qry.addBindValue(ui->gestation2_vertebralColumn_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_vertebralColumn_description->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_SNC' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // heart
    qry.prepare(R"(
        UPDATE
            tableGestation2_heart
        SET
            `position` = ?,
            heartBeat = ?,
            heartBeat_frequency = ?,
            heartBeat_rhythm = ?,
            pericordialCollections = ?,
            planPatruCamere = ?,
            planPatruCamere_description = ?,
            ventricularEjectionPathLeft = ?,
            ventricularEjectionPathLeft_description = ?,
            ventricularEjectionPathRight = ?,
            ventricularEjectionPathRight_description = ?,
            intersectionVesselMagistral = ?,
            intersectionVesselMagistral_description = ?,
            planTreiVase = ?,
            planTreiVase_description = ?,
            archAorta = ?,
            planBicav = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_heartPosition->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_heartPosition->text());
    qry.addBindValue(ui->gestation2_heartBeat->currentIndex());
    qry.addBindValue(ui->gestation2_heartBeat_frequency->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_heartBeat_frequency->text());
    qry.addBindValue(ui->gestation2_heartBeat_rhythm->currentIndex());
    qry.addBindValue(ui->gestation2_pericordialCollections->currentIndex());
    qry.addBindValue(ui->gestation2_planPatruCamere->currentIndex());
    qry.addBindValue(ui->gestation2_planPatruCamere_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_planPatruCamere_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathLeft->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathLeft_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ventricularEjectionPathLeft_description->text());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathRight->currentIndex());
    qry.addBindValue(ui->gestation2_ventricularEjectionPathRight_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ventricularEjectionPathRight_description->text());
    qry.addBindValue(ui->gestation2_intersectionVesselMagistral->currentIndex());
    qry.addBindValue(ui->gestation2_intersectionVesselMagistral_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_intersectionVesselMagistral_description->text());
    qry.addBindValue(ui->gestation2_planTreiVase->currentIndex());
    qry.addBindValue(ui->gestation2_planTreiVase_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_planTreiVase_description->text());
    qry.addBindValue(ui->gestation2_archAorta->currentIndex());
    qry.addBindValue(ui->gestation2_planBicav->currentIndex());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_heart' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // thorax
    qry.prepare(R"(
        UPDATE
            tableGestation2_thorax
        SET
            pulmonaryAreas = ?,
            pulmonaryAreas_description = ?,
            pleuralCollections = ?,
            diaphragm = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_pulmonaryAreas->currentIndex());
    qry.addBindValue(ui->gestation2_pulmonaryAreas_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_pulmonaryAreas_description->text());
    qry.addBindValue(ui->gestation2_pleuralCollections->currentIndex());
    qry.addBindValue(ui->gestation2_diaphragm->currentIndex());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_thorax' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // abdomen
    qry.prepare(R"(
        UPDATE
            tableGestation2_abdomen
        SET
            abdominalWall = ?,
            abdominalCollections = ?,
            stomach = ?,
            stomach_description = ?,
            abdominalOrgans = ?,
            cholecist = ?,
            cholecist_description = ?,
            intestine = ?,
            intestine_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_abdominalWall->currentIndex());
    qry.addBindValue(ui->gestation2_abdominalCollections->currentIndex());
    qry.addBindValue(ui->gestation2_stomach->currentIndex());
    qry.addBindValue(ui->gestation2_stomach_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_stomach_description->text());
    qry.addBindValue(ui->gestation2_abdominalOrgans->currentIndex());
    qry.addBindValue(ui->gestation2_cholecist->currentIndex());
    qry.addBindValue(ui->gestation2_cholecist_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cholecist_description->text());
    qry.addBindValue(ui->gestation2_intestine->currentIndex());
    qry.addBindValue(ui->gestation2_intestine_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_intestine_description->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_abdomen' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // s.urinar
    qry.prepare(R"(
        UPDATE
            tableGestation2_urinarySystem
        SET
            kidneys = ?,
            kidneys_descriptions = ?,
            ureter = ?,
            ureter_descriptions = ?,
            bladder = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_kidneys->currentIndex());
    qry.addBindValue(ui->gestation2_kidneys_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_kidneys_description->text());
    qry.addBindValue(ui->gestation2_ureter->currentIndex());
    qry.addBindValue(ui->gestation2_ureter_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ureter_description->text());
    qry.addBindValue(ui->gestation2_bladder->currentIndex());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_urinarySystem' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // other
    qry.prepare(R"(
        UPDATE
            tableGestation2_other
        SET
            externalGenitalOrgans = ?,
            externalGenitalOrgans_aspect = ?,
            extremities = ?,
            extremities_descriptions = ?,
            fetusMass = ?,
            placenta = ?,
            placentaLocalization = ?,
            placentaDegreeMaturation = ?,
            placentaDepth = ?,
            placentaStructure = ?,
            placentaStructure_descriptions = ?,
            umbilicalCordon = ?,
            umbilicalCordon_description = ?,
            insertionPlacenta = ?,
            amnioticIndex = ?,
            amnioticIndexAspect = ?,
            amnioticBedDepth = ?,
            cervix = ?,
            cervix_description = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_fetusSex->currentIndex());
    qry.addBindValue(QVariant());
    qry.addBindValue(ui->gestation2_extremities->currentIndex());
    qry.addBindValue(ui->gestation2_extremities_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_extremities_description->text());
    qry.addBindValue(ui->gestation2_fetusMass->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_fetusMass->text());
    qry.addBindValue(ui->gestation2_placenta->currentIndex());
    qry.addBindValue(ui->gestation2_placenta_localization->text().isEmpty()
                     ? QVariant()
                     : ui->gestation1_location_placenta->text());
    qry.addBindValue(ui->gestation2_placentaDegreeMaturation->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_placentaDegreeMaturation->text());
    qry.addBindValue(ui->gestation2_placentaDepth->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_placentaDepth->text());
    qry.addBindValue(ui->gestation2_placentaStructure->currentIndex());
    qry.addBindValue(ui->gestation2_placentaStructure_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_placentaStructure_description->text());
    qry.addBindValue(ui->gestation2_umbilicalCordon->currentIndex());
    qry.addBindValue(ui->gestation2_umbilicalCordon_description->text().isEmpty()
            ? QVariant()
            : ui->gestation2_umbilicalCordon_description->text());
    qry.addBindValue(ui->gestation2_insertionPlacenta->currentIndex());
    qry.addBindValue(ui->gestation2_amnioticIndex->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_amnioticIndex->text());
    qry.addBindValue(ui->gestation2_amnioticIndexAspect->currentIndex());
    qry.addBindValue(ui->gestation2_amnioticBedDepth->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_amnioticBedDepth->text());
    qry.addBindValue(ui->gestation2_cervix->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cervix->text());
    qry.addBindValue(ui->gestation2_cervix_description->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cervix_description->text());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_other' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    // doppler
    qry.prepare(R"(
        UPDATE
            tableGestation2_doppler
        SET
            ombilic_PI = ?,
            ombilic_RI = ?,
            ombilic_SD = ?,
            ombilic_flux = ?,
            cerebral_PI = ?,
            cerebral_RI = ?,
            cerebral_SD = ?,
            cerebral_flux = ?,
            uterRight_PI = ?,
            uterRight_RI = ?,
            uterRight_SD = ?,
            uterRight_flux = ?,
            uterLeft_PI = ?,
            uterLeft_RI = ?,
            uterLeft_SD = ?,
            uterLeft_flux = ?,
            ductVenos = ?
        WHERE
            id_reportEcho = ?;
    )");
    qry.addBindValue(ui->gestation2_ombilic_PI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ombilic_PI->text());
    qry.addBindValue(ui->gestation2_ombilic_RI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ombilic_RI->text());
    qry.addBindValue(ui->gestation2_ombilic_SD->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_ombilic_SD->text());
    qry.addBindValue(ui->gestation2_ombilic_flux->currentIndex());
    qry.addBindValue(ui->gestation2_cerebral_PI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cerebral_PI->text());
    qry.addBindValue(ui->gestation2_cerebral_RI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cerebral_RI->text());
    qry.addBindValue(ui->gestation2_cerebral_SD->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_cerebral_SD->text());
    qry.addBindValue(ui->gestation2_cerebral_flux->currentIndex());
    qry.addBindValue(ui->gestation2_uterRight_PI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterRight_PI->text());
    qry.addBindValue(ui->gestation2_uterRight_RI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterRight_RI->text());
    qry.addBindValue(ui->gestation2_uterRight_SD->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterRight_SD->text());
    qry.addBindValue(ui->gestation2_uterRight_flux->currentIndex());
    qry.addBindValue(ui->gestation2_uterLeft_PI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterLeft_PI->text());
    qry.addBindValue(ui->gestation2_uterLeft_RI->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterLeft_RI->text());
    qry.addBindValue(ui->gestation2_uterLeft_SD->text().isEmpty()
                     ? QVariant()
                     : ui->gestation2_uterLeft_SD->text());
    qry.addBindValue(ui->gestation2_uterLeft_flux->currentIndex());
    qry.addBindValue(ui->gestation2_ductVenos->currentIndex());
    qry.addBindValue(m_id);
    if (! qry.exec()) {
        err = tr("Eroarea actualizarii datelor documentului nr.%1 în tabela 'tableGestation2_doppler' - %2")
              .arg(ui->editDocNumber->text(), qry.lastError().text().isEmpty() ? tr("eroare indisponibilă") : qry.lastError().text());
        return false;
    }

    return true;
}

void DocReportEcho::updateDataDocOrderEcho()
{
    if (m_id_docOrderEcho == Enums::IDX::IDX_UNKNOW || m_id_docOrderEcho == 0)
        return;

    QString str;
    if (list_play->count() > 0) {
        str = QString("UPDATE orderEcho SET attachedImages = '%1' WHERE id = '%2';")
                .arg(QString::number(2),
                     QString::number(m_id_docOrderEcho));
    } else {
        str = QString("UPDATE orderEcho SET attachedImages = '%1' WHERE id = '%2';")
                .arg((m_count_images == 0) ? QString::number(m_count_images) : QString::number(1),
                     QString::number(m_id_docOrderEcho));
    }

    if (! db->execQuery(str))
        qCritical(logCritical()) << tr("Document 'Raport ecografic' cu id='%1', nr.='%2' - eroare la instalarea valorii 'attachedImages' in documentul parinte 'Comanda ecografica' cu id='%3'.")
                                        .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_id_docOrderEcho));
}

// *******************************************************************
// **************** EXTRAGEREA DATELOR DIN BD SI SETAREA IN FORMA ****

void DocReportEcho::setDataFromSystemOrgansInternal()
{
    QString str = R"(
        SELECT
            l.id,
            l.id_reportEcho,
            l.[left] AS liver_left_lobe,
            l.[right] AS liver_right_lobe,
            l.contur AS liver_contur,
            l.parenchim AS liver_parenchim,
            l.ecogenity AS liver_ecogenity,
            l.formations AS liver_formations,
            l.ductsIntrahepatic AS liver_ductsIntrahepatic,
            l.porta AS liver_porta,
            l.lienalis AS liver_lienalis,
            l.concluzion AS liver_concluzion,
            l.recommendation AS liver_recommendation,
            c.form AS cholecist_form,
            c.dimens AS cholecist_dimens,
            c.walls AS cholecist_walls,
            c.choledoc AS cholecist_choledoc,
            c.formations AS cholecist_formations,
            p.cefal AS pancreas_cefal,
            p.corp AS pancreas_corp,
            p.tail AS pancreas_tail,
            p.texture AS pancreas_texture,
            p.ecogency AS pancreas_ecogency,
            p.formations AS pancreas_formations,
            s.dimens AS spleen_dimens,
            s.contur AS spleen_contur,
            s.parenchim AS spleen_parenchim,
            s.formations AS spleen_formations,
            i.formations AS intestinal_formation
        FROM
            tableLiver AS l
        LEFT JOIN
            tableCholecist AS c ON l.id_reportEcho = c.id_reportEcho
        LEFT JOIN
            tablePancreas AS p ON l.id_reportEcho = p.id_reportEcho
        LEFT JOIN
            tableSpleen AS s ON l.id_reportEcho = s.id_reportEcho
        LEFT JOIN
            tableIntestinalLoop AS i ON l.id_reportEcho = i.id_reportEcho
        WHERE
            l.id_reportEcho = ?;
    )";

    if (globals().thisMySQL) {
        str = str.replace("[", "`");
        str = str.replace("]", "`");
    }

    QSqlQuery qry;
    qry.prepare(str);
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // liver
            ui->liver_left->setText(qry.value(record.indexOf("liver_left_lobe")).toString());
            ui->liver_right->setText(qry.value(record.indexOf("liver_right_lobe")).toString());
            ui->liver_contour->setText(qry.value(record.indexOf("liver_contur")).toString());
            ui->liver_parenchyma->setText(qry.value(record.indexOf("liver_parenchim")).toString());
            ui->liver_ecogenity->setText(qry.value(record.indexOf("liver_ecogenity")).toString());
            ui->liver_formations->setText(qry.value(record.indexOf("liver_formations")).toString());
            ui->liver_duct_hepatic->setText(qry.value(record.indexOf("liver_ductsIntrahepatic")).toString());
            ui->liver_porta->setText(qry.value(record.indexOf("liver_porta")).toString());
            ui->liver_lienalis->setText(qry.value(record.indexOf("liver_lienalis")).toString());
            ui->organsInternal_concluzion->setPlainText(qry.value(record.indexOf("liver_concluzion")).toString());
            ui->organsInternal_recommendation->setText(qry.value(record.indexOf("liver_recommendation")).toString());
            // cholecist
            ui->cholecist_form->setText(qry.value(record.indexOf("cholecist_form")).toString());
            ui->cholecist_dimens->setText(qry.value(record.indexOf("cholecist_dimens")).toString());
            ui->cholecist_walls->setText(qry.value(record.indexOf("cholecist_walls")).toString());
            ui->cholecist_coledoc->setText(qry.value(record.indexOf("cholecist_choledoc")).toString());
            ui->cholecist_formations->setText(qry.value(record.indexOf("cholecist_formations")).toString());
            // pancreas
            ui->pancreas_cefal->setText(qry.value(record.indexOf("pancreas_cefal")).toString());
            ui->pancreas_corp->setText(qry.value(record.indexOf("pancreas_corp")).toString());
            ui->pancreas_tail->setText(qry.value(record.indexOf("pancreas_tail")).toString());
            ui->pancreas_parenchyma->setText(qry.value(record.indexOf("pancreas_texture")).toString());
            ui->pancreas_ecogenity->setText(qry.value(record.indexOf("pancreas_ecogency")).toString());
            ui->pancreas_formations->setText(qry.value(record.indexOf("pancreas_formations")).toString());
            // spleen
            ui->spleen_contour->setText(qry.value(record.indexOf("spleen_dimens")).toString());
            ui->spleen_dimens->setText(qry.value(record.indexOf("spleen_contur")).toString());
            ui->spleen_parenchyma->setText(qry.value(record.indexOf("spleen_parenchim")).toString());
            ui->spleen_formations->setText(qry.value(record.indexOf("spleen_formations")).toString());
            // intestinal loop
            ui->intestinalHandles->setPlainText(qry.value(record.indexOf("intestinal_formation")).toString());
        }
    }
}

void DocReportEcho::setDataFromSystemUrinary()
{
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            k.id,
            k.id_reportEcho,
            k.contour_right AS kidney_contour_right,
            k.contour_left AS kidney_contour_left,
            k.dimens_right AS kidney_dimens_right,
            k.dimens_left AS kidney_dimens_left,
            k.corticomed_right AS kidney_corticomed_right,
            k.corticomed_left AS kidney_corticomed_left,
            k.pielocaliceal_right AS kidney_pielocaliceal_right,
            k.pielocaliceal_left AS kidney_pielocaliceal_left,
            k.formations AS kidney_formations,
            k.suprarenal_formations AS kidney_suprarenal_formations,
            k.concluzion AS kidney_concluzion,
            k.recommendation AS kidney_recommendation,
            b.volum AS bladder_volum,
            b.walls AS bladder_walls,
            b.formations AS bladder_formations
        FROM
            tableKidney k
        LEFT JOIN
            tableBladder AS b ON k.id_reportEcho = b.id_reportEcho
        WHERE
            k.id_reportEcho = ?;
    )");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // kidney
            ui->kidney_contur_right->setCurrentText(qry.value(record.indexOf("kidney_contour_right")).toString());
            ui->kidney_contur_left->setCurrentText(qry.value(record.indexOf("kidney_contour_left")).toString());
            ui->kidney_right->setText(qry.value(record.indexOf("kidney_dimens_right")).toString());
            ui->kidney_left->setText(qry.value(record.indexOf("kidney_dimens_left")).toString());
            ui->kidney_corticomed_right->setText(qry.value(record.indexOf("kidney_corticomed_right")).toString());
            ui->kidney_corticomed_left->setText(qry.value(record.indexOf("kidney_corticomed_left")).toString());
            ui->kidney_pielocaliceal_right->setText(qry.value(record.indexOf("kidney_pielocaliceal_right")).toString());
            ui->kidney_pielocaliceal_left->setText(qry.value(record.indexOf("kidney_pielocaliceal_left")).toString());
            ui->kidney_formations->setText(qry.value(record.indexOf("kidney_formations")).toString());
            ui->adrenalGlands->setPlainText(qry.value(record.indexOf("kidney_suprarenal_formations")).toString());
            ui->urinary_system_concluzion->setPlainText(qry.value(record.indexOf("kidney_concluzion")).toString());
            ui->urinary_system_recommendation->setText(qry.value(record.indexOf("kidney_recommendation")).toString());
            // bladder
            ui->bladder_volum->setText(qry.value(record.indexOf("bladder_volum")).toString());
            ui->bladder_walls->setText(qry.value(record.indexOf("bladder_walls")).toString());
            ui->bladder_formations->setText(qry.value(record.indexOf("bladder_formations")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableProstate()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableProstate WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->prostate_radioBtn_transrectal->setChecked(qry.value(record.indexOf("transrectal")).toInt());
            ui->prostate_dimens->setText(qry.value(record.indexOf("dimens")).toString());
            ui->prostate_volum->setText(qry.value(record.indexOf("volume")).toString());
            ui->prostate_contur->setText(qry.value(record.indexOf("contour")).toString());
            ui->prostate_ecostructure->setText(qry.value(record.indexOf("ecostructure")).toString());
            ui->prostate_ecogency->setText(qry.value(record.indexOf("ecogency")).toString());
            ui->prostate_formations->setText(qry.value(record.indexOf("formations")).toString());
            ui->prostate_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->prostate_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableGynecology()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGynecology WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            bool transvaginal_checked = qry.value(record.indexOf("transvaginal")).toBool();
            ui->gynecology_btn_transvaginal->setChecked(transvaginal_checked);
            ui->gynecology_btn_transabdom->setChecked(!transvaginal_checked);

            ui->gynecology_dateMenstruation->setDate(QDate::fromString(qry.value(record.indexOf("dateMenstruation")).toString(), "yyyy-MM-dd"));
            ui->gynecology_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            ui->gynecology_uterus_dimens->setText(qry.value(record.indexOf("uterus_dimens")).toString());
            ui->gynecology_uterus_pozition->setText(qry.value(record.indexOf("uterus_pozition")).toString());
            ui->gynecology_uterus_ecostructure->setText(qry.value(record.indexOf("uterus_ecostructure")).toString());
            ui->gynecology_uterus_formations->setPlainText(qry.value(record.indexOf("uterus_formations")).toString());
            ui->gynecology_combo_jonctional_zone->setCurrentText(qry.value(record.indexOf("junctional_zone")).toString());
            ui->gynecology_jonctional_zone_description->setText(qry.value(record.indexOf("junctional_zone_description")).toString());
            ui->gynecology_ecou_dimens->setText(qry.value(record.indexOf("ecou_dimens")).toString());
            ui->gynecology_ecou_ecostructure->setText(qry.value(record.indexOf("ecou_ecostructure")).toString());
            ui->gynecology_cervix_dimens->setText(qry.value(record.indexOf("cervix_dimens")).toString());
            ui->gynecology_cervix_ecostructure->setText(qry.value(record.indexOf("cervix_ecostructure")).toString());
            ui->gynecology_combo_canal_cervical->setCurrentText(qry.value(record.indexOf("cervical_canal")).toString());
            ui->gynecology_canal_cervical_formations->setText(qry.value(record.indexOf("cervical_canal_formations")).toString());
            ui->gynecology_douglas->setText(qry.value(record.indexOf("douglas")).toString());
            ui->gynecology_plex_venos->setText(qry.value(record.indexOf("plex_venos")).toString());
            ui->gynecology_ovary_right_dimens->setText(qry.value(record.indexOf("ovary_right_dimens")).toString());
            ui->gynecology_ovary_left_dimens->setText(qry.value(record.indexOf("ovary_left_dimens")).toString());
            ui->gynecology_ovary_right_volum->setText(qry.value(record.indexOf("ovary_right_volum")).toString());
            ui->gynecology_ovary_left_volum->setText(qry.value(record.indexOf("ovary_left_volum")).toString());
            ui->gynecology_follicule_right->setText(qry.value(record.indexOf("ovary_right_follicle")).toString());
            ui->gynecology_follicule_left->setText(qry.value(record.indexOf("ovary_left_follicle")).toString());
            ui->gynecology_ovary_formations_right->setPlainText(qry.value(record.indexOf("ovary_right_formations")).toString());
            ui->gynecology_ovary_formations_left->setPlainText(qry.value(record.indexOf("ovary_left_formations")).toString());
            ui->gynecology_combo_fallopian_tubes->setCurrentText(qry.value(record.indexOf("fallopian_tubes")).toString());
            ui->gynecology_fallopian_tubes_formations->setText(qry.value(record.indexOf("fallopian_tubes_formations")).toString());
            ui->gynecology_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gynecology_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableBreast()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableBreast WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->breast_right_ecostructure->setText(qry.value(record.indexOf("breast_right_ecostrcture")).toString());
            ui->breast_right_duct->setText(qry.value(record.indexOf("breast_right_duct")).toString());
            ui->breast_right_ligament->setText(qry.value(record.indexOf("breast_right_ligament")).toString());
            ui->breast_right_formations->setPlainText(qry.value(record.indexOf("breast_right_formations")).toString());
            ui->breast_right_ganglions->setText(qry.value(record.indexOf("breast_right_ganglions")).toString());
            ui->breast_left_ecostructure->setText(qry.value(record.indexOf("breast_left_ecostrcture")).toString());
            ui->breast_left_duct->setText(qry.value(record.indexOf("breast_left_duct")).toString());
            ui->breast_left_ligament->setText(qry.value(record.indexOf("breast_left_ligament")).toString());
            ui->breast_left_formations->setPlainText(qry.value(record.indexOf("breast_left_formations")).toString());
            ui->breast_left_ganglions->setText(qry.value(record.indexOf("breast_left_ganglions")).toString());
            ui->breast_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableThyroid()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableThyroid WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->thyroid_right_dimens->setText(qry.value(record.indexOf("thyroid_right_dimens")).toString());
            ui->thyroid_right_volum->setText(qry.value(record.indexOf("thyroid_right_volum")).toString());
            ui->thyroid_left_dimens->setText(qry.value(record.indexOf("thyroid_left_dimens")).toString());
            ui->thyroid_left_volum->setText(qry.value(record.indexOf("thyroid_left_volum")).toString());
            ui->thyroid_istm->setText(qry.value(record.indexOf("thyroid_istm")).toString());
            ui->thyroid_ecostructure->setText(qry.value(record.indexOf("thyroid_ecostructure")).toString());
            ui->thyroid_formations->setPlainText(qry.value(record.indexOf("thyroid_formations")).toString());
            ui->thyroid_ganglions->setText(qry.value(record.indexOf("thyroid_ganglions")).toString());
            ui->thyroid_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->thyroid_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableGestation0()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGestation0 WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->gestation0_view_good->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 0);
            ui->gestation0_view_medium->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 1);
            ui->gestation0_view_difficult->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 2);

            ui->gestation0_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            if (db->existColumnInTable("tableGestation0", "lmp")){
                ui->gestation0_LMP->setDate(QDate::fromString(qry.value(record.indexOf("lmp")).toString(), "yyyy-MM-dd"));
                calculateGestationalAge(ui->gestation0_LMP->date());
                calculateDueDate(ui->gestation0_LMP->date());
            }
            ui->gestation0_gestation->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation0_GS_dimens->setText(qry.value(record.indexOf("GS")).toString());
            ui->gestation0_GS_age->setText(qry.value(record.indexOf("GS_age")).toString());
            ui->gestation0_CRL_dimens->setText(qry.value(record.indexOf("CRL")).toString());
            ui->gestation0_CRL_age->setText(qry.value(record.indexOf("CRL_age")).toString());
            ui->gestation0_BCF->setText(qry.value(record.indexOf("BCF")).toString());
            ui->gestation0_liquid_amniotic->setText(qry.value(record.indexOf("liquid_amniotic")).toString());
            ui->gestation0_miometer->setText(qry.value(record.indexOf("miometer")).toString());
            ui->gestation0_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation0_ovary->setText(qry.value(record.indexOf("ovary")).toString());
            ui->gestation0_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation0_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableGestation1()
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM tableGestation1 WHERE id_reportEcho = ?;");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            ui->gestation1_view_good->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 0);
            ui->gestation1_view_medium->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 1);
            ui->gestation1_view_difficult->setChecked(qry.value(record.indexOf("view_examination")).toInt() == 2);

            ui->gestation1_antecedent->setText(qry.value(record.indexOf("antecedent")).toString());
            if (db->existColumnInTable("gestation1", "lmp")) {
                ui->gestation1_LMP->setDate(QDate::fromString(qry.value(record.indexOf("lmp")).toString(), "yyyy-MM-dd"));
                calculateGestationalAge(ui->gestation1_LMP->date());
                calculateDueDate(ui->gestation1_LMP->date());
            }
            ui->gestation1_gestation->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation1_CRL_dimens->setText(qry.value(record.indexOf("CRL")).toString());
            ui->gestation1_CRL_age->setText(qry.value(record.indexOf("CRL_age")).toString());
            ui->gestation1_BPD_dimens->setText(qry.value(record.indexOf("BPD")).toString());
            ui->gestation1_BPD_age->setText(qry.value(record.indexOf("BPD_age")).toString());
            ui->gestation1_NT_dimens->setText(qry.value(record.indexOf("NT")).toString());
            ui->gestation1_NT_percent->setText(qry.value(record.indexOf("NT_percent")).toString());
            ui->gestation1_BN_dimens->setText(qry.value(record.indexOf("BN")).toString());
            ui->gestation1_BN_percent->setText(qry.value(record.indexOf("BN_percent")).toString());
            ui->gestation1_BCF->setText(qry.value(record.indexOf("BCF")).toString());
            ui->gestation1_FL_dimens->setText(qry.value(record.indexOf("FL")).toString());
            ui->gestation1_FL_age->setText(qry.value(record.indexOf("FL_age")).toString());
            ui->gestation1_callote_cranium->setText(qry.value(record.indexOf("callote_cranium")).toString());
            ui->gestation1_plex_choroid->setText(qry.value(record.indexOf("plex_choroid")).toString());
            ui->gestation1_vertebral_column->setText(qry.value(record.indexOf("vertebral_column")).toString());
            ui->gestation1_stomach->setText(qry.value(record.indexOf("stomach")).toString());
            ui->gestation1_bladder->setText(qry.value(record.indexOf("bladder")).toString());
            ui->gestation1_diaphragm->setText(qry.value(record.indexOf("diaphragm")).toString());
            ui->gestation1_abdominal_wall->setText(qry.value(record.indexOf("abdominal_wall")).toString());
            ui->gestation1_location_placenta->setText(qry.value(record.indexOf("location_placenta")).toString());
            ui->gestation1_sac_vitelin->setText(qry.value(record.indexOf("sac_vitelin")).toString());
            ui->gestation1_amniotic_liquid->setText(qry.value(record.indexOf("amniotic_liquid")).toString());
            ui->gestation1_miometer->setText(qry.value(record.indexOf("miometer")).toString());
            ui->gestation1_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation1_ovary->setText(qry.value(record.indexOf("ovary")).toString());
            ui->gestation1_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation1_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());
        }
    }
}

void DocReportEcho::setDataFromTableGestation2()
{
    QSqlQuery qry;
    qry.prepare(R"(
        SELECT
            ges.id AS ges_id,
            ges.id_reportEcho,
            ges.gestation_age,
            ges.trimestru,
            ges.dateMenstruation,
            ges.view_examination,
            ges.single_multiple_pregnancy,
            ges.single_multiple_pregnancy_description,
            ges.antecedent,
            ges.comment,
            ges.concluzion,
            ges.recommendation,
            bio.BPD,
            bio.BPD_age,
            bio.HC,
            bio.HC_age,
            bio.AC,
            bio.AC_age,
            bio.FL,
            bio.FL_age,
            bio.FetusCorresponds,
            cr.calloteCranium,
            cr.facialeProfile,
            cr.nasalBones,
            cr.nasalBones_dimens,
            cr.eyeball,
            cr.eyeball_desciption,
            cr.nasolabialTriangle,
            cr.nasolabialTriangle_description,
            cr.nasalFold,
            snc.hemispheres,
            snc.fissureSilvius,
            snc.corpCalos,
            snc.ventricularSystem,
            snc.ventricularSystem_description,
            snc.cavityPellucidSeptum,
            snc.choroidalPlex,
            snc.choroidalPlex_description,
            snc.cerebellum,
            snc.cerebellum_description,
            snc.vertebralColumn,
            snc.vertebralColumn_description,
            hrt.`position`,
            hrt.heartBeat,
            hrt.heartBeat_frequency,
            hrt.heartBeat_rhythm,
            hrt.pericordialCollections,
            hrt.planPatruCamere,
            hrt.planPatruCamere_description,
            hrt.ventricularEjectionPathLeft,
            hrt.ventricularEjectionPathLeft_description,
            hrt.ventricularEjectionPathRight,
            hrt.ventricularEjectionPathRight_description,
            hrt.intersectionVesselMagistral,
            hrt.intersectionVesselMagistral_description,
            hrt.planTreiVase,
            hrt.planTreiVase_description,
            hrt.archAorta,
            hrt.planBicav,
            th.pulmonaryAreas,
            th.pulmonaryAreas_description,
            th.pleuralCollections,
            th.diaphragm,
            abd.abdominalWall,
            abd.abdominalCollections,
            abd.stomach,
            abd.stomach_description,
            abd.abdominalOrgans,
            abd.cholecist,
            abd.cholecist_description,
            abd.intestine,
            abd.intestine_description,
            us.kidneys,
            us.kidneys_descriptions,
            us.ureter,
            us.ureter_descriptions,
            us.bladder,
            oth.externalGenitalOrgans,
            oth.externalGenitalOrgans_aspect,
            oth.extremities,
            oth.extremities_descriptions,
            oth.fetusMass,
            oth.placenta,
            oth.placentaLocalization,
            oth.placentaDegreeMaturation,
            oth.placentaDepth,
            oth.placentaStructure,
            oth.placentaStructure_descriptions,
            oth.umbilicalCordon,
            oth.umbilicalCordon_description,
            oth.insertionPlacenta,
            oth.amnioticIndex,
            oth.amnioticIndexAspect,
            oth.amnioticBedDepth,
            oth.cervix,
            oth.cervix_description,
            dop.ombilic_PI,
            dop.ombilic_RI,
            dop.ombilic_SD,
            dop.ombilic_flux,
            dop.cerebral_PI,
            dop.cerebral_RI,
            dop.cerebral_SD,
            dop.cerebral_flux,
            dop.uterRight_PI,
            dop.uterRight_RI,
            dop.uterRight_SD,
            dop.uterRight_flux,
            dop.uterLeft_PI,
            dop.uterLeft_RI,
            dop.uterLeft_SD,
            dop.uterLeft_flux,
            dop.ductVenos
        FROM
            tableGestation2 AS ges
        LEFT JOIN
            tableGestation2_biometry AS bio ON bio.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_cranium AS cr ON cr.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_SNC AS snc ON snc.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_heart AS hrt ON hrt.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_thorax AS th ON th.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_abdomen AS abd ON abd.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_urinarySystem AS us ON us.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_other AS oth ON oth.id_reportEcho = ges.id_reportEcho
        LEFT JOIN
            tableGestation2_doppler AS dop ON dop.id_reportEcho = ges.id_reportEcho
        WHERE
            ges.id_reportEcho = ?;
    )");
    qry.addBindValue(m_id);
    if (qry.exec()) {
        while (qry.next()) {
            QSqlRecord record = qry.record();
            // tableGestation2
            ui->gestation2_dateMenstruation->setDate(QDate::fromString(qry.value(record.indexOf("dateMenstruation")).toString(), "yyyy-MM-dd"));
            ui->gestation2_gestation_age->setText(qry.value(record.indexOf("gestation_age")).toString());
            ui->gestation2_view_examination->setCurrentIndex(qry.value(record.indexOf("view_examination")).toInt());
            if (qry.value(record.indexOf("trimestru")).toInt() == 2){
                ui->gestation2_trimestru2->setChecked(true);
                clickedGestation2Trimestru(0);
            } else {
                ui->gestation2_trimestru3->setChecked(true);
                clickedGestation2Trimestru(1);
            }
            ui->gestation2_pregnancy->setCurrentIndex(qry.value(record.indexOf("single_multiple_pregnancy")).toInt());
            ui->gestation2_pregnancy_description->setText(qry.value(record.indexOf("single_multiple_pregnancy_description")).toString());
            ui->gestation2_comment->setPlainText(qry.value(record.indexOf("comment")).toString());
            ui->gestation2_concluzion->setPlainText(qry.value(record.indexOf("concluzion")).toString());
            ui->gestation2_recommendation->setText(qry.value(record.indexOf("recommendation")).toString());

            // tableGestation2_biometry
            ui->gestation2_bpd->setText(qry.value(record.indexOf("BPD")).toString());
            ui->gestation2_bpd_age->setText(qry.value(record.indexOf("BPD_age")).toString());
            ui->gestation2_hc->setText(qry.value(record.indexOf("HC")).toString());
            ui->gestation2_hc_age->setText(qry.value(record.indexOf("HC_age")).toString());
            ui->gestation2_ac->setText(qry.value(record.indexOf("AC")).toString());
            ui->gestation2_ac_age->setText(qry.value(record.indexOf("AC_age")).toString());
            ui->gestation2_fl->setText(qry.value(record.indexOf("FL")).toString());
            ui->gestation2_fl_age->setText(qry.value(record.indexOf("FL_age")).toString());
            ui->gestation2_fetus_age->setText(qry.value(record.indexOf("FetusCorresponds")).toString());
            setDueDateGestation2();

            // tableGestation2_cranium
            ui->gestation2_calloteCranium->setCurrentIndex(qry.value(record.indexOf("calloteCranium")).toInt());
            ui->gestation2_facialeProfile->setCurrentIndex(qry.value(record.indexOf("facialeProfile")).toInt());
            ui->gestation2_nasalBones->setCurrentIndex(qry.value(record.indexOf("nasalBones")).toInt());
            ui->gestation2_nasalBones_dimens->setText(qry.value(record.indexOf("nasalBones_dimens")).toString());
            ui->gestation2_eyeball->setCurrentIndex(qry.value(record.indexOf("eyeball")).toInt());
            ui->gestation2_eyeball_desciption->setText(qry.value(record.indexOf("eyeball_desciption")).toString());
            ui->gestation2_nasolabialTriangle->setCurrentIndex(qry.value(record.indexOf("nasolabialTriangle")).toInt());
            ui->gestation2_nasolabialTriangle_description->setText(qry.value(record.indexOf("nasolabialTriangle_description")).toString());
            ui->gestation2_nasalFold->setText(qry.value(record.indexOf("nasalFold")).toString());

            // tableGestation2_SNC
            ui->gestation2_hemispheres->setCurrentIndex(qry.value(record.indexOf("hemispheres")).toInt());
            ui->gestation2_fissureSilvius->setCurrentIndex(qry.value(record.indexOf("fissureSilvius")).toInt());
            ui->gestation2_corpCalos->setCurrentIndex(qry.value(record.indexOf("corpCalos")).toInt());
            ui->gestation2_ventricularSystem->setCurrentIndex(qry.value(record.indexOf("ventricularSystem")).toInt());
            ui->gestation2_ventricularSystem_description->setText(qry.value(record.indexOf("ventricularSystem_description")).toString());
            ui->gestation2_cavityPellucidSeptum->setCurrentIndex(qry.value(record.indexOf("cavityPellucidSeptum")).toInt());
            ui->gestation2_choroidalPlex->setCurrentIndex(qry.value(record.indexOf("choroidalPlex")).toInt());
            ui->gestation2_choroidalPlex_description->setText(qry.value(record.indexOf("choroidalPlex_description")).toString());
            ui->gestation2_cerebellum->setCurrentIndex(qry.value(record.indexOf("cerebellum")).toInt());
            ui->gestation2_cerebellum_description->setText(qry.value(record.indexOf("cerebellum_description")).toString());
            ui->gestation2_vertebralColumn->setCurrentIndex(qry.value(record.indexOf("vertebralColumn")).toInt());
            ui->gestation2_vertebralColumn_description->setText(qry.value(record.indexOf("vertebralColumn_description")).toString());

            // tableGestation2_heart
            ui->gestation2_heartPosition->setText(qry.value(record.indexOf("position")).toString());
            ui->gestation2_heartBeat->setCurrentIndex(qry.value(record.indexOf("heartBeat")).toInt());
            ui->gestation2_heartBeat_frequency->setText(qry.value(record.indexOf("heartBeat_frequency")).toString());
            ui->gestation2_heartBeat_rhythm->setCurrentIndex(qry.value(record.indexOf("heartBeat_rhythm")).toInt());
            ui->gestation2_pericordialCollections->setCurrentIndex(qry.value(record.indexOf("pericordialCollections")).toInt());
            ui->gestation2_planPatruCamere->setCurrentIndex(qry.value(record.indexOf("planPatruCamere")).toInt());
            ui->gestation2_planPatruCamere_description->setText(qry.value(record.indexOf("planPatruCamere_description")).toString());
            ui->gestation2_ventricularEjectionPathLeft->setCurrentIndex(qry.value(record.indexOf("ventricularEjectionPathLeft")).toInt());
            ui->gestation2_ventricularEjectionPathLeft_description->setText(qry.value(record.indexOf("ventricularEjectionPathLeft_description")).toString());
            ui->gestation2_ventricularEjectionPathRight->setCurrentIndex(qry.value(record.indexOf("ventricularEjectionPathRight")).toInt());
            ui->gestation2_ventricularEjectionPathRight_description->setText(qry.value(record.indexOf("ventricularEjectionPathRight_description")).toString());
            ui->gestation2_intersectionVesselMagistral->setCurrentIndex(qry.value(record.indexOf("intersectionVesselMagistral")).toInt());
            ui->gestation2_intersectionVesselMagistral_description->setText(qry.value(record.indexOf("intersectionVesselMagistral_description")).toString());
            ui->gestation2_planTreiVase->setCurrentIndex(qry.value(record.indexOf("planTreiVase")).toInt());
            ui->gestation2_planTreiVase_description->setText(qry.value(record.indexOf("planTreiVase_description")).toString());
            ui->gestation2_archAorta->setCurrentIndex(qry.value(record.indexOf("archAorta")).toInt());
            ui->gestation2_planBicav->setCurrentIndex(qry.value(record.indexOf("planBicav")).toInt());

            // tableGestation2_thorax
            ui->gestation2_pulmonaryAreas->setCurrentIndex(qry.value(record.indexOf("pulmonaryAreas")).toInt());
            ui->gestation2_pulmonaryAreas_description->setText(qry.value(record.indexOf("pulmonaryAreas_description")).toString());
            ui->gestation2_pleuralCollections->setCurrentIndex(qry.value(record.indexOf("pleuralCollections")).toInt());
            ui->gestation2_diaphragm->setCurrentIndex(qry.value(record.indexOf("diaphragm")).toInt());

            // tableGestation2_abdomen
            ui->gestation2_abdominalWall->setCurrentIndex(qry.value(record.indexOf("abdominalWall")).toInt());
            ui->gestation2_abdominalCollections->setCurrentIndex(qry.value(record.indexOf("abdominalCollections")).toInt());
            ui->gestation2_stomach->setCurrentIndex(qry.value(record.indexOf("stomach")).toInt());
            ui->gestation2_stomach_description->setText(qry.value(record.indexOf("stomach_description")).toString());
            ui->gestation2_abdominalOrgans->setCurrentIndex(qry.value(record.indexOf("abdominalOrgans")).toInt());
            ui->gestation2_cholecist->setCurrentIndex(qry.value(record.indexOf("cholecist")).toInt());
            ui->gestation2_cholecist_description->setText(qry.value(record.indexOf("cholecist_description")).toString());
            ui->gestation2_intestine->setCurrentIndex(qry.value(record.indexOf("intestine")).toInt());
            ui->gestation2_intestine_description->setText(qry.value(record.indexOf("intestine_description")).toString());

            // tableGestation2_urinarySystem
            ui->gestation2_kidneys->setCurrentIndex(qry.value(record.indexOf("kidneys")).toInt());
            ui->gestation2_kidneys_description->setText(qry.value(record.indexOf("kidneys_descriptions")).toString());
            ui->gestation2_ureter->setCurrentIndex(qry.value(record.indexOf("ureter")).toInt());
            ui->gestation2_ureter_description->setText(qry.value(record.indexOf("ureter_descriptions")).toString());
            ui->gestation2_bladder->setCurrentIndex(qry.value(record.indexOf("bladder")).toInt());

            // tableGestation2_other
            ui->gestation2_extremities->setCurrentIndex(qry.value(record.indexOf("extremities")).toInt());
            ui->gestation2_extremities_description->setText(qry.value(record.indexOf("extremities_descriptions")).toString());
            ui->gestation2_placenta->setCurrentIndex(qry.value(record.indexOf("placenta")).toInt());
            ui->gestation2_placenta_localization->setText(qry.value(record.indexOf("placentaLocalization")).toString());
            ui->gestation2_placentaDegreeMaturation->setText(qry.value(record.indexOf("placentaDegreeMaturation")).toString());
            ui->gestation2_placentaDepth->setText(qry.value(record.indexOf("placentaDepth")).toString());
            ui->gestation2_placentaStructure->setCurrentIndex(qry.value(record.indexOf("placentaStructure")).toInt());
            ui->gestation2_placentaStructure_description->setText(qry.value(record.indexOf("placentaStructure_descriptions")).toString());
            ui->gestation2_umbilicalCordon->setCurrentIndex(qry.value(record.indexOf("umbilicalCordon")).toInt());
            ui->gestation2_umbilicalCordon_description->setText(qry.value(record.indexOf("umbilicalCordon_description")).toString());
            ui->gestation2_insertionPlacenta->setCurrentIndex(qry.value(record.indexOf("insertionPlacenta")).toInt());
            ui->gestation2_amnioticIndex->setText(qry.value(record.indexOf("amnioticIndex")).toString());
            ui->gestation2_amnioticIndexAspect->setCurrentIndex(qry.value(record.indexOf("amnioticIndexAspect")).toInt());
            ui->gestation2_amnioticBedDepth->setText(qry.value(record.indexOf("amnioticBedDepth")).toString());
            ui->gestation2_cervix->setText(qry.value(record.indexOf("cervix")).toString());
            ui->gestation2_cervix_description->setText(qry.value(record.indexOf("cervix_description")).toString());
            ui->gestation2_fetusMass->setText(qry.value(record.indexOf("fetusMass")).toString());
            ui->gestation2_fetusSex->setCurrentIndex(qry.value(record.indexOf("externalGenitalOrgans")).toInt());
            updateDescriptionFetusWeight();

            // tableGestation2_doppler
            ui->gestation2_ombilic_PI->setText(qry.value(record.indexOf("ombilic_PI")).toString());
            ui->gestation2_ombilic_RI->setText(qry.value(record.indexOf("ombilic_RI")).toString());
            ui->gestation2_ombilic_SD->setText(qry.value(record.indexOf("ombilic_SD")).toString());
            ui->gestation2_ombilic_flux->setCurrentIndex(qry.value(record.indexOf("ombilic_flux")).toInt());
            ui->gestation2_cerebral_PI->setText(qry.value(record.indexOf("cerebral_PI")).toString());
            ui->gestation2_cerebral_RI->setText(qry.value(record.indexOf("cerebral_RI")).toString());
            ui->gestation2_cerebral_SD->setText(qry.value(record.indexOf("cerebral_SD")).toString());
            ui->gestation2_cerebral_flux->setCurrentIndex(qry.value(record.indexOf("cerebral_flux")).toInt());
            ui->gestation2_uterRight_PI->setText(qry.value(record.indexOf("uterRight_PI")).toString());
            ui->gestation2_uterRight_RI->setText(qry.value(record.indexOf("uterRight_RI")).toString());
            ui->gestation2_uterRight_SD->setText(qry.value(record.indexOf("uterRight_SD")).toString());
            ui->gestation2_uterRight_flux->setCurrentIndex(qry.value(record.indexOf("uterRight_flux")).toInt());
            ui->gestation2_uterLeft_PI->setText(qry.value(record.indexOf("uterLeft_PI")).toString());
            ui->gestation2_uterLeft_RI->setText(qry.value(record.indexOf("uterLeft_RI")).toString());
            ui->gestation2_uterLeft_SD->setText(qry.value(record.indexOf("uterLeft_SD")).toString());
            ui->gestation2_uterLeft_flux->setCurrentIndex(qry.value(record.indexOf("uterLeft_flux")).toInt());
            ui->gestation2_ductVenos->setCurrentIndex(qry.value(record.indexOf("ductVenos")).toInt());
            updateTextDescriptionDoppler();
        }
    }
}

DatabaseProvider *DocReportEcho::dbProvider()
{
    return &m_dbProvider;
}

void DocReportEcho::initSyncDocs()
{
    // 1. pregatim date
    DatesDocsOrderReportSync data;
    data.thisMySQL     = globals().thisMySQL;
    data.dateTime_doc  = ui->editDocDate->dateTime(); // report
    data.id_order      = m_id_docOrderEcho;
    data.id_report     = m_id;
    data.nr_report     = ui->editDocNumber->text();
    data.id_patient    = m_idPacient;

    // 2. Creăm thread-ul pentru trimiterea
    QThread *thread = new QThread();

    // 3. alocam memoria worker-lui si mutam in flux nou
    auto worker = new docSyncWorker(dbProvider(), data);
    worker->moveToThread(thread);

    // 4. conectarea - lansarea procesului de syncronizare
    connect(thread, &QThread::started, worker, &docSyncWorker::process);

    // 5. conectarea - procesarea finisarii syncronizarii

    // 6. conectarea - distrugea workerului
    connect(worker, &docSyncWorker::finished, thread, &QThread::quit);
    connect(worker, &docSyncWorker::finished, worker, &docSyncWorker::deleteLater);

    // 7. distrugerea thread-lui
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // 8. lansarea thread-lui
    thread->start();

}

// *******************************************************************
// **************** EVENIMENTE FORMEI ********************************

void DocReportEcho::closeEvent(QCloseEvent *event)
{
    player->stop();

    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            onWritingDataClose();
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

bool DocReportEcho::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Verificăm dacă tasta apăsată este Enter
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QWidget *currentWidget = QApplication::focusWidget();
            if (!currentWidget)
                return false; // Dacă nu există widget activ, continuăm propagarea

            // Dacă este apăsat Ctrl+Enter sau Shift+Enter, permite introducerea de rând nou
            if (keyEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
                return false; // Nu preluăm evenimentul, îl lăsăm să ajungă la QPlainTextEdit
            }

            // Logica similară din keyPressEvent
            if (ui->stackedWidget->currentIndex() == page_organs_internal) {
                if (currentWidget == ui->cholecist_formations)
                    ui->scrollArea_organs_internal->ensureWidgetVisible(ui->spleen_formations);
                else if (currentWidget == ui->spleen_dimens)
                    ui->scrollArea_organs_internal->ensureWidgetVisible(ui->organsInternal_recommendation);
            }

            if (ui->stackedWidget->currentIndex() == page_urinary_system) {
                if (currentWidget == ui->adrenalGlands) {
                    ui->scrollArea_urinary_system->ensureWidgetVisible(ui->urinary_system_recommendation);
                }
            }

            if (ui->stackedWidget->currentIndex() == page_breast) {
                if (currentWidget == ui->breast_left_formations) {
                    ui->scrollArea_breast->ensureWidgetVisible(ui->breast_recommendation);
                }
            }

            if (ui->stackedWidget->currentIndex() == page_gynecology) {
                if (currentWidget == ui->gynecology_ecou_dimens) {
                    ui->scrollArea_gynecology->ensureWidgetVisible(ui->gynecology_ovary_formations_right);
                } else if (currentWidget == ui->gynecology_ovary_formations_right)
                    ui->scrollArea_gynecology->ensureWidgetVisible(ui->gynecology_recommendation);
            }

            if (ui->stackedWidget->currentIndex() == page_gestation0) {
                if (currentWidget == ui->gestation0_miometer) {
                    ui->scrollArea_gestation0->ensureWidgetVisible(ui->gestation0_recommendation);
                }
            }

            if (ui->stackedWidget->currentIndex() == page_gestation1) {
                if (currentWidget == ui->gestation1_abdominal_wall) {
                    ui->scrollArea_gestation1->ensureWidgetVisible(ui->gestation1_recommendation);
                }
            }

            // Trecem la următorul widget în lanțul de focus
            focusNextChild();
            return true; // Marchează evenimentul ca procesat
        }

        return QObject::eventFilter(obj, event); // Continuăm cu filtrarea normală
    }

    if (obj == ui->btnParameters){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnParameters->pos().x() - 20, ui->btnParameters->pos().y() + 30)); // determinam parametrii globali
            popUp->setPopupText(tr("Parametrii documentului.")); // setam textul
            popUp->showFromGeometryTimer(p);                     // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnHistory){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnParameters->pos().x() + 810, ui->btnParameters->pos().y() + 52)); // determinam parametrii globali
            popUp->setPopupText(tr("Deschide<br>\"Istoria pacientului\".")); // setam textul
            popUp->showFromGeometryTimer(p);                     // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnOpenCatPatient){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnParameters->pos().x() + 830, ui->btnParameters->pos().y() + 52)); // determinam parametrii globali
            popUp->setPopupText(tr("Redactarea datelor<br>pacientului.")); // setam textul
            popUp->showFromGeometryTimer(p);                               // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    } else if (obj == ui->btnOpenDocErderEcho){
        if (event->type() == QEvent::Enter){
            QPoint p = mapToGlobal(QPoint(ui->btnParameters->pos().x() + 1160, ui->btnParameters->pos().y() + 52)); // determinam parametrii globali
            popUp->setPopupText(tr("Deschide documentul<br>\"Comanda ecografic\304\203\".")); // setam textul
            popUp->showFromGeometryTimer(p);                     // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);

}

void DocReportEcho::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
//        setTitleDoc();
//        updateHeaderTableSource();
//        updateHeaderTableOrder();
    }
}

void DocReportEcho::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();

        if (ui->stackedWidget->currentIndex() == page_organs_internal){
            if (ui->spleen_parenchyma->hasFocus())
                ui->scrollArea_organs_internal->ensureWidgetVisible(ui->organsInternal_concluzion);
        }

        if (ui->stackedWidget->currentIndex() == page_urinary_system) {
            if (ui->adrenalGlands->hasFocus()) {
                ui->scrollArea_organs_internal->ensureWidgetVisible(ui->urinary_system_concluzion);
            }
        }

        if (ui->stackedWidget->currentIndex() == page_breast) {
            if (ui->breast_left_formations->hasFocus()) {
                ui->scrollArea_breast->ensureWidgetVisible(ui->breast_concluzion);
            }
        }

        if (ui->stackedWidget->currentIndex() == page_gynecology){
            if (ui->gynecology_ovary_right_dimens->hasFocus())
                ui->scrollArea_gynecology->ensureWidgetVisible(ui->gynecology_recommendation);
        }

        if (ui->stackedWidget->currentIndex() == page_gestation0){
            if (ui->gestation0_miometer->hasFocus())
                ui->scrollArea_gestation0->ensureWidgetVisible(ui->gestation0_recommendation);
        }

        if (ui->stackedWidget->currentIndex() == page_gestation1){
            if (ui->gestation1_location_placenta->hasFocus())
                ui->scrollArea_gestation1->ensureWidgetVisible(ui->gestation1_recommendation);
        }
    }
}
