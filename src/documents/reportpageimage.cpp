#include "reportpageimage.h"
#include "documents/reportdialog.h"
#include "ui_reportpageimage.h"

#include <QBuffer>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <algorithm>

//----------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------

ReportPageImage::ReportPageImage(DataBase &db,
                                 QSqlDatabase &currentDB,
                                 QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageImage)
    , m_db(db)
    , m_currentDB(currentDB)
    , toolBar(new ToolBarCustom(this, ToolBarCustom::AddEditDelete))
    , toolButtonStyleForIcon(m_db.toolButtonStyleForIcon())
{
    ui->setupUi(this);

    rows_items_images = {
        {ui->image1, ui->comment_image1, n_image1, QByteArray()},
        {ui->image2, ui->comment_image2, n_image2, QByteArray()},
        {ui->image3, ui->comment_image3, n_image3, QByteArray()},
        {ui->image4, ui->comment_image4, n_image4, QByteArray()},
        {ui->image5, ui->comment_image5, n_image5, QByteArray()}
    };

    toolBar->setStyles(toolButtonStyleForIcon);
    ui->layoutToolBar->addWidget(toolBar);

    auto btnSave = new QToolButton(this);
    btnSave->setIcon(QIcon(":/img/btns/save.png"));
    btnSave->setStyleSheet(toolButtonStyleForIcon);
    ui->layoutToolBar->addWidget(btnSave);

    ui->layoutToolBar->addStretch();

    connect(toolBar->getBtnAddDoc(), &QAbstractButton::clicked,
            this, &ReportPageImage::onAddImage, Qt::UniqueConnection);
    connect(toolBar->getBtnEditDoc(), &QAbstractButton::clicked,
            this, &ReportPageImage::onEditImage, Qt::UniqueConnection);
    connect(toolBar->getBtnDeletDoc(), &QAbstractButton::clicked,
            this, &ReportPageImage::onDeleteImage, Qt::UniqueConnection);
    connect(btnSave, &QAbstractButton::clicked,
            this, &ReportPageImage::onSaveImage, Qt::UniqueConnection);
}

ReportPageImage::~ReportPageImage()
{
    delete ui;
}

bool ReportPageImage::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);

    q.prepare("SELECT * FROM imagesReports WHERE id_reportEcho = :id_reportEcho");
    q.bindValue(":id_reportEcho", idReport);
    if(!q.exec()) {
        qWarning(logWarning()).noquote()
        << "ReportPageImage error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    if (!q.next()) {
        qInfo(logInfo()) << "Raportul nu are imagini atașate.";
        return false;
    }

    countImages = 0;

    for (auto &row : rows_items_images) {
        if (row.label_img) {
            row.label_img->clear();
            row.label_img->setText(tr("Nu este setată imaginea"));
        }

        if (row.comment_img)
            row.comment_img->clear();

        row.bArray.clear();
    }

    for (size_t i = 0; i < rows_items_images.size(); ++i) {
        auto& row = rows_items_images[i];
        const int idx = static_cast<int>(i) + 1; // coloanele 1..5

        const QString image_col   = QStringLiteral("image_%1").arg(idx);
        const QString comment_col = QStringLiteral("comment_%1").arg(idx);

        //--- Imagine
        const QByteArray base64Data = q.value(image_col).toByteArray();
        if (!base64Data.isEmpty())
            row.bArray = QByteArray::fromBase64(base64Data);

        const QSize targetSize = QSize(640, 400);
        if (setPixmapBase64(row.label_img, base64Data, targetSize))
            ++countImages;
        else {
            row.bArray.clear();
            if (row.label_img)
                row.label_img->setText(tr("Imagine invalidă"));
        }

        //--- Comentariu
        const QString cmt = q.value(comment_col).toString();
        if (row.comment_img && ! cmt.isEmpty())
            row.comment_img->setPlainText(cmt);
    }

    /** 3. Setam numarul de imagini */
    emit countImagesBtn(countImages);

    return true;
}

bool ReportPageImage::saveData(int idReport)
{
    return saveData(idReport, m_currentDB, QStringLiteral("imagesReports"));
}

bool ReportPageImage::saveData(int idReport, const QSqlDatabase &database, const QString &table)
{
    if (idReport <= 0)
        return false;

    if (rows_items_images.size() < 5)
        return false;

    auto blobOrNull = [](const QByteArray &ba) -> QVariant
    {
        return ba.isEmpty() ? QVariant() : QVariant(ba.toBase64());
    };

    auto textOrNull = [](QPlainTextEdit *edit) -> QVariant
    {
        if (!edit)
            return QVariant();

        const QString text = edit->toPlainText().trimmed();
        return text.isEmpty() ? QVariant() : QVariant(text);
    };

    int idOrder   = 0;
    int idPatient = 0;
    int idUser    = 0;

    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        idOrder   = report->getIdOrder();
        idPatient = report->getIdPatient();
        idUser    = report->getIdUser();
    }

    QSqlQuery check(database);
    if (!check.prepare(QStringLiteral("SELECT 1 FROM %1 WHERE id_reportEcho = :id").arg(table)))
        return false;
    check.bindValue(":id", idReport);
    if (!check.exec()) {
        qCritical(logCritical()) << "Image existence query:" << check.lastError().text();
        return false;
    }
    const bool exists = check.next();
    check.finish();

    QSqlQuery q(database);

    if (exists) {
        q.prepare(QString::fromUtf8(R"(
            UPDATE %1 SET
                id_orderEcho = :id_orderEcho,
                patient_id  = :patient_id,
                image_1      = :image_1,
                image_2      = :image_2,
                image_3      = :image_3,
                image_4      = :image_4,
                image_5      = :image_5,
                comment_1    = :comment_1,
                comment_2    = :comment_2,
                comment_3    = :comment_3,
                comment_4    = :comment_4,
                comment_5    = :comment_5,
                id_user      = :id_user
             WHERE
                id_reportEcho = :id_reportEcho
        )").arg(table));
    } else {
        q.prepare(QString::fromUtf8(R"(
            INSERT INTO %1 (
                id_reportEcho,
                id_orderEcho,
                patient_id,
                image_1,
                image_2,
                image_3,
                image_4,
                image_5,
                comment_1,
                comment_2,
                comment_3,
                comment_4,
                comment_5,
                id_user,
                uuid
            ) VALUES (
                :id_reportEcho,
                :id_orderEcho,
                :patient_id,
                :image_1,
                :image_2,
                :image_3,
                :image_4,
                :image_5,
                :comment_1,
                :comment_2,
                :comment_3,
                :comment_4,
                :comment_5,
                :id_user,
                :uuid
            )
        )").arg(table));
    }

    q.bindValue(":id_reportEcho", idReport);
    q.bindValue(":id_orderEcho",  idOrder);
    q.bindValue(":patient_id",   idPatient);

    q.bindValue(":image_1", blobOrNull(rows_items_images[0].bArray));
    q.bindValue(":image_2", blobOrNull(rows_items_images[1].bArray));
    q.bindValue(":image_3", blobOrNull(rows_items_images[2].bArray));
    q.bindValue(":image_4", blobOrNull(rows_items_images[3].bArray));
    q.bindValue(":image_5", blobOrNull(rows_items_images[4].bArray));

    q.bindValue(":comment_1", textOrNull(rows_items_images[0].comment_img));
    q.bindValue(":comment_2", textOrNull(rows_items_images[1].comment_img));
    q.bindValue(":comment_3", textOrNull(rows_items_images[2].comment_img));
    q.bindValue(":comment_4", textOrNull(rows_items_images[3].comment_img));
    q.bindValue(":comment_5", textOrNull(rows_items_images[4].comment_img));

    q.bindValue(":id_user", idUser);
    if (!exists)
        q.bindValue(":uuid", QUuid::createUuid().toRfc4122(), QSql::Binary);

    if (!q.exec()) {
        qCritical(logCritical())
            << "saveData() error:" << q.lastError().text();
        qCritical(logCritical()).noquote()
            << "SQL:" << q.lastQuery();
        return false;
    }

    return true;
}

QString ReportPageImage::concluzionText() const
{
    return QString();
}

ReportSections::ReportSystem ReportPageImage::system() const
{
    return ReportSections::ReportSystem::Images;
}

void ReportPageImage::onAddImage()
{
    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        if (report->documentModified() ||
            report->documentIsNew()) {
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Atașarea imaginei"));
            msg.setDetailedText(tr("Imaginea poate fi atașată după validarea documentului."));
            msg.exec();
            return;
        }
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst());

    dialog.close();
}

void ReportPageImage::onEditImage()
{
    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        if (report->documentModified() ||
            report->documentIsNew()) {
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Atașarea imaginei"));
            msg.setDetailedText(tr("Imaginea poate fi atașată după validarea documentului."));
            msg.exec();
            return;
        }
    }

    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Selectează imaginea"),
        QString(),
        tr("Imagini (*.png *.jpg *.jpeg *.bmp *.gif *.tif *.tiff *.webp)")
        );

    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

void ReportPageImage::onDeleteImage()
{
    /** Determinam nr.tab-ului curent */
    const int idx = ui->tabWidget->currentIndex() + 1;

    /** Gasim randul necesar */
    auto it = std::find_if(rows_items_images.begin(),
                           rows_items_images.end(),
                           [idx](const StructImg &r)
                           {
                               return r.nr_img == idx;
                           });

    if (it == rows_items_images.end())
        return;

    auto &r = *it;

    /** Verificam daca exista imagine */
    const bool hadImage = !r.bArray.isEmpty();

    /** Stergem imaginea din memorie */
    r.bArray.clear();

    /** Stergem imaginea din QLabel */
    r.label_img->clear();
    r.label_img->setText(tr("Nu este setată imaginea"));

    /** Optional: stergem si comentariul */
    if (r.comment_img)
        r.comment_img->clear();

    /** Actualizam contorul doar daca era imagine */
    if (hadImage && countImages > 0)
        emit countImagesBtn(--countImages);
}

void ReportPageImage::onSaveImage()
{
    const int idx = ui->tabWidget->currentIndex() + 1;

    auto it = std::find_if(rows_items_images.begin(),
                           rows_items_images.end(),
                           [idx](const StructImg &r)
                           {
                               return r.nr_img == idx;
                           });

    if (it == rows_items_images.end())
        return;

    const auto &r = *it;

    if (r.bArray.isEmpty()) {
        QMessageBox::information(this,
                                 QGuiApplication::applicationDisplayName(),
                                 tr("Nu există imagine pentru salvare."));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Salvează imaginea"),
        QDir::homePath() + QString("/image_%1.jpg").arg(idx),
        tr("JPEG Image (*.jpg *.jpeg);;PNG Image (*.png)")
        );

    if (fileName.isEmpty())
        return;

    QImage image;
    if (!image.loadFromData(r.bArray)) {
        QMessageBox::warning(this,
                             QGuiApplication::applicationDisplayName(),
                             tr("Imaginea din memorie nu a putut fi decodificată."));
        return;
    }

    QString format = "JPG";
    const QString lower = QFileInfo(fileName).suffix().toLower();

    if (lower == "png")
        format = "PNG";
    else if (lower == "jpg" || lower == "jpeg")
        format = "JPG";

    if (!image.save(fileName, format.toUtf8().constData(), 90)) {
        QMessageBox::warning(this,
                             QGuiApplication::applicationDisplayName(),
                             tr("Nu s-a putut salva imaginea:\n%1")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return;
    }

    emit showPopUp(tr("Imaginea a fost salvată cu succes:\n%1")
                       .arg(QDir::toNativeSeparators(fileName)));
}

void ReportPageImage::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this,
                                 QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                     .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return;
    }

    /** Redimensionăm și transformăm imaginea - scalarea pentru baza de date (calitatea imaginei) */
    QImage scaledImage = newImage.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    /** Conversie imagine în QByteArray (JPEG 90%) */
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    if (!scaledImage.save(&buffer, "JPEG", 90)) { // daca 'PNG' -> se mareste de 2-3 ori
        QMessageBox::warning(this,                // dimensiunea fisierului + 90% calitatea
                             QGuiApplication::applicationDisplayName(),
                             tr("Nu s-a putut converti imaginea în format JPEG."));
        return;
    }

    /** Afișăm imaginea în interfață */
    QPixmap pixmap;
    if (!pixmap.loadFromData(imageData, "JPEG")) {
        QMessageBox::warning(this,
                             QGuiApplication::applicationDisplayName(),
                             tr("Imaginea procesată nu a putut fi încărcată în memorie."));
        return;
    }

    /** Determinam nr.tab-lui */
    const int idx = ui->tabWidget->currentIndex() + 1;

    /** gasim randul necesar dupa conditia */
    auto it = std::find_if(rows_items_images.begin(),
                           rows_items_images.end(),
                           [idx](const StructImg &r)
                           {
                               return r.nr_img == idx;
                           });

    /** daca it ajuns pana la sfarsit ne oprim */
    if (it == rows_items_images.end())
        return;

    /** setam imaginea */
    auto &r = *it;
    r.label_img->setPixmap(pixmap.scaled(640,400, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    /** completam ByteArray pu comoditatea salvarii in BD */
    const bool wasEmpty = r.bArray.isEmpty();
    r.bArray = imageData;

    /** emitem signal pu instalarea nr.imagini in buton */
    if (wasEmpty)
        emit countImagesBtn(++countImages);
}

bool ReportPageImage::setPixmapBase64(QLabel *label,
                                      const QByteArray &base64,
                                      const QSize &targetSize)
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

    return true;
}

bool ReportPageImage::existDocument(const int idReport)
{
    if (idReport <= 0)
        return false;

    QSqlQuery q(m_currentDB);
    q.prepare(R"(
        SELECT 1
          FROM imagesReports
         WHERE id_reportEcho = :id_reportEcho
         LIMIT 1
    )");
    q.bindValue(":id_reportEcho", idReport);

    if (!q.exec()) {
        qCritical(logCritical()) << "existDocument() error:" << q.lastError().text();
        return false;
    }

    return q.next();
}
