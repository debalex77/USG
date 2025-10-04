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
#include "docreportechohandler.h"
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
    ui(new Ui::DocReportEcho),
    m_handler(std::make_unique<DocReportEchoHandler>(*this)),
    m_sectionsSystem(0)
{
    ui->setupUi(this);

    setWindowTitle(tr("Raport ecografic %1").arg("[*]")); /** setam titlul */
    initInstallEventFilter();     /** Initiem instalarea filtrului de evenimente */
    initRequiredStructure();      /** initierea/setarea structurelor necesare */

    if (globals().showDocumentsInSeparatWindow) /** fereastra in dialog aparte */
        setWindowFlags(Qt::Window);

    /** alocam memoria */
    db              = new DataBase(this);                 /** clasa cu functii universale */
    popUp           = new PopUp(this);                    /** pu mesaje */
    timer           = new QTimer(this);                   /** timer pu vizualizarea secundelor in data */
    completer       = new QCompleter(this);               /** completer pu alegerea pacientilor din comboPatient */
    modelPatients   = new QStandardItemModel(completer);  /** model pu catalogul pacientilor */
    proxyPatient    = new BaseSortFilterProxyModel(this); /** proxy pu sortarea pacientilor */
    data_percentage = new DataPercentage(this);           /** clasa unde se afla datele percentilelor */

    style_toolButton = db->getStyleForToolButton(); /** determ. stilul btn pu descr.format. si concluziilor */

    ui->editDocDate->setDisplayFormat("dd.MM.yyyy hh:mm:ss"); /** setarile datei documentului */
    ui->editDocDate->setCalendarPopup(true);

    initGroupRadioButton();  /** initiem grupelor pu Radio button: gynecology & gestation */
    initFooterDoc();         /** setam numele autorului + imaginea utilizatorului */
    initSetCompleter();      /** initializam setarea completerului */
    constructionFormVideo(); /** forma video */
    initConnections();       /** initiem conectarile */

    ui->gynecology_text_date_menstruation->setText(""); /** stergem textul LMP */

    /** pozitionrea ferestrei */
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
    this->resize(1350, 800);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);

    /** frame si redimensionarea */
    ui->frame_table->resize(900, ui->frame_btn->height()); /** initial */
    initSetStyleFrame(); /** setam stilul frame */
}

DocReportEcho::~DocReportEcho()
{
    delete player;
#if defined(Q_OS_LINUX) || defined (Q_OS_WIN)
    delete videoWidget;
#elif defined(Q_OS_MACOS)
    delete scene;
    delete view;
#endif
    delete ui;
}

void DocReportEcho::appendSectionSystem(SectionSystem f)
{
    m_sectionsSystem |= f;
    emit sectionsSystemChanged();
}

void DocReportEcho::appendSectionSystem(SectionsSystem f)
{
    m_sectionsSystem |= f;
    emit sectionsSystemChanged();
}

DocReportEcho::SectionsSystem DocReportEcho::getSectionsSystem() const
{
    return m_sectionsSystem;
}

void DocReportEcho::setSectionsSystem(SectionsSystem s)
{
    if (m_sectionsSystem == s)
        return;
    m_sectionsSystem = s;
    emit sectionsSystemChanged();
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

Ui::DocReportEcho *DocReportEcho::uiPtr()
{
    return ui;
}

const Ui::DocReportEcho *DocReportEcho::uiPtr() const
{
    return ui;
}

void DocReportEcho::markModified()
{
    setWindowModified(true);
}

int DocReportEcho::getViewExaminationGestation(const int gest) const
{
    if (gest == 0 && group_btn_gestation0) return group_btn_gestation0->checkedId();
    else if (gest == 1 && group_btn_gestation1) return group_btn_gestation1->checkedId();
    else if (gest == 2 && group_btn_gestation2) return group_btn_gestation2->checkedId();
    else return -1;
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

QStringList DocReportEcho::getAllRecommandation() const
{
    return all_recommandation;
}

void DocReportEcho::appendAllRecomandation(const QString text)
{
    all_recommandation << text;
}

void DocReportEcho::enforceMaxForSender()
{
    QObject* s = sender();
    if (!s)
        return;

    // citește limita (default 300)
    int maxLen = s->property("maxLen").toInt();
    if (maxLen <= 0)
        maxLen = 300;

    auto clampDoc = [maxLen](auto* w) {
        if (!w)
            return;
        QString t = w->toPlainText();
        if (t.size() <= maxLen)
            return;
        QSignalBlocker blocker(w);
        int pos = w->textCursor().position();
        t.truncate(maxLen);
        w->setPlainText(t);
        QTextCursor c = w->textCursor();
        c.setPosition(std::min(pos, maxLen));
        w->setTextCursor(c);
    };

    // suportă atât QTextEdit cât și QPlainTextEdit
    if (auto *te = qobject_cast<QTextEdit*>(s))
        clampDoc(te);
    else if (auto *pe = qobject_cast<QPlainTextEdit*>(s))
        clampDoc(pe);
}

void DocReportEcho::updateTextConcluzionBySystem()
{
    ui->concluzion->clear();

    for (auto& r : rows_template_concluzion) {
        if (! (m_sectionsSystem & r.section_system))
            continue;
        if (! r.item_edit)
            continue;
        if (r.item_edit->toPlainText().isEmpty())
            continue;
        ui->concluzion->appendPlainText(r.item_edit->toPlainText());
    }
}

void DocReportEcho::slot_sectionsSystemChanged()
{
    m_organs_internal = (m_sectionsSystem & OrgansInternal);
    m_urinary_system  = (m_sectionsSystem & UrinarySystem);
    m_prostate        = (m_sectionsSystem & Prostate);
    m_gynecology      = (m_sectionsSystem & Gynecology);
    m_breast          = (m_sectionsSystem & Breast);
    m_thyroide        = (m_sectionsSystem & Thyroid);
    m_gestation0      = (m_sectionsSystem & Gestation0);
    m_gestation1      = (m_sectionsSystem & Gestation1);
    m_gestation2      = (m_sectionsSystem & Gestation2);
    m_gestation3      = false;
    m_lymphNodes      = (m_sectionsSystem & LymphNodes);
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

inline bool setPixmapBase64(QLabel* label, const QByteArray& base64, const QSize& targetSize, int* imagesCounter)
{
    if (!label)
        return false;

    if (base64.isEmpty()) {
        label->clear();
        return false;
    }

    QByteArray raw = QByteArray::fromBase64(base64);
    if (raw.isEmpty()) {
        label->clear();
        return false;
    }

    QBuffer buffer(&raw);
    buffer.open(QIODevice::ReadOnly);

    QImageReader reader(&buffer);
    reader.setAutoTransform(true);

    QImage img = reader.read();
    if (img.isNull()) {
        label->clear();
        return false;
    }

    QPixmap px = QPixmap::fromImage(std::move(img));
    const QSize size = (targetSize.isValid() ? targetSize : QSize(640, 400));
    px = px.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(std::move(px));

    if (imagesCounter)
        ++(*imagesCounter);

    return true;
}

void DocReportEcho::loadImageOpeningDocument()
{
    /** 1. Interogare */
    QSqlQuery qry((globals().thisMySQL) ? db->getDatabase() : db->getDatabaseImage());
    qry.prepare("SELECT * FROM imagesReports WHERE id_reportEcho = ?");
    qry.addBindValue(m_id);

    if (! qry.exec()) {
        qWarning(logWarning()) << tr("Eroare de executare a solicitării 'loadImageOpeningDocument': %1")
                                      .arg(qry.lastError().text());
        setCountImages(0);
        return;
    }

    if (! qry.next()) { // Nu există rând
        setCountImages(0);
        return;
    }
    const QSqlRecord rec = qry.record();

    /** 2. Incarcare prin iteratie */
    int imagesLoaded = 0;
    for (size_t i = 0; i < rows_items_images.size(); ++i) {
        const auto& row = rows_items_images[i];
        const int idx = static_cast<int>(i) + 1; // coloanele 1..5

        const QString image_col   = QStringLiteral("image_%1").arg(idx);
        const QString comment_col = QStringLiteral("comment_%1").arg(idx);

        const int image_col_idx   = rec.indexOf(image_col);
        const int comment_col_idx = rec.indexOf(comment_col);

        //--- Imagine
        if (image_col_idx >= 0) {
            const QByteArray base64Data = qry.value(image_col_idx).toByteArray();
            const QSize targetSize = QSize(640, 400);//row.label_img ? row.label_img->size() : QSize(640, 400);
            setPixmapBase64(row.label_img, base64Data, targetSize, &imagesLoaded);
        } else if (row.label_img) {
            row.label_img->clear();
        }

        //--- Comentariu
        if (comment_col_idx >= 0) {
            const QString cmt = qry.value(comment_col_idx).toString();
            if (row.comment_img && ! cmt.isEmpty())
                row.comment_img->setPlainText(cmt);
        } else if (row.comment_img) {
            row.comment_img->clear();
        }
    }

    /** 3. Setam numarul de imagini */
    setCountImages(imagesLoaded);

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

void DocReportEcho::onLinkActivatedForOpenImg(const QString &link)
{
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

    auto *m_sender = qobject_cast<QLabel*>(sender());

    if (! m_sender) return;
    if (rows_items_images.empty()) return;
    if (link != "#LoadImage") return;

    for (const auto& r : rows_items_images) {
        if (m_sender == r.label_img) {
            QFileDialog dialog(this, tr("Open File"));
            initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

            if (dialog.exec() == QDialog::Accepted)
                if (! dialog.selectedFiles().constFirst().isEmpty())
                    loadFile(dialog.selectedFiles().constFirst(), r.nr_img);
            dialog.close();
            break;
        }
    }
}

void DocReportEcho::clickClearImage()
{
    auto *b = qobject_cast<QToolButton*>(sender());
    if (!b)
        return;

    if (rows_items_images.empty())
        return;

    for (const auto& r : rows_items_images) {
        if (b == r.btn) {
            r.label_img->setText("");
            r.label_img->setTextFormat(Qt::RichText);
            r.label_img->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
            r.comment_img->clear();
            removeImageIntoTableimagesReports(r.nr_img);
            if (m_count_images > 0)
                setCountImages(m_count_images - 1);
            break;
        }
    }
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
    /** activarea controlului evenimentelor asupra butoanelor */
    ui->btnParameters->setMouseTracking(true);
    ui->btnParameters->installEventFilter(this);
    ui->btnHistory->setMouseTracking(true);
    ui->btnHistory->installEventFilter(this);
    ui->btnOpenCatPatient->setMouseTracking(true);
    ui->btnOpenCatPatient->installEventFilter(this);
    ui->btnOpenDocErderEcho->setMouseTracking(true);
    ui->btnOpenDocErderEcho->installEventFilter(this);

    /** conectarea la modificarea proprietatilor documentului */
    connect(this, &DocReportEcho::sectionsSystemChanged, this, &DocReportEcho::slot_sectionsSystemChanged, Qt::UniqueConnection);
    connect(this, &DocReportEcho::ItNewChanged, this, &DocReportEcho::slot_ItNewChanged, Qt::UniqueConnection);
    connect(this, &DocReportEcho::IdChanged, this, &DocReportEcho::slot_IdChanged, Qt::UniqueConnection);
    connect(this, &DocReportEcho::IdPacientChanged, this, &DocReportEcho::slot_IdPacientChanged, Qt::UniqueConnection);
    connect(this, &DocReportEcho::IdDocOrderEchoChanged, this, &DocReportEcho::slot_IdDocOrderEchoChanged, Qt::UniqueConnection);

    /** setam stilul butoanelor */
    ui->btnHistory->setStyleSheet(style_toolButton);
    ui->btnOpenCatPatient->setStyleSheet(style_toolButton);
    ui->btnOpenDocErderEcho->setStyleSheet(style_toolButton);

    /** navigarea paginelor prin functia universala */
    QList<QCommandLinkButton*> list_btn_link = ui->frame_btn->findChildren<QCommandLinkButton*>();
    for (int n = 0; n < list_btn_link.count(); n++) {
        if (list_btn_link[n]->objectName() == "btnNormograms" ||
            list_btn_link[n]->objectName() == "btnComment")
            continue;
        connect(list_btn_link[n], &QAbstractButton::clicked, this, &DocReportEcho::clickNavSystem, Qt::UniqueConnection);
    }

    /** conectarea la btn suplimentare */
    connect(ui->btnParameters, &QPushButton::clicked, this, &DocReportEcho::openParameters, Qt::UniqueConnection);
    connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged, Qt::UniqueConnection);
    connect(ui->btnOpenCatPatient, &QPushButton::clicked, this, &DocReportEcho::openCatPatient, Qt::UniqueConnection);
    connect(ui->btnHistory, &QPushButton::clicked, this, &DocReportEcho::openHistoryPatient, Qt::UniqueConnection);
    connect(ui->btnOpenDocErderEcho, &QPushButton::clicked, this, &DocReportEcho::openDocOrderEcho, Qt::UniqueConnection);

    /** conectarea si procesarea atasarii/eliminarii imaginilor */
    for (const auto& r : rows_items_images) {
        r.label_img->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        r.label_img->setTextFormat(Qt::RichText);
        r.label_img->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        connect(r.label_img, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&DocReportEcho::onLinkActivatedForOpenImg), Qt::UniqueConnection);
        connect(r.btn, &QToolButton::clicked, this, &DocReportEcho::clickClearImage, Qt::UniqueConnection);
        connect(r.comment_img, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified, Qt::UniqueConnection);
    }

    // calculate gestation age
    connect(ui->gestation0_LMP, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged, Qt::UniqueConnection);
    connect(ui->gestation1_LMP, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged, Qt::UniqueConnection);
    connect(ui->gestation2_dateMenstruation, &QDateEdit::dateChanged, this, &DocReportEcho::onDateLMPChanged, Qt::UniqueConnection);
    connect(ui->gestation2_fetus_age, &QLineEdit::textEdited, this, &DocReportEcho::setDueDateGestation2, Qt::UniqueConnection);

    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::slot_CountImagesChanged, Qt::UniqueConnection);
    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified, Qt::UniqueConnection);
    connect(this, &DocReportEcho::CountVideoChanged, this, &DocReportEcho::slot_CountVideoChanged, Qt::UniqueConnection);

    createMenuPrint();

    // descrierea doppler
    connect(ui->gestation2_fetusMass, &QLineEdit::editingFinished, this, &DocReportEcho::updateDescriptionFetusWeight, Qt::UniqueConnection);
    connect(ui->gestation2_ombilic_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_uterLeft_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_uterRight_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler, Qt::UniqueConnection);
    connect(ui->gestation2_cerebral_PI, &QLineEdit::editingFinished, this, &DocReportEcho::updateTextDescriptionDoppler, Qt::UniqueConnection);

    connect(ui->btnOk, &QPushButton::clicked, this, &DocReportEcho::onWritingDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QPushButton::clicked, this, &DocReportEcho::onWritingData, Qt::UniqueConnection);
    connect(ui->btnClose, &QPushButton::clicked, this, &DocReportEcho::onClose, Qt::UniqueConnection);
}

void DocReportEcho::connectionTemplateConcluzion()
{
    const auto c = Qt::UniqueConnection;

    for (auto& r : rows_templateSystem_action) {
        if (! (m_sectionsSystem & r.section_system))
            continue;

        if (! r.item_edit)    // opțional: protecție la nullptr
            continue;

        connect(r.item_edit, &LineEditCustom::onClickSelect,
                this, &DocReportEcho::handlerOpenSelectTemplate, c);
        connect(r.item_edit, &LineEditCustom::onClickAddItem,
                this, &DocReportEcho::handlerAddTemplate, c);
        connect(r.item_edit, &LineEditCustom::textChanged,
                this, &DocReportEcho::dataWasModified, c);
    }

    for (auto& r : rows_templateSystem_btn) {
        if (! (m_sectionsSystem & r.section_system))
            continue;
        if (! r.item_edit)
            continue;
        r.btn_add->setStyleSheet(style_toolButton);
        r.btn_select->setStyleSheet(style_toolButton);
        connect(r.btn_select, &QAbstractButton::clicked,
                this, &DocReportEcho::handlerOpenSelectTemplate, c);
        connect(r.btn_add, &QAbstractButton::clicked,
                this, &DocReportEcho::handlerAddTemplate, c);
        connect(r.item_edit, &QPlainTextEdit::textChanged,
                this, &DocReportEcho::dataWasModified, c);
    }

    for (auto& r : rows_template_concluzion) {
        if (! (m_sectionsSystem & r.section_system))
            continue;
        if (! r.item_edit)
            continue;

        if (r.btn_add)
            r.btn_add->setStyleSheet(style_toolButton);
        if (r.btn_select)
            r.btn_select->setStyleSheet(style_toolButton);

        connect(r.btn_select, &QAbstractButton::clicked,
                this, &DocReportEcho::handlerSelectTemplateConcluzion, c);
        connect(r.btn_add, &QAbstractButton::clicked,
                this, &DocReportEcho::handlerAddTemplateConcluzion, c);
        connect(r.item_edit, &QPlainTextEdit::textChanged,
                this, &DocReportEcho::dataWasModified, c);
        connect(r.item_edit, &QPlainTextEdit::textChanged,
                this, &DocReportEcho::updateTextConcluzionBySystem);
    }

    for (auto& r : rows_all_recommandation) {
        if (! (m_sectionsSystem & r.section_system))
            continue;
        if (! r.item_edit)
            continue;
        all_recommandation << r.item_edit->text();
    }
}

// *******************************************************************
// **************** BUTOANE SUPLIMENTARE - PARAMETRII, PACIENT ETC. **

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
    dialogInvestig->set_t_lymphNodes(m_lymphNodes);

    if (dialogInvestig->exec() != QDialog::Accepted)
        return;

    m_handler->applyPropertyMaxLengthText();
    m_handler->applyDefaultsForSelected();
    m_handler->appleConnections(true);

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

void DocReportEcho::handlerOpenSelectTemplate()
{
    QObject* s = sender();
    if (!s) return;

    /** ----- 1. LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_templateSystem_action.begin(),
                               rows_templateSystem_action.end(),
                               [edit](const TemplateSystemActions& r)
                               {
                                   return r.item_edit == edit && edit->actionOpenList() == r.action_select;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_templateSystem_action.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;

        /** deschide fereastra de selecție */
        auto* dlg = new CatForSqlTableModel(this);
        dlg->setWindowIcon(QIcon(":/img/templates.png"));
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->m_filter_templates = r.name_system;
        dlg->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::SystemTemplates);
        dlg->setProperty("typeForm",    CatForSqlTableModel::TypeForm::SelectForm);
        dlg->show();

        /** folosit QPointer în lambda ca protecție
         ** în cazul închiderii dialogului înainte de emiterea semnalului */
        QPointer<CatForSqlTableModel> dlgPtr = dlg;
        QPointer<QLineEdit> editPtr = r.item_edit; // LineEditCustom* e QLineEdit*
        connect(dlg, &CatForSqlTableModel::mSelectData, this, [dlgPtr, editPtr](){
            if (! dlgPtr || ! editPtr)
                return;
            editPtr->setText(dlgPtr->getSelectName());
            dlgPtr->close();
        });
        return;
    };

    /** ----- 2. QAbstractButton + QPlainTextEdit --------- */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_templateSystem_btn.begin(),
                               rows_templateSystem_btn.end(),
                               [btn] (const TemplateSystemBtn& r)
                               {
                                   return r.btn_select == btn;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_templateSystem_btn.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;

        /** deschide fereastra de selecție */
        auto* dlg = new CatForSqlTableModel(this);
        dlg->setWindowIcon(QIcon(":/img/templates.png"));
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->m_filter_templates = r.name_system;
        dlg->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::SystemTemplates);
        dlg->setProperty("typeForm",    CatForSqlTableModel::TypeForm::SelectForm);
        dlg->show();

        /** folosit QPointer în lambda ca protecție
         ** în cazul închiderii dialogului înainte de emiterea semnalului */
        QPointer<CatForSqlTableModel> dlgPtr = dlg;
        QPointer<QPlainTextEdit> editPtr = r.item_edit; // QPlainTextEdit*
        connect(dlg, &CatForSqlTableModel::mSelectData, this, [dlgPtr, editPtr](){
            if (! dlgPtr || ! editPtr)
                return;
            editPtr->setPlainText(dlgPtr->getSelectName());
            dlgPtr->close();
        });
        return;
    };
}

void DocReportEcho::insertTemplateIntoDB(const QString text, const QString name_system)
{
    /** verificam dublajul */
    QSqlQuery qry;
    qry.prepare(R"(SELECT name FROM formationsSystemTemplates WHERE name = ? AND typeSystem = ?)");
    qry.addBindValue(text);
    qry.addBindValue(name_system);
    if (qry.exec() && qry.next()) {
        if (qry.value(0).toString() == text) {
            QMessageBox msgBox(QMessageBox::Question,
                               tr("Verificarea dublajului"),
                               tr("Descrierea <b><u>'%1'</u></b> exist\304\203 ca \310\231ablon.<br>"
                                  "Dori\310\233i s\304\203 prelungi\310\233i validarea ?")
                                   .arg(text),
                               QMessageBox::Yes | QMessageBox::No, this);

            if (msgBox.exec() == QMessageBox::No){
                return;
            }
        }
    }

    /** inseram sablonul */
    qry.prepare(R"(INSERT INTO formationsSystemTemplates (id, deletionMark, name, typeSystem) VALUES (?, ?, ?, ?))");
    qry.addBindValue(db->getLastIdForTable("formationsSystemTemplates") + 1);
    qry.addBindValue(0);
    qry.addBindValue(text);
    qry.addBindValue(name_system);
    if (qry.exec()) {
        popUp->setPopupText(tr("\310\230ablonul ad\304\203ugat cu succes<br>"
                               "\303\256n baza de date."));
        popUp->show();
    }
}

void DocReportEcho::handlerAddTemplate()
{
    QObject* s = sender();
    if (!s) return;

    /** ----- 1. LineEditCustom + QLineEdit --------- */
    if (auto edit = qobject_cast<LineEditCustom*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_templateSystem_action.begin(),
                               rows_templateSystem_action.end(),
                               [edit](const TemplateSystemActions& r)
                               {
                                   return r.item_edit == edit && edit->actionAddItem() == r.action_add;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_templateSystem_action.end())
            return;

        const auto& r = *it;                                      /** pointer la obiect */
        insertTemplateIntoDB(r.item_edit->text(), r.name_system); /** inseram sablonul */
        return;                                                   /** nu mai prelungim */
    }

    /** ----- 2. QAbstractButton + QPlainTextEdit --------- */
    if (auto btn = qobject_cast<QAbstractButton*>(s)) {

        /** functia standart de cautare a randului dupa conditia */
        auto it = std::find_if(rows_templateSystem_btn.begin(),
                               rows_templateSystem_btn.end(),
                               [btn] (const TemplateSystemBtn& r)
                               {
                                   return r.btn_add == btn;
                               });

        /** daca it ajuns pana la sfarsit ne oprim */
        if (it == rows_templateSystem_btn.end())
            return;

        /** pointer la obiect */
        const auto& r = *it;

        /** inseram sablonul */
        insertTemplateIntoDB(r.item_edit->toPlainText(), r.name_system);
    }
}

void DocReportEcho::insertTemplateConcluzionIntoDB(const QString text, const QString name_system)
{
    QSqlQuery qry;
    qry.prepare(R"(SELECT name FROM conclusionTemplates WHERE name = ?)");
    qry.addBindValue(text);
    if (qry.exec()){
        if (qry.next() && qry.value(0).toString() == text) {

            QMessageBox msgBox(QMessageBox::Question,
                               tr("Verificarea dublajului"),
                               tr("Concluzia <b>%1</b> exist\304\203 ca \310\231ablon.<br>"
                                  "Dori\310\233i s\304\203 prelungi\310\233i validarea ?").arg(text),
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
    qry.addBindValue(text);
    qry.addBindValue(name_system);
    if (qry.exec()){
        popUp->setPopupText(tr("\310\230ablonul ad\304\203ugat cu succes<br>"
                               "\303\256n baza de date."));
        popUp->show();
    } else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Adaugarea \310\231ablonului"));
        msgBox.setText(tr("\310\230ablonul - <b>%1</b> - nu a fost adaugat.").arg(text));
        msgBox.setDetailedText(tr((qry.lastError().text().isEmpty()) ? "eroarea indisponibila" : qry.lastError().text().toStdString().c_str()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
}

void DocReportEcho::handlerSelectTemplateConcluzion()
{
    if (rows_template_concluzion.empty()) return;
    auto s = qobject_cast<QAbstractButton*>(sender());
    if (! s) return;

    /** functia standart de cautare a randului dupa conditia */
    auto it = std::find_if(rows_template_concluzion.begin(),
                           rows_template_concluzion.end(),
                           [s](const TemplateConcluzions& r)
                           {
                               return r.btn_select == s;
                           });

    /** daca it ajuns pana la sfarsit ne oprim */
    if (it == rows_template_concluzion.end())
        return;

    /** pointer la obiect */
    const auto& r = *it;

    CatForSqlTableModel* template_concluzion = new CatForSqlTableModel(this);
    template_concluzion->setWindowIcon(QIcon(":/img/templates.png"));
    template_concluzion->setAttribute(Qt::WA_DeleteOnClose);
    template_concluzion->m_filter_templates = r.name_system;
    template_concluzion->setProperty("typeCatalog", CatForSqlTableModel::TypeCatalog::ConclusionTemplates);
    template_concluzion->setProperty("typeForm", CatForSqlTableModel::TypeForm::SelectForm);
    template_concluzion->show();

    /** folosit QPointer în lambda ca protecție
    ** în cazul închiderii dialogului înainte de emiterea semnalului */
    QPointer<CatForSqlTableModel> dlgPtr = template_concluzion;
    QPointer<QPlainTextEdit> editPtr = r.item_edit; // QPlainTextEdit*
    connect(template_concluzion, &CatForSqlTableModel::mSelectData, this, [dlgPtr, editPtr](){
        if (! dlgPtr || ! editPtr)
            return;
        editPtr->setPlainText(editPtr->toPlainText() + " " + dlgPtr->getSelectName());
        dlgPtr->close();
    });
}

void DocReportEcho::handlerAddTemplateConcluzion()
{
    if (rows_template_concluzion.empty()) return;
    auto s = qobject_cast<QAbstractButton*>(sender());
    if (! s) return;

    /** functia standart de cautare a randului dupa conditia */
    auto it = std::find_if(rows_template_concluzion.begin(),
                           rows_template_concluzion.end(),
                           [s](const TemplateConcluzions& r)
                           {
                               return r.btn_add == s;
                           });

    /** daca it ajuns pana la sfarsit ne oprim */
    if (it == rows_template_concluzion.end())
        return;

    /** pointer la obiect */
    const auto& r = *it;

    /** inseram sablonul concluziei in BD*/
    insertTemplateConcluzionIntoDB(r.item_edit->toPlainText(), r.name_system);
}

// *******************************************************************
// **************** PANOU LATERAL CU BTN PE SISTEME ******************

void DocReportEcho::clickNavSystem()
{
    auto *b = qobject_cast<QAbstractButton*>(sender());
    if (!b)
        return;

    if (rows_btn_page.empty())
        return;

    for (const auto& r : rows_btn_page) {
        if (b == r.btn) {
            const int indexPage = r.pageIndex;
            ui->stackedWidget->setCurrentIndex(indexPage);
            break;
        }
    }
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

void DocReportEcho::clickBtnNormograms()
{
    normograms = new Normograms(this);
    normograms->setAttribute(Qt::WA_DeleteOnClose);
    normograms->show();
}

// *******************************************************************
// **************** CREAREA MENIULUI DE PRINTARE *********************

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
// **************** PROCESAREA SLOT-URILOR ***************************

void DocReportEcho::slot_ItNewChanged()
{
    if (m_itNew){

        setWindowTitle(tr("Raport ecografic (crearea) %1").arg("[*]"));

        connect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
        timer->start(1000);                    // inițierea timerului pu data docum.

        setIdUser(globals().idUserApp);

        m_handler->applyPropertyMaxLengthText(); // limitarea lungimei textului + placeholder
        m_handler->applyDefaultsForSelected();   // textul implicit/initial
        m_handler->appleConnections(m_itNew);    // conectarile la modificarea formei
        connectionTemplateConcluzion();

        ui->comment->setHidden(true);
    }
    initEnableBtn();
    updateStyleBtnInvestigations();
}

void DocReportEcho::slot_IdChanged()
{
    if (m_id == Enums::IDX::IDX_UNKNOW)
        return;

    m_handler->loadAllSelectedIntoForm();
    m_handler->applyPropertyMaxLengthText();
    m_handler->appleConnections(true);
    connectionTemplateConcluzion();

    initEnableBtn();
    updateStyleBtnInvestigations();

     /** verificam daca este indicata directoriu de stocare a
      ** fisierelor video.
      ** Daca directoriu nu este indicat deschiderea documentului
      ** poate dura foarte mult din cauza cautarii prin tot
      ** hard discul a fisierelor video */
    if (globals().pathDirectoryVideo != nullptr) {
        findVideoFiles();
    };

    /** extragem imaginile documentului */
    disconnect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified);
    loadImageOpeningDocument();
    connect(this, &DocReportEcho::CountImagesChanged, this, &DocReportEcho::dataWasModified);

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

    if (m_gestation2 && ui->gestation2_concluzion->toPlainText().isEmpty()){

        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicat\304\203 <b>'Concluzia (sarcina trimestru II/III)'</b> raportului !!!<br>"
                                    "Dori\310\233i s\304\203 continua\310\233i validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);

        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    return true;
}

void DocReportEcho::onPrint(Enums::TYPE_PRINT _typeReport, QString &filePDF)
{
    m_handler->printDoc(_typeReport, filePDF, m_report, db);
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
        if (! m_handler->insertDataDoc(db, details_error)){
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

        if (! m_handler->updateDataDoc(db, details_error)){
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
// **************** FUNCTII DE INITIALIZARE **************************

void DocReportEcho::initInstallEventFilter()
{
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
}

void DocReportEcho::initRequiredStructure()
{
    /** structura: btn(QCommandLinkButton/QAbstractButton) + pageIndex(QEnum) */
    rows_btn_page = {
        { ui->btnOrgansInternal, page_organs_internal },
        { ui->btnUrinarySystem , page_urinary_system  },
        { ui->btnProstate      , page_prostate        },
        { ui->btnGynecology    , page_gynecology      },
        { ui->btnBreast        , page_breast          },
        { ui->btnThyroid       , page_thyroid         },
        { ui->btnGestation0    , page_gestation0      },
        { ui->btnGestation1    , page_gestation1      },
        { ui->btnGestation2    , page_gestation2      },
        { ui->btnLymphNodes    , page_LymphNodes      },
        { ui->btnImages        , page_images          },
        { ui->btnVideo         , page_video           }
    };

    /** structura: action_select(QAction) + action_add(QAction) + item_edit(LineEditCustom) + name_system(QString) + section_system(QEnum) */
    rows_templateSystem_action = {
        //--- organs internal
        {ui->liver_formations->actionOpenList(), ui->liver_formations->actionAddItem() , ui->liver_formations , "Ficat", OrgansInternal},
        {ui->cholecist_formations->actionOpenList(), ui->cholecist_formations->actionAddItem(), ui->cholecist_formations, "Colecist", OrgansInternal},
        {ui->pancreas_formations->actionOpenList(), ui->pancreas_formations->actionAddItem(), ui->pancreas_formations, "Pancreas", OrgansInternal},
        {ui->spleen_formations->actionOpenList(), ui->spleen_formations->actionAddItem(), ui->spleen_formations, "Splina", OrgansInternal},
        {ui->organsInternal_recommendation->actionOpenList(), ui->organsInternal_recommendation->actionAddItem(), ui->organsInternal_recommendation, "Recomandari (org.interne)", OrgansInternal},
        //--- urinary system
        {ui->kidney_formations->actionOpenList(), ui->kidney_formations->actionAddItem(), ui->kidney_formations, "Rinichi", UrinarySystem},
        {ui->bladder_formations->actionOpenList(), ui->bladder_formations->actionAddItem(), ui->bladder_formations, "V.urinara", UrinarySystem},
        {ui->urinary_system_recommendation->actionOpenList(), ui->urinary_system_recommendation->actionAddItem(), ui->urinary_system_recommendation, "Recomandari (s.urinar)", UrinarySystem},
        //--- prostate
        {ui->prostate_formations->actionOpenList(), ui->prostate_formations->actionAddItem(), ui->prostate_formations, "Prostata", Prostate},
        {ui->prostate_recommendation->actionOpenList(), ui->prostate_recommendation->actionAddItem(), ui->prostate_recommendation, "Recomandari (prostata)", Prostate},
        //--- gynecology
        {ui->gynecology_recommendation->actionOpenList(), ui->gynecology_recommendation->actionAddItem(), ui->gynecology_recommendation, "Recomandari (ginecologia)", Gynecology},
        //--- breast
        {ui->breast_recommendation->actionOpenList(), ui->breast_recommendation->actionAddItem(), ui->breast_recommendation, "Recomandari (gl.mamare)", Breast},
        //--- thyroid
        {ui->thyroid_recommendation->actionOpenList(), ui->thyroid_recommendation->actionAddItem(), ui->thyroid_recommendation, "Recomandari (tiroida)", Thyroid},
        //--- gestation
        {ui->gestation0_recommendation->actionOpenList(), ui->gestation0_recommendation->actionAddItem(), ui->gestation0_recommendation, "Recomandari (gestatation0)", Gestation0},
        {ui->gestation1_recommendation->actionOpenList(), ui->gestation1_recommendation->actionAddItem(), ui->gestation1_recommendation, "Recomandari (gestatation1)", Gestation1},
        {ui->gestation2_recommendation->actionOpenList(), ui->gestation2_recommendation->actionAddItem(), ui->gestation2_recommendation, "Recomandari (gestatation2)", Gestation2}
    };

    /** structura: btn_select(QAbstractButton) + btn_add(QAbstractButton) + item_edit(QPlainTextEdit) + name_system(QString) + section_system(QEnum) */
    rows_templateSystem_btn = {
        {ui->btnSelectTemplatesIntestinalFormations, ui->btnAddTemplatesIntestinalFormations, ui->intestinalHandles                , "Intestine"               , OrgansInternal},
        {ui->btnSelectTempletsAdrenalGlands        , ui->btnAddTempletsAdrenalGlands        , ui->adrenalGlands                    , "Gl.suprarenale"          , UrinarySystem},
        {ui->btnSelectTempletsBreastLeft           , ui->btnAddTempletsBreastLeft           , ui->breast_left_formations           , "Gl.mamara (stanga)"      , Breast},
        {ui->btnSelectTempletsBreastRight          , ui->btnAddTempletsBreastRight          , ui->breast_right_formations          , "Gl.mamara (dreapta)"     , Breast},
        {ui->btnSelectTempletsThyroid              , ui->btnAddTempletsThyroid              , ui->thyroid_formations               , "Tiroida"                 , Thyroid},
        {ui->btnSelectTempletsUter                 , ui->btnAddTempletsUter                 , ui->gynecology_uterus_formations     , "Ginecologia (uter)"      , Gynecology},
        {ui->btnSelectTempletsOvarLeft             , ui->btnAddTempletsOvarLeft             , ui->gynecology_ovary_formations_left , "Ginecologia (ovar stang)", Gynecology},
        {ui->btnSelectTempletsOvarRight            , ui->btnAddTempletsOvarRight            , ui->gynecology_ovary_formations_right, "Ginecologia (ovar drept)", Gynecology}
    };

    /** structura: btn(QToolButton) + label_img(QLabel) + comment_img(QPlainTextEdit) + nr_img(int) */
    rows_items_images = {
        {ui->btn_clear_image1, ui->image1, ui->comment_image1, n_image1},
        {ui->btn_clear_image2, ui->image2, ui->comment_image2, n_image2},
        {ui->btn_clear_image3, ui->image3, ui->comment_image3, n_image3},
        {ui->btn_clear_image4, ui->image4, ui->comment_image4, n_image4},
        {ui->btn_clear_image5, ui->image5, ui->comment_image5, n_image5},
    };

    /** structura: btn_select(QAbstractButton) + btn_add(QAbstractButton) + item_edit(QPlainTextEdit) + name_system(QString) */
    rows_template_concluzion = {
        {ui->btn_template_organsInternal , ui->btn_add_template_organsInternal, ui->organsInternal_concluzion, "Organe interne"  , OrgansInternal        },
        {ui->btn_template_urinary_system , ui->btn_add_template_urinary_system, ui->urinary_system_concluzion, "Sistemul urinar" , UrinarySystem         },
        {ui->btn_template_prostate       , ui->btn_add_template_prostate      , ui->prostate_concluzion      , "Prostata"        , Prostate              },
        {ui->btn_template_gynecology     , ui->btn_add_template_gynecology    , ui->gynecology_concluzion    , "Ginecologia"     , Gynecology            },
        {ui->btn_template_breast         , ui->btn_add_template_breast        , ui->breast_concluzion        , "Gl.mamare"       , Breast                },
        {ui->btn_template_thyroid        , ui->btn_add_template_thyroid       , ui->thyroid_concluzion       , "Tiroida"         , Thyroid               },
        {ui->btn_template_gestation0     , ui->btn_add_template_gestation0    , ui->gestation0_concluzion    , "Sarcina până la 11 săptămâni", Gestation0},
        {ui->btn_template_gestation1     , ui->btn_add_template_gestation1    , ui->gestation1_concluzion    , "Sarcina 11-14 săptămâni"     , Gestation1},
        {nullptr                         , nullptr                            , ui->gestation2_concluzion    , ""                            , Gestation2},
        {nullptr                         , nullptr                            , ui->ln_conluzion             , "Gangl.limfatici"             , LymphNodes}
    };

    /** structura: item_edit(QLineEdit) + section_system(QEnum)  */
    rows_all_recommandation = {
        {ui->organsInternal_recommendation , OrgansInternal},
        {ui->urinary_system_recommendation , UrinarySystem},
        {ui->prostate_recommendation       , Prostate},
        {ui->gynecology_recommendation     , Gynecology},
        {ui->breast_recommendation         , Breast},
        {ui->thyroid_recommendation        , Thyroid},
        {ui->gestation0_recommendation     , Gestation0},
        {ui->gestation1_recommendation     , Gestation1},
        {ui->ln_recommand                  , LymphNodes}
    };

    /** structura pu radiobutton */
    rows_radionBtn = {
        {group_btn_prostate  , ui->prostate_radioBtn_transabdom, ui->prostate_radioBtn_transrectal, nullptr                      , ui->prostate_radioBtn_transabdom},
        {group_btn_gynecology, ui->gynecology_btn_transabdom   , ui->gynecology_btn_transvaginal  , nullptr                      , ui->gynecology_btn_transvaginal },
        {group_btn_gestation0, ui->gestation0_view_good        , ui->gestation0_view_medium       , ui->gestation0_view_difficult, ui->gestation0_view_medium      },
        {group_btn_gestation1, ui->gestation1_view_good        , ui->gestation1_view_medium       , ui->gestation1_view_difficult, ui->gestation1_view_medium      },
        {group_btn_gestation2, ui->gestation2_trimestru2       , ui->gestation2_trimestru3        , nullptr                      , ui->gestation2_trimestru2       }
    };
}

void DocReportEcho::initGroupRadioButton()
{
    // initializam radiobutton
    for (auto& r : rows_radionBtn) {
        r.group = new QButtonGroup(this);
        if (r.btn_1)
            r.group->addButton(r.btn_1);
        if (r.btn_2)
            r.group->addButton(r.btn_2);
        if (r.btn_3)
            r.group->addButton(r.btn_3);
        if (r.btn_checked)
            r.btn_checked->setChecked(true);
    }
    clickedGestation2Trimestru(0);
}

void DocReportEcho::initSetStyleFrame()
{

    if (globals().isSystemThemeDark){
        const QString style_frame = R"(
            QFrame#customFrame
            {
                background-color: #2b2b2b;
                border: 1px solid #555; /* Linie subțire gri */
                border-radius: 5px;
            }
        )";
        ui->frame_btn->setStyleSheet(R"(
            background-color: #2b2b2b;
            border: 1px solid #555;
            border-radius: 5px;
        )");
        ui->frame_table->setObjectName("customFrame");
        ui->frame_table->setStyleSheet(style_frame);

        ui->frame_3->setObjectName("customFrame");
        ui->frame_3->setStyleSheet(style_frame);

        ui->frame_4->setObjectName("customFrame");
        ui->frame_4->setStyleSheet(style_frame);

        ui->frameVideo->setObjectName("customFrame");
        ui->frameVideo->setStyleSheet(style_frame);
    }
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
// **************** ACCESIBILITATEA BUTOANELOR ***********************

// Helper pentru orice „are măcar un bit”
static inline bool any(DocReportEcho::SectionsSystem s, DocReportEcho::SectionsSystem mask) {
    return (s & mask) != 0;
}

void DocReportEcho::initEnableBtn()
{
    const auto s = m_sectionsSystem;

    /** determinam ce sistem e ales
     ** ascundem tot widgetul dacă nu e niciun flag setat (la document nou & fără secțiuni) */
    const bool hasAnySection =
        any(s, OrgansInternal | UrinarySystem | Prostate | Gynecology |
               Breast | Thyroid | Gestation0 | Gestation1 | Gestation2 | LymphNodes);

    ui->stackedWidget->setHidden(m_id == Enums::IDX::IDX_UNKNOW &&
                                 ! hasAnySection);

    /** butonul Normograms e vizibil dacă există oricare din Gestation{0,1,2} */
    const bool hasAnyGestation = any(s, Gestation0 | Gestation1 | Gestation2);
    ui->btnNormograms->setHidden(! hasAnyGestation);

    /** structura cu btn si paginile selectate */
    struct SectionRow {
        QPushButton* btn;
        SectionSystem flag;
        PageReport page;
    };

    /** tabel de rutare: flag + buton + pagina */
    const SectionRow rows[] = {
        { ui->btnOrgansInternal, OrgansInternal, page_organs_internal },
        { ui->btnUrinarySystem , UrinarySystem , page_urinary_system  },
        { ui->btnProstate      , Prostate      , page_prostate        },
        { ui->btnGynecology    , Gynecology    , page_gynecology      },
        { ui->btnBreast        , Breast        , page_breast          },
        { ui->btnThyroid       , Thyroid       , page_thyroid         },
        { ui->btnGestation0    , Gestation0    , page_gestation0      },
        { ui->btnGestation1    , Gestation1    , page_gestation1      },
        { ui->btnGestation2    , Gestation2    , page_gestation2      },
        { ui->btnLymphNodes    , LymphNodes    , page_LymphNodes      },
    };

    /** setăm hidden/enabled pe butoane și memorăm prima pagină disponibilă */
    int firstPage = -1;
    for (const SectionRow& r : rows) {
        const bool present = any(s, r.flag);
        r.btn->setHidden(!present);
        r.btn->setEnabled(present);
        if (present && firstPage == -1)
            firstPage = r.page;
    }

    /** dacă nu e nimic de afișat, gata */
    if (firstPage == -1)
        return;

    /** selectăm prima pagină disponibilă și îi dăm focus butonului asociat */
    ui->stackedWidget->setCurrentIndex(firstPage);

    /** butonul corespunzător paginii curente și dăm focus */
    for (const SectionRow& r : rows) {
        if (r.page == firstPage) {
            r.btn->setFocus();
            break;
        }
    }
}

void DocReportEcho::updateStyleBtnInvestigations()
{
    const QString style_pressed = globals().isSystemThemeDark
        ? R"(QCommandLinkButton { background:#5b5b5b; border:1px inset #00baff; color:#fff; })"
        : R"(background:#C2C2C3; border:1px inset navy;)";

    const QString style_unpressed = globals().isSystemThemeDark
        ? R"(QCommandLinkButton { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #4b4b4b, stop:1 #3c3c3c); border:0; color:#fff; })"
        : R"(background-color:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f6f7fa, stop:1 #dadbde); border:0;)";

    if (rows_btn_page.empty())
        return;

    // determinam pagina curenta
    const int current = ui->stackedWidget->currentIndex();

    // setam stilul
    for (const auto& r : rows_btn_page) {
        r.btn->setStyleSheet( (current == r.pageIndex) ? style_pressed : style_unpressed );
    }

    // Comment are logic aparte: pressed dacă e vizibil
    ui->btnComment->setStyleSheet( ui->comment->isVisible() ? style_pressed : style_unpressed );
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
// **************** ACTUALIZAREA DATELOR IN BD ***********************

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
