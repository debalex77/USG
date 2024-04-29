#include "docreportecho.h"
#include "ui_docreportecho.h"
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else

#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QScreen>
#include <QStandardPaths>

#include <catalogs/catforsqltablemodel.h>
#endif

DocReportEcho::DocReportEcho(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocReportEcho)
{
    ui->setupUi(this);

    setWindowTitle(tr("Raport ecografic %1").arg("[*]"));

    // ******************************************************************
    db        = new DataBase(this);
    popUp     = new PopUp(this);
    timer     = new QTimer(this);                       // alocam memoria
    completer = new QCompleter(this);
    modelPatients = new QStandardItemModel(completer);
    proxyPatient  = new BaseSortFilterProxyModel(this);

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
    initConnections();  // initiem conectarile
    connections_tags();

    // ******************************************************************
    ui->gynecology_text_date_menstruation->setText("");

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    this->resize(1350, 800);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);

#if defined(Q_OS_WIN)
    ui->frame_btn->setStyle(style_fusion);
    ui->frame_table->setStyle(style_fusion);
#endif

    ui->frame_table->resize(900, ui->frame_btn->height());
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
    delete db;
    delete ui;
}

// *******************************************************************
// **************** VALIDAREA SI PRINTAREA DIN ALTE OBIECTE***********

void DocReportEcho::onPrintDocument(const int _typeReport)
{
    onPrint(_typeReport);
    this->close();
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

// *******************************************************************
// **************** ATASAREA IMAGINELOR ******************************

bool DocReportEcho::loadFile(const QString &fileName, const int numberImage)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Nu este setată imaginea %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    // convertam imaginea
    QDir dir;
    QString converted_file_name = dir.toNativeSeparators(QDir::tempPath() + "/USG_report_" + QString::number(m_id) + "_" + QString::number(numberImage));
    QPixmap::fromImage(newImage).scaled(600,450).save(converted_file_name, "jpeg", 70);

    if (numberImage == n_image1){        
        ui->image1->setPixmap(QPixmap(converted_file_name));
        setCountImages(m_count_images + 1);
    }
    else if (numberImage == n_image2){
        ui->image2->setPixmap(QPixmap(converted_file_name));
        setCountImages(m_count_images + 1);
    }
    else if (numberImage == n_image3){
        ui->image3->setPixmap(QPixmap(converted_file_name));
        setCountImages(m_count_images + 1);
    }
    else if (numberImage == n_image4){
        ui->image4->setPixmap(QPixmap(converted_file_name));
        setCountImages(m_count_images + 1);
    }
    else if (numberImage == n_image5){
        ui->image5->setPixmap(QPixmap(converted_file_name));
        setCountImages(m_count_images + 1);
    }

    if (! QFile(globals::pathImageBaseAppSettings).exists() && globals::thisSqlite){
        QMessageBox::information(this,
                                 tr("Verificarea setărilor"),
                                 tr("Imaginea nu este salvată în baza de date !!!<br>"
                                    "Nu este indicată localizarea bazei de date a imaginilor.<br>"
                                    "Deschideți setările aplicației și indicați drumul spre baza de date a imaginilor."),
                                 QMessageBox::Ok);
        qInfo(logInfo()) << tr("Imaginea nu este salvata. Nu este indicat drumul spre baza de date a imaginilor.");
        return false;
    }

    if (db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id), (globals::thisMySQL) ? db->getDatabase() : db->getDatabaseImage()))
        updateImageIntoTableimagesReports(converted_file_name, numberImage);
    else
        insertImageIntoTableimagesReports(converted_file_name, numberImage);

    QString str_cmd;
#if defined(Q_OS_LINUX)
    str_cmd = "rm -f " + converted_file_name;
#elif defined(Q_OS_MACOS)
    str_cmd = "rm -f " + converted_file_name;
#elif defined(Q_OS_WIN)
    str_cmd = "del /f " + converted_file_name;
#endif
    try {
        system(str_cmd.toStdString().c_str());
    } catch (...) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Eliminarea imaginei."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Eliminarea imaginei din directoriul temporar nu s-a efectuat !!!"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        throw;
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
    QSqlQuery qry((globals::thisMySQL) ? db->getDatabase() : db->getDatabaseImage());
    qry.prepare(QString("SELECT * FROM imagesReports WHERE id_reportEcho = :id;"));
    qry.bindValue(":id", m_id);
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
                ui->image1->setPixmap(outPixmap1.scaled(600,450));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap2 = QPixmap();
            if (! outByteArray2.isEmpty() && outPixmap2.loadFromData(outByteArray2, "jpeg")){
                ui->image2->setPixmap(outPixmap2.scaled(600,450));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap3 = QPixmap();
            if (! outByteArray3.isEmpty() && outPixmap3.loadFromData(outByteArray3)){
                ui->image3->setPixmap(outPixmap3.scaled(600,450));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap4 = QPixmap();
            if (! outByteArray4.isEmpty() && outPixmap4.loadFromData(outByteArray4)){
                ui->image4->setPixmap(outPixmap4.scaled(600,450));
                setCountImages(m_count_images + 1);
            }
            QPixmap outPixmap5 = QPixmap();
            if (! outByteArray5.isEmpty() && outPixmap5.loadFromData(outByteArray5)){
                ui->image5->setPixmap(outPixmap5.scaled(600,450));
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

    if (m_id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca imaginea este necesar de validat documentul.<br>Doriți să validați documentul ?"),
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

    if (m_id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca imaginea este necesar de validat documentul.<br>Doriți să validați documentul ?"),
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

    if (m_id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca imaginea este necesar de validat documentul.<br>Doriți să validați documentul ?"),
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

    if (m_id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca imaginea este necesar de validat documentul.<br>Doriți să validați documentul ?"),
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

    if (m_id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca imaginea este necesar de validat documentul.<br>Doriți să validați documentul ?"),
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

void DocReportEcho::insertImageIntoTableimagesReports(const QString &fileName, const int numberImage)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)){
        qWarning(logWarning()) << tr("Eroare la deschiderea fisierului '%1'.").arg(fileName);
        return;
    }
    QByteArray inByteArray = file.readAll();

    QByteArray inByteArray_image1;
    QByteArray inByteArray_image2;
    QByteArray inByteArray_image3;
    QByteArray inByteArray_image4;
    QByteArray inByteArray_image5;

    switch (numberImage) {
    case n_image1:
        inByteArray_image1 = inByteArray;
        break;
    case n_image2:
        inByteArray_image2 = inByteArray;
        break;
    case n_image3:
        inByteArray_image3 = inByteArray;
        break;
    case n_image4:
        inByteArray_image4 = inByteArray;
        break;
    case n_image5:
        inByteArray_image5 = inByteArray;
        break;
    default:;
        break;
    }

    QSqlQuery qry(db->getDatabaseImage());
    qry.prepare("INSERT INTO imagesReports ("
                "id_reportEcho,"
                "id_orderEcho,"
                "id_patients,"
                "image_1,"
                "image_2,"
                "image_3,"
                "image_4,"
                "image_5,"
                "comment_1,"
                "comment_2,"
                "comment_3,"
                "comment_4,"
                "comment_5,"
                "id_user) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    qry.addBindValue(m_id);
    qry.addBindValue(m_id_docOrderEcho);
    qry.addBindValue(m_idPacient);
    qry.addBindValue((inByteArray_image1.isEmpty()) ? QVariant() : inByteArray_image1.toBase64());
    qry.addBindValue((inByteArray_image2.isEmpty()) ? QVariant() : inByteArray_image2.toBase64());
    qry.addBindValue((inByteArray_image3.isEmpty()) ? QVariant() : inByteArray_image3.toBase64());
    qry.addBindValue((inByteArray_image4.isEmpty()) ? QVariant() : inByteArray_image4.toBase64());
    qry.addBindValue((inByteArray_image5.isEmpty()) ? QVariant() : inByteArray_image5.toBase64());
    qry.addBindValue((ui->comment_image1->toPlainText().isEmpty()) ? QVariant() : ui->comment_image1->toPlainText());
    qry.addBindValue((ui->comment_image2->toPlainText().isEmpty()) ? QVariant() : ui->comment_image2->toPlainText());
    qry.addBindValue((ui->comment_image3->toPlainText().isEmpty()) ? QVariant() : ui->comment_image3->toPlainText());
    qry.addBindValue((ui->comment_image4->toPlainText().isEmpty()) ? QVariant() : ui->comment_image4->toPlainText());
    qry.addBindValue((ui->comment_image5->toPlainText().isEmpty()) ? QVariant() : ui->comment_image5->toPlainText());
    qry.addBindValue(m_idUser);
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
        qInfo(logInfo()) << tr("A fost salvata imaginea %1 cu succes in baza de date 'BD_IMAGE'.").arg(numberImage);
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Inserarea imaginei."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Imaginea nu a fost salvată în baza de date !!!"));
        msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
    file.close();
}

void DocReportEcho::updateImageIntoTableimagesReports(const QString &fileName, const int numberImage)
{
    QString m_comment;
    switch (numberImage) {
    case n_image1:
        m_comment = ui->comment_image1->toPlainText();
        break;
    case n_image2:
        m_comment = ui->comment_image2->toPlainText();
        break;
    case n_image3:
        m_comment = ui->comment_image3->toPlainText();
        break;
    case n_image4:
        m_comment = ui->comment_image4->toPlainText();
        break;
    case n_image5:
        m_comment = ui->comment_image5->toPlainText();
        break;
    default:;
        break;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)){
        qWarning(logWarning()) << tr("Eroare la deschiderea fisierului '%1'.").arg(fileName);
        return;
    }
    QByteArray inByteArray = file.readAll();

    QSqlQuery qry(db->getDatabaseImage());
    qry.prepare("UPDATE imagesReports SET "
                "image_" + QString::number(numberImage) + " = :img,"
                "comment_" + QString::number(numberImage) + " = :comment "
                " WHERE id_reportEcho = :id_reportEcho;");
    qry.bindValue(":id_reportEcho", m_id);
    qry.bindValue(":img",     inByteArray.toBase64());
    qry.bindValue(":comment", (m_comment.isEmpty()) ? QVariant() : m_comment);
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
    file.close();
}

void DocReportEcho::removeImageIntoTableimagesReports(const int numberImage)
{
    if (m_id == idx_unknow)
        return;

    if (! db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id), db->getDatabaseImage()))
        return;

    QSqlQuery qry(db->getDatabaseImage());
    qry.prepare(QString("UPDATE imagesReports SET "
                        "image_" + QString::number(numberImage) + " = '',"
                        "comment_" + QString::number(numberImage) + " = '' "
                        " WHERE id_reportEcho = '" + QString::number(m_id) + "';"));
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea a fost eliminată cu succes din baza de date."));
        popUp->show();
        qInfo(logInfo()) << tr("Imaginea %1 a fost eliminată cu succes din baza de date 'BD_IMAGE'.").arg(numberImage);
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Eliminarea imaginei."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Imaginea nu a fost eliminată în baza de date !!!"));
        msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
}

void DocReportEcho::updateCommentIntoTableimagesReports()
{
    if (m_id == idx_unknow)
        return;

    if (! db->existIdDocument("imagesReports", "id_reportEcho", QString::number(m_id), db->getDatabaseImage()))
        return;

    QSqlQuery qry(db->getDatabaseImage());
    qry.prepare(QString("UPDATE imagesReports SET "
                        "comment_1 = '" + ui->comment_image1->toPlainText() + "',"
                        "comment_2 = '" + ui->comment_image2->toPlainText() + "',"
                        "comment_3 = '" + ui->comment_image3->toPlainText() + "',"
                        "comment_4 = '" + ui->comment_image4->toPlainText() + "',"
                        "comment_5 = '" + ui->comment_image5->toPlainText() + "' "
                        " WHERE id_reportEcho = '" + QString::number(m_id) + "';"));
    if (! qry.exec()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Inserarea comentariului."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Comentariul nu a fost inserat/actualizat în baza de date !!!"));
        msgBox.setDetailedText((qry.lastError().text().isEmpty()) ? "unknow" : qry.lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStyleSheet("QPushButton{width:120px;}");
        msgBox.exec();
    }
}

// *******************************************************************
// **************** INITIEREA CONEXIUNILOR SI PROCESAREA LOR *********

void DocReportEcho::initConnections()
{
    ui->btnParameters->setMouseTracking(true);
    ui->btnParameters->installEventFilter(this);
    ui->btnHistory->setMouseTracking(true);
    ui->btnHistory->installEventFilter(this);
    ui->btnOpenCatPatient->setMouseTracking(true);
    ui->btnOpenCatPatient->installEventFilter(this);
    ui->btnOpenDocErderEcho->setMouseTracking(true);
    ui->btnOpenDocErderEcho->installEventFilter(this);

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

    connect(ui->btn_add_template_organsInternal, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_urinary_system, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_prostate, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gynecology, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_breast, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_thyroid, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gestation0, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);
    connect(ui->btn_add_template_gestation1, &QAbstractButton::clicked, this , &DocReportEcho::clickBtnAddConcluzionTemplates);

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
    if (m_idPacient == idx_unknow)
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
    if (m_id_docOrderEcho == idx_unknow)
        return;
    DocOrderEcho* docOrder = new DocOrderEcho(this);
    docOrder->setAttribute(Qt::WA_DeleteOnClose);
    docOrder->setProperty("itNew", false);
    docOrder->setProperty("Id", m_id_docOrderEcho);
    docOrder->setGeometry(250, 150, 1400, 800);
    docOrder->show();
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

void DocReportEcho::createMenuPrint()
{
    if (globals::showDesignerMenuPrint) {
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
    onPrint(MethodOpenReport::openDesigner);
}

void DocReportEcho::clickOpenPreviewReport()
{
    onPrint(MethodOpenReport::openPreview);
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
                tr("Concluzia <b>%1</b> există ca șablon.<br>Doriți să prelungiți validarea ?").arg(str),
                    QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            msgBox.setButtonText(QMessageBox::Yes, tr("Da"));
            msgBox.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//            msgBox.addButton(tr("Da"), QMessageBox::YesRole);
//            msgBox.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
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
                        "%1) VALUES (?, ?, ?, ?, ?);").arg((globals::thisMySQL) ? "`system`" : "system"));
    qry.addBindValue(db->getLastIdForTable("conclusionTemplates") + 1);
    qry.addBindValue(0);
    qry.addBindValue(db->getLastIdForTable("conclusionTemplates") + 1);
    qry.addBindValue(str);
    qry.addBindValue(str_system);
    if (qry.exec()){
        popUp->setPopupText(tr("Șablonul adăugat cu succes<br>"
                               "în baza de date."));
            popUp->show();
    } else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Adaugarea șablonului"));
        msgBox.setText(tr("Șablonul - <b>%1</b> - nu a fost adaugat.").arg(str));
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
    connect(template_concluzion, &CatForSqlTableModel::mSelectData, this, [=]()
            {
                QString str = template_concluzion->getSelectName();

                if (ui->stackedWidget->currentIndex() == page_organs_internal)
                    ui->organsInternal_concluzion->setPlainText((ui->organsInternal_concluzion->toPlainText().isEmpty()) ? str : ui->organsInternal_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_urinary_system)
                    ui->urinary_system_concluzion->setPlainText((ui->urinary_system_concluzion->toPlainText().isEmpty()) ? str : ui->urinary_system_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_prostate)
                    ui->prostate_concluzion->setPlainText((ui->prostate_concluzion->toPlainText().isEmpty()) ? str : ui->prostate_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_gynecology)
                    ui->gynecology_concluzion->setPlainText((ui->gynecology_concluzion->toPlainText().isEmpty()) ? str : ui->gynecology_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_breast)
                    ui->breast_concluzion->setPlainText((ui->breast_concluzion->toPlainText().isEmpty()) ? str : ui->breast_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_thyroid)
                    ui->thyroid_concluzion->setPlainText((ui->thyroid_concluzion->toPlainText().isEmpty()) ? str : ui->thyroid_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_gestation0)
                    ui->gestation0_concluzion->setPlainText((ui->gestation0_concluzion->toPlainText().isEmpty()) ? str : ui->gestation0_concluzion->toPlainText() + " " + str);
                else if (ui->stackedWidget->currentIndex() == page_gestation1)
                    ui->gestation1_concluzion->setPlainText((ui->gestation1_concluzion->toPlainText().isEmpty()) ? str : ui->gestation1_concluzion->toPlainText() + " " + str);
            });
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

    QList<QLineEdit*> list = ui->stackedWidget->widget(page_organs_internal)->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
    }
    ui->organsInternal_concluzion->setPlaceholderText(tr("... maximum 500 caractere"));
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, &DocReportEcho::dataWasModified);
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->organsInternal_concluzion->toPlainText().length() > 500)
                    ui->organsInternal_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->organsInternal_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_organs_internal.isEmpty())
            str_concluzion_organs_internal = ui->concluzion->toPlainText();
        ui->concluzion->setPlainText(ui->organsInternal_concluzion->toPlainText());
    });
    ui->organsInternal_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->organsInternal_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_organs_internal()
{
    QList<QLineEdit*> list = ui->stackedWidget->widget(page_organs_internal)->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &DocReportEcho::dataWasModified);
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
    connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->urinary_system_concluzion->toPlainText().length() > 500)
                    ui->urinary_system_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->urinary_system_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_urinary_system.isEmpty()){
            str_concluzion_urinary_system = ui->concluzion->toPlainText();
            if (str_concluzion_urinary_system.isEmpty())
                str_concluzion_urinary_system = "null";
        }
        if (str_concluzion_urinary_system == "null")
            ui->concluzion->setPlainText(ui->urinary_system_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_urinary_system + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->urinary_system_concluzion->toPlainText());
    });
    ui->urinary_system_recommendation->setPlaceholderText(tr("...maximum 255 caractere"));
    ui->urinary_system_recommendation->setMaxLength(255);
}

void DocReportEcho::disconnections_urinary_system()
{
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
    connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->prostate_concluzion->toPlainText().length() > 500)
                    ui->prostate_concluzion->textCursor().deleteChar();
            });
    connect(ui->prostate_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_prostate.isEmpty()){
            str_concluzion_prostate = ui->concluzion->toPlainText();
            if (str_concluzion_prostate.isEmpty())
                str_concluzion_prostate = "null";
        }
        if (str_concluzion_prostate == "null")
            ui->concluzion->setPlainText(ui->prostate_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_prostate + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->prostate_concluzion->toPlainText());
    });
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
    connect(ui->gynecology_uterus_formations, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->gynecology_uterus_formations->toPlainText().length() > 500)
                    ui->gynecology_uterus_formations->textCursor().deletePreviousChar();
            });
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
    ui->gynecology_ovary_formations_left->setPlaceholderText(tr("... maximum 300 caractere"));
    connect(ui->gynecology_ovary_formations_left, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->gynecology_ovary_formations_left->toPlainText().length() > 300)
                    ui->gynecology_ovary_formations_left->textCursor().deletePreviousChar();
            });
    ui->gynecology_ovary_formations_right->setPlaceholderText(tr("... maximum 300 caractere"));
    connect(ui->gynecology_ovary_formations_right, &QPlainTextEdit::textChanged, this, [=]()
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
    connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->gynecology_concluzion->toPlainText().length() > 500)
                    ui->gynecology_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gynecology_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_gynecology.isEmpty()){
            str_concluzion_gynecology = ui->concluzion->toPlainText();
            if (str_concluzion_gynecology.isEmpty())
                str_concluzion_gynecology = "null";
        }
        if (str_concluzion_gynecology == "null")
            ui->concluzion->setPlainText(ui->gynecology_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_gynecology + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->gynecology_concluzion->toPlainText());
    });
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
    connect(ui->breast_left_formations, &QPlainTextEdit::textChanged, this, [=]()
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
    connect(ui->breast_right_formations, &QPlainTextEdit::textChanged, this, [=]()
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
    connect(ui->breast_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_brest.isEmpty()){
            str_concluzion_brest = ui->concluzion->toPlainText();
            if (str_concluzion_brest.isEmpty())
                str_concluzion_brest = "null";
        }
        if (str_concluzion_brest == "null")
            ui->concluzion->setPlainText(ui->breast_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_brest + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->breast_concluzion->toPlainText());
    });
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
    connect(ui->thyroid_formations, &QPlainTextEdit::textChanged, this, [=]()
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
    connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->thyroid_concluzion->toPlainText().length() > 500)
                    ui->thyroid_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->thyroid_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_thyroid.isEmpty()){
            str_concluzion_thyroid = ui->concluzion->toPlainText();
            if (str_concluzion_thyroid.isEmpty())
                str_concluzion_thyroid = "null";
        }
        if (str_concluzion_thyroid == "null")
            ui->concluzion->setPlainText(ui->thyroid_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_thyroid + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->thyroid_concluzion->toPlainText());
    });
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
    ui->gestation0_gestation->setInputMask("DDs. Dz.");
    ui->gestation0_GS_age->setInputMask("DDs. Dz.");
    ui->gestation0_CRL_age->setInputMask("DDs. Dz.");

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
    connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->gestation0_concluzion->toPlainText().length() > 500)
                    ui->gestation0_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gestation0_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_gestation0.isEmpty()){
            str_concluzion_gestation0 = ui->concluzion->toPlainText();
            if (str_concluzion_gestation0.isEmpty())
                str_concluzion_gestation0 = "null";
        }
        if (str_concluzion_gestation0 == "null")
            ui->concluzion->setPlainText(ui->gestation0_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_gestation0 + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->gestation0_concluzion->toPlainText());
    });

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
    ui->gestation1_gestation->setInputMask("DDs. Dz.");
    ui->gestation1_CRL_age->setInputMask("DDs. Dz.");
    ui->gestation1_BPD_age->setInputMask("DDs. Dz.");
    ui->gestation1_FL_age->setInputMask("DDs. Dz.");

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
    connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged, this, [=]()
            {
                if (ui->gestation1_concluzion->toPlainText().length() > 500)
                    ui->gestation1_concluzion->textCursor().deletePreviousChar();
            });
    connect(ui->gestation1_concluzion, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (str_concluzion_gestation1.isEmpty()){
            str_concluzion_gestation1 = ui->concluzion->toPlainText();
            if (str_concluzion_gestation1.isEmpty())
                str_concluzion_gestation1 = "null";
        }
        if (str_concluzion_gestation1 == "null")
            ui->concluzion->setPlainText(ui->gestation1_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_gestation1 + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->gestation1_concluzion->toPlainText());
    });

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
    ui->gestation2_gestation_age->setInputMask("DDs. Dz.");
    ui->gestation2_bpd_age->setInputMask("DDs. Dz.");
    ui->gestation2_hc_age->setInputMask("DDs. Dz.");
    ui->gestation2_ac_age->setInputMask("DDs. Dz.");
    ui->gestation2_fl_age->setInputMask("DDs. Dz.");
    ui->gestation2_fetus_age->setInputMask("DDs. Dz.");

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

    connect(ui->gestation2_comment, &QPlainTextEdit::textChanged, this, [=](){
        if (ui->gestation2_comment->toPlainText().length() > 250)
            ui->gestation2_comment->textCursor().deletePreviousChar();
    });

    connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged, this, [=](){
        if (ui->gestation2_concluzion->toPlainText().length() > 500)
            ui->gestation2_concluzion->textCursor().deletePreviousChar();
    });

    connect(ui->gestation2_concluzion, &QPlainTextEdit::textChanged, this, [this](){
        if (str_concluzion_gestation2.isEmpty()){
            str_concluzion_gestation2 = ui->concluzion->toPlainText();
            if (str_concluzion_gestation2.isEmpty())
                str_concluzion_gestation2 = "null";
        }
        if (str_concluzion_gestation2 == "null")
            ui->concluzion->setPlainText(ui->gestation2_concluzion->toPlainText());
        else
            ui->concluzion->setPlainText(str_concluzion_gestation2 + ((ui->concluzion->toPlainText().isEmpty()) ? "":"\n") + ui->gestation2_concluzion->toPlainText());
    });

    connect(group_btn_gestation2, QOverload<int>::of(&QButtonGroup::idClicked),
            this, QOverload<int>::of(&DocReportEcho::clickedGestation2Trimestru));

    connect(ui->gestation2_nasalFold, &QLineEdit::editingFinished, this, [=]() {
        ui->tabWidget_morfology->setCurrentIndex(1); // SNC
    });

    connect(ui->gestation2_vertebralColumn_description, &QLineEdit::editingFinished, this, [=](){
        ui->tabWidget_morfology->setCurrentIndex(2); // heart
    });

    connect(ui->gestation2_diaphragm_description, &QLineEdit::editingFinished, this, [=](){
        ui->tabWidget_morfology->setCurrentIndex(4); // abdomen
    });

    connect(ui->gestation2_intestine_description, &QLineEdit::editingFinished, this, [=](){
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
// **************** PROCESAREA SLOT-URILOR ***************************

void DocReportEcho::slot_ItNewChanged()
{
    if (m_itNew){

        setWindowTitle(tr("Raport ecografic (crearea) %1").arg("[*]"));

        connect(timer, &QTimer::timeout, this, &DocReportEcho::updateTimer);
        timer->start(1000);                    // inițierea timerului pu data docum.

        setIdUser(globals::idUserApp);

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
        if (m_gestation0)
            connections_gestation0();
        if (m_gestation1)
            connections_gestation1();
        if (m_gestation2) {
            ui->gestation2_dateMenstruation->setDate(QDate::currentDate());
            connections_gestation2();
        }

        ui->comment->setHidden(true);
    }
    initEnableBtn();
    updateStyleBtnInvestigations();
}

void DocReportEcho::slot_IdChanged()
{
    if (m_id == idx_unknow)
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
        if (globals::thisMySQL){
            QString str_date = items.constFind("dateDoc").value();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            str_date = str_date.replace(QRegExp("T"), " ").replace(".000","");
#else
            str_date = str_date.replace(QRegularExpression("T"), " ").replace(".000","");
#endif
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
            setDataFromTableLiver();
            setDataFromTableCholecist();
            setDataFromTablePancreas();
            setDataFromTableSpleen();
            connections_organs_internal();    // conectam modificarea formei la modificarea textului
        }

        if (m_urinary_system){
            disconnections_urinary_system();
            setDataFromTableKidney();
            setDataFromTableBladder();
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
    if (m_idPacient == idx_unknow)
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
    if (m_id_docOrderEcho == idx_unknow)
        return;

    QMap<QString, QString> items;
    if (db->getObjectDataById("orderEcho", m_id_docOrderEcho, items)){
        if (globals::thisMySQL){
            QString str_date = items.constFind("dateDoc").value();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            str_date = str_date.replace(QRegExp("T"), " ").replace(".000","");
#else
            str_date = str_date.replace(QRegularExpression("T"), " ").replace(".000","");
#endif
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
        if (m_idPacient == idx_unknow)
            setIdPacient(items.constFind("id_pacients").value().toInt());
    }
}

void DocReportEcho::slot_IdUserChanged()
{
    if (m_idUser == idx_unknow)
        setIdUser(globals::idUserApp);
}

void DocReportEcho::slot_CountImagesChanged()
{
    if (m_count_images > 0)
        ui->btnImages->setText(tr("Imagini (atașate %1)").arg(QString::number(m_count_images)));
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
    if (_id == idx_unknow || _id == idx_write)     // verificarea id
        return;

    setIdPacient(index.data(Qt::UserRole).toInt()); // setam 'id' pacientului
}

// *******************************************************************
// **************** VALIDAREA, PRINTAREA DOCUMENTULUI ****************

bool DocReportEcho::controlRequiredObjects()
{
    if (m_id == idx_unknow){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> documentului !!!<br>Adresați-vă administratorului aplicației"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_idPacient == idx_unknow || m_idPacient == idx_write){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> pacientului !!!<br>Adresați-vă administratorului aplicației"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_idUser == idx_unknow || m_idUser == idx_write){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este determinat <b>'ID'</b> autorului documentului !!!<br>Adresați-vă administratorului aplicației"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (ui->editDocNumber->text().isEmpty()){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este indicat <b>'Numărul'</b> documentului !!!<br>Adresați-vă administratorului aplicației"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (ui->concluzion->toPlainText().isEmpty()){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este indicată <b>'Concluzia'</b> raportului !!!<br>Validarea nu este posibilă."),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    if (m_organs_internal && ui->organsInternal_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (organelor interne)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_urinary_system && ui->urinary_system_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (sistemului urinar)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_prostate && ui->prostate_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (prostatei)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gynecology && ui->gynecology_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (ginecologica)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_breast && ui->breast_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (gl.mamare)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_thyroide && ui->thyroid_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (gl.tiroide)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gestation0 && ui->gestation0_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (sarcina până la 11 săptămâni)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    if (m_gestation1 && ui->gestation1_concluzion->toPlainText().isEmpty()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea datelor"),
                                 tr("Nu este indicată <b>'Concluzia (sarcina 11-14 săptămâni)'</b> raportului !!!<br>Doriți să continuați validarea documentului ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        messange_box.addButton(tr("Da"), QMessageBox::YesRole);
//        messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (messange_box.exec() == QMessageBox::No)
            return false;
    }

    return true;
}

void DocReportEcho::onPrint(const int _typeReport)
{
    if (globals::pathTemplatesDocs.isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea setărilor"),
                             tr("Nu este indicat directoriul cu șabloanele de tipar.<br>"
                                "Tipărirea documentului nu este posibilă."),
                             QMessageBox::Ok);
        return;
    }

    static int exist_logo = 0;

    // *************************************************************************************
    // alocam memoria
    m_report = new LimeReport::ReportEngine(this);
    model_logo = new QStandardItemModel(1,1);
    modelPatient_print  = new QSqlQueryModel(this);

    // *************************************************************************************
    // titlu raportului
    m_report->setPreviewWindowTitle(tr("Raport ecografic nr.") + ui->editDocNumber->text() + tr(" din ") + ui->editDocDate->dateTime().toString("dd.MM.yyyy hh:mm:ss") + tr(" (printare)"));

    // *************************************************************************************
    // logotipul
    QPixmap pix_logo = QPixmap();
    QStandardItem* img_item = new QStandardItem();
    QString name_key_logo = "logo_" + globals::nameUserApp;
    if (globals::cache_img.find(name_key_logo, &pix_logo)){
        QImage m_logo = pix_logo.toImage();
        img_item->setData(m_logo.scaled(300,50), Qt::DisplayRole);
        model_logo->setItem(0, 0, img_item);
        exist_logo = 1;
    } else {
        QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", m_idUser);
        if (! outByteArray.isEmpty() && pix_logo.loadFromData(outByteArray)){
            globals::cache_img.insert(name_key_logo, pix_logo);
            QImage m_logo = pix_logo.toImage();
            img_item->setData(m_logo.scaled(300,50), Qt::DisplayRole);
            model_logo->setItem(0, 0, img_item);
            exist_logo = 1;
        }
    }
    m_report->dataManager()->addModel("table_logo", model_logo, true);

    // ************************************************************************************
    // setam datele organizatiei, pacientului in model
    if (modelOrganization->rowCount() > 0)
        modelOrganization->clear();
    modelOrganization->setQuery(db->getQryFromTableConstantById(globals::idUserApp));
    m_report->dataManager()->addModel("main_organization", modelOrganization, false);

    if (modelPatient_print->rowCount() > 0)
        modelPatient_print->clear();
    modelPatient_print->setQuery(db->getQryFromTablePatientById(m_idPacient));
    m_report->dataManager()->addModel("table_patient", modelPatient_print, false);

    m_report->dataManager()->clearUserVariables();
    m_report->dataManager()->setReportVariable("v_exist_logo", exist_logo);
    m_report->dataManager()->setReportVariable("unitMeasure", (globals::unitMeasure == "milimetru") ? "mm" : "cm");

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
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Complex.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Complex.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - complex: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - complex: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - complex: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_organs_internal && !m_urinary_system){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("unit_measure_volum", "ml");
        modelOrgansInternal->setQuery(db->getQryForTableOrgansInternalById(m_id));
        m_report->dataManager()->addModel("table_organs_internal", modelOrgansInternal, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Organs internal.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Organs internal.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - organs internal: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - organs internal: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - organs internal: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
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
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Urinary system.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Urinary system.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - urinary system: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - urinary system: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - urinary system: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
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
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Prostate.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Prostate.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - prostate: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - prostate: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - prostate: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
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
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Gynecology.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Gynecology.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - gynecology: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - gynecology: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gynecology: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_breast){

        // extragem datele si introducem in model + setam variabile
        modelBreast->setQuery(db->getQryForTableBreastById(m_id));
        m_report->dataManager()->addModel("table_breast", modelBreast, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Breast.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Breast.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - breast: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - breast: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - breast: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_thyroide){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("unit_measure_volum", "cm3");
        modelThyroid->setQuery(db->getQryForTableThyroidById(m_id));
        m_report->dataManager()->addModel("table_thyroid", modelThyroid, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Thyroid.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Thyroid.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - thyroid: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - thyroid: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - thyroid: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_gestation0){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("ivestigation_view", group_btn_gestation0->checkedId());
        modelGestationO->setQuery(db->getQryForTableGestation0dById(m_id));
        m_report->dataManager()->addModel("table_gestation0", modelGestationO, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Gestation0.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Gestation0.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - gestation0: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - gestation0: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation0: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_gestation1){

        // extragem datele si introducem in model + setam variabile
        m_report->dataManager()->setReportVariable("ivestigation_view", group_btn_gestation1->checkedId());
        modelGestation1->setQuery(db->getQryForTableGestation1dById(m_id));
        m_report->dataManager()->addModel("table_gestation1", modelGestation1, false);

        // completam sablonul
        if (! m_report->loadFromFile(globals::pathTemplatesDocs + "/Gestation1.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Gestation1.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentarea preview sau designer
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - gestation1: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview){
            qInfo(logInfo()) << tr("Printare (preview) - gestation1: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation1: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    if (m_gestation2){
        modelGestation2->setQuery(db->getQryForTableGestation2(m_id));
        m_report->dataManager()->addModel("table_gestation2", modelGestation2, false);

        // sablonul
        if (! m_report->loadFromFile((ui->gestation2_trimestru2->isChecked()) ?
                                     globals::pathTemplatesDocs + "/Gestation2.lrxml" : globals::pathTemplatesDocs + "/Gestation3.lrxml")){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Printarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Printarea documentului nu este posibila."));
            msgBox.setDetailedText(tr("Nu au fost incarcate datele in sablon - %1").arg(globals::pathTemplatesDocs + "/Gestation2.lrxml"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();

            delete img_item;

            this->show();

            return;
        }

        // prezentam
        m_report->setShowProgressDialog(true);
        if (_typeReport == openDesigner){
            qInfo(logInfo()) << tr("Printare (designer) - gestation2: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        } else if (_typeReport == openPreview) {
            qInfo(logInfo()) << tr("Printare (preview) - gestation2: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->previewReport();
        } else {
            qInfo(logInfo()) << tr("Printare (designer) - gestation2: document 'Raport ecografic' id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                                    .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
            m_report->designReport();
        }
    }

    this->show();

    // *************************************************************************************
    // elibiram memoria
    delete img_item;
    delete model_logo;
    delete modelPatient_print;
    delete m_report;
}

bool DocReportEcho::onWritingData()
{
    if (m_itNew && m_id == idx_unknow)
        m_id = db->getLastIdForTable("reportEcho") + 1; // incercare de a seta 'id' documentului

    if (! controlRequiredObjects())
        return false;

    if (m_post == idx_unknow)  // daca a fost apasat btnOk = propritatea trebuia sa fie m_post == idx_post
        setPost(idx_write);    // setam post = idx_write

    QString details_error;

    if (m_itNew){    
        if (! insertingDocumentDataIntoTables(details_error)){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Validarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Validarea documentului nu s-a efectuat."));
            msgBox.setDetailedText(details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
        }     
        if (m_post == idx_write || m_post == idx_post){
            popUp->setPopupText(tr("Documentul a fost %1 cu succes<br> in baza de date.")
                                .arg((m_post == idx_write) ? tr("salvat") : tr("validat")));
            popUp->show();
            setItNew(false);
        }
        qInfo(logInfo()) << tr("Document 'Raport ecografic' - crearea: id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                            .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
    } else {

        if (! updatingDocumentDataIntoTables(details_error)){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Actualizarea documentului"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Actualizarea datelor documentului nu s-a efectuat."));
            msgBox.setDetailedText(details_error);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QPushButton{width:120px;}");
            msgBox.exec();
            return false;
        }
        if (m_post == idx_write || m_post == idx_post){
            popUp->setPopupText(tr("Datele documentului au fost actualizate<br> cu succes."));
            popUp->show();
            setItNew(false);
        }
        qInfo(logInfo()) << tr("Document 'Raport ecografic' - modificarea: id='%1', nr.='%2', id_patient='%3', pacientul='%4'")
                            .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_idPacient), ui->comboPatient->currentText());
    }

    updateDataDocOrderEcho(); // actualizarea datelor doc.Comanda ecografica - inserarea valorii atasarii imaginei

    // imaginea se inscrie in BD - vezi functia loadFile()
    // iar comentariile la imagini la validarea documentului
    updateCommentIntoTableimagesReports();

    emit PostDocument(); // pu actualizarea listei documentelor
    return true;
}

void DocReportEcho::onWritingDataClose()
{
    setPost(idx_post); // setam proprietatea 'post'

    if (onWritingData()){
        QDialog::accept();
        emit mCloseThisForm();

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Printarea documentului"));
        msgBox.setIcon(QMessageBox::Question);
#if defined(Q_OS_LINUX)
        msgBox.setText(tr("Doriți să printați documentul ?"));
#elif defined(Q_OS_MACOS)
        msgBox.setText(tr("Doriți să printați documentul ?"));
#elif defined(Q_OS_WIN)
        msgBox.setText(tr("Dori\310\233i s\304\203 printa\310\233i documentul ?"));
#endif
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        msgBox.setButtonText(QMessageBox::Yes, tr("Da"));
        msgBox.setButtonText(QMessageBox::No, tr("Nu"));
//#else
//        msgBox.addButton(tr("Da"), QMessageBox::YesRole);
//        msgBox.addButton(tr("Nu"), QMessageBox::NoRole);
#endif
        if (msgBox.exec() == QMessageBox::Yes)
            onPrint(openPreview);
    }
}

void DocReportEcho::onClose()
{
    this->close();
    emit mCloseThisForm();
}

// *******************************************************************
// **************** INSERAREA DATELOR IMPLICITE A DOCUMENTULUI *******

void DocReportEcho::setDefaultDataTableLiver()
{
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
    if (ui->cholecist_form->text().isEmpty())
        ui->cholecist_form->setText("obisnuita");
    if (ui->cholecist_formations->text().isEmpty())
        ui->cholecist_formations->setText("abs.");
}

void DocReportEcho::setDefaultDataTablePancreas()
{
    if (ui->pancreas_ecogenity->text().isEmpty())
        ui->pancreas_ecogenity->setText("sporita");
    if (ui->pancreas_parenchyma->text().isEmpty())
        ui->pancreas_parenchyma->setText("omogena");
    if (ui->pancreas_formations->text().isEmpty())
        ui->pancreas_formations->setText("lichidiene, solide abs.");
}

void DocReportEcho::setDefaultDataTableSpleen()
{
    if (ui->spleen_contour->text().isEmpty())
        ui->spleen_contour->setText("clar");
    if (ui->spleen_parenchyma->text().isEmpty())
        ui->spleen_parenchyma->setText("omogena");
    if (ui->spleen_formations->text().isEmpty())
        ui->spleen_formations->setText("lichidiene, solide abs.");
}

void DocReportEcho::setDefaultDataKidney()
{
    if (ui->kidney_formations->text().isEmpty())
        ui->kidney_formations->setText("solide, lichide abs., sectoare hiperecogene 2-3 mm fără umbră acustică");
    if (ui->kidney_pielocaliceal_left->text().isEmpty())
        ui->kidney_pielocaliceal_left->setText("nu este dilatat");
    if (ui->kidney_pielocaliceal_right->text().isEmpty())
        ui->kidney_pielocaliceal_right->setText("nu este dilatat");
}

void DocReportEcho::setDefaultDataBladder()
{
    if (ui->bladder_formations->text().isEmpty())
        ui->bladder_formations->setText("diverticuli, calculi abs.");
}

void DocReportEcho::setDefaultDataProstate()
{
    if (ui->prostate_contur->text().isEmpty())
        ui->prostate_contur->setText("clar");
    if (ui->prostate_ecogency->text().isEmpty())
        ui->prostate_ecogency->setText("scăzută");
    if (ui->prostate_ecostructure->text().isEmpty())
        ui->prostate_ecostructure->setText("omogenă");
    if (ui->prostate_formations->text().isEmpty())
        ui->prostate_formations->setText("abs.");
    if (ui->prostate_recommendation->text().isEmpty())
        ui->prostate_recommendation->setText("consultația urologului");
}

void DocReportEcho::setDefaultDataGynecology()
{
    if (m_id == idx_unknow){
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
        ui->gynecology_recommendation->setText("consultația ginecologului");
}

void DocReportEcho::setDefaultDataBreast()
{
    if (ui->breast_right_ecostructure->text().isEmpty())
        ui->breast_right_ecostructure->setText("omogenă");
    if (ui->breast_right_duct->text().isEmpty())
        ui->breast_right_duct->setText("norm.");
    if (ui->breast_right_ligament->text().isEmpty())
        ui->breast_right_ligament->setText("norm");
    if (ui->breast_right_formations->toPlainText().isEmpty())
        ui->breast_right_formations->setPlainText("lichidiene, solide abs.");
    if (ui->breast_right_ganglions->text().isEmpty())
        ui->breast_right_ganglions->setText("fără modificări patologice");

    if (ui->breast_left_ecostructure->text().isEmpty())
        ui->breast_left_ecostructure->setText("omogenă");
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
    if (ui->thyroid_ecostructure->text().isEmpty())
        ui->thyroid_ecostructure->setText("omogenă");
    if (ui->thyroid_formations->toPlainText().isEmpty())
        ui->thyroid_formations->setPlainText("l.drept - formațiuni lichidiene, solide abs.\nl.stâng - formațiuni lichidiene, solide abs.");
    if (ui->thyroid_ganglions->text().isEmpty())
        ui->thyroid_ganglions->setText("fara modificări patologice");
    if (ui->thyroid_recommendation->text().isEmpty())
        ui->thyroid_recommendation->setText("consultația endocrinologului");
}

void DocReportEcho::setDefaultDataGestation0()
{
    if (ui->gestation0_antecedent->text().isEmpty())
        ui->gestation0_antecedent->setText("abs.");
    if (ui->gestation0_BCF->text().isEmpty())
        ui->gestation0_BCF->setText("prezenți, ritmici");
    if (ui->gestation0_liquid_amniotic->text().isEmpty())
        ui->gestation0_liquid_amniotic->setText("omogen, transparent");
    if (ui->gestation0_miometer->text().isEmpty())
        ui->gestation0_miometer->setText("omogen; formațiuni solide, lichidiene abs.");
    if (ui->gestation0_cervix->text().isEmpty())
        ui->gestation0_cervix->setText("omogen; formațiuni solide, lichidiene abs.; inchis, lungimea 32,9 mm");
    if (ui->gestation0_ovary->text().isEmpty())
        ui->gestation0_ovary->setText("aspect ecografic normal");
    if (ui->gestation0_recommendation->text().isEmpty())
        ui->gestation0_recommendation->setText("consultația ginecologului, examen ecografic la 11-12 săptămâni a sarcinei");
}

void DocReportEcho::setDefaultDataGestation1()
{
    if (ui->gestation1_antecedent->text().isEmpty())
        ui->gestation1_antecedent->setText("abs.");
    if (ui->gestation1_BCF->text().isEmpty())
        ui->gestation1_BCF->setText("prezenți, ritmici");
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
        ui->gestation1_miometer->setText("omogen; formațiuni solide, lichidiene abs.");
    if (ui->gestation1_cervix->text().isEmpty())
        ui->gestation1_cervix->setText("omogen; formațiuni solide, lichidiene abs.; inchis, lungimea 32,9 mm");
    if (ui->gestation1_ovary->text().isEmpty())
        ui->gestation1_ovary->setText("aspect ecografic normal");
    if (ui->gestation1_recommendation->text().isEmpty())
        ui->gestation1_recommendation->setText("consultația ginecologului, examen ecografic la 18-20 săptămâni a sarcinei");
}

void DocReportEcho::setDefaultDataGestation2()
{
    if (ui->gestation2_recommendation->text().isEmpty())
        ui->gestation2_recommendation->setText("consultația ginecologului");
}

// *******************************************************************
// **************** ACCESIBILITATEA BUTOANELOR ***********************

void DocReportEcho::initEnableBtn()
{
    int m_current_page = -1;

    if (m_id == idx_unknow &&
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
    if (ui->stackedWidget->currentIndex() == page_organs_internal){
        ui->btnOrgansInternal->setStyleSheet("background: 0px #C2C2C3; "
                                             "border: 1px inset blue; "
                                             "border-color: navy;");
    } else {
        ui->btnOrgansInternal->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                             "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_urinary_system){
        ui->btnUrinarySystem->setStyleSheet("background: 0px #C2C2C3; "
                                            "border: 1px inset blue; "
                                            "border-color: navy;");
    } else {
        ui->btnUrinarySystem->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                            "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_prostate){
        ui->btnProstate->setStyleSheet("background: 0px #C2C2C3; "
                                       "border: 1px inset blue; "
                                       "border-color: navy;");
    } else {
        ui->btnProstate->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                       "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_gynecology){
        ui->btnGynecology->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnGynecology->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_breast){
        ui->btnBreast->setStyleSheet("background: 0px #C2C2C3; "
                                     "border: 1px inset blue; "
                                     "border-color: navy;");
    } else {
        ui->btnBreast->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                     "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_thyroid){
        ui->btnThyroid->setStyleSheet("background: 0px #C2C2C3; "
                                      "border: 1px inset blue; "
                                      "border-color: navy;");
    } else {
        ui->btnThyroid->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                      "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_gestation0){
        ui->btnGestation0->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnGestation0->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_gestation1){
        ui->btnGestation1->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnGestation1->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_gestation2){
        ui->btnGestation2->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnGestation2->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }

    if (ui->comment->isVisible()){
        ui->btnComment->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnComment->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }

    if (ui->stackedWidget->currentIndex() == page_images){
        ui->btnImages->setStyleSheet("background: 0px #C2C2C3; "
                                         "border: 1px inset blue; "
                                         "border-color: navy;");
    } else {
        ui->btnImages->setStyleSheet("background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                         "border: 0px #8f8f91;");
    }
}

// *******************************************************************
// **************** INITIEREA - COMPLETER & FOOTER DOCUMENTULUI ******

void DocReportEcho::initSetCompleter()
{
    if (modelPatients->rowCount() > 0)
        modelPatients->clear();

    QString strQuery = QString("SELECT pacients.id, "
                               "    fullNamePacients.nameBirthdayIDNP AS FullName "
                               "FROM "
                               "    pacients "
                               "INNER JOIN "
                               "    fullNamePacients ON fullNamePacients.id_pacients = pacients.id "
                               "WHERE "
                               "    pacients.deletionMark = 0 ORDER BY FullName ASC;");
    QMap<int, QString> data = db->getMapDataQuery(strQuery);

    QMapIterator<int, QString> it(data);
    while (it.hasNext()) {
        it.next();
        int     _id   = it.key();
        QString _name = it.value();
        _name.replace("#1", "\n");

        QStandardItem *item = new QStandardItem;
        item->setData(_id,   Qt::UserRole);
        item->setData(_name, Qt::DisplayRole);

        modelPatients->appendRow(item);  // adaugam datele in model
    }

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
    labelAuthor->setText(globals::nameUserApp);
    labelAuthor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAuthor->setStyleSheet("padding-left: 3px; color: rgb(49, 151, 116);");

    ui->layoutAuthor->addWidget(labelPix);
    ui->layoutAuthor->addWidget(labelAuthor);
    ui->layoutAuthor->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA DATELOR IN BD ************

bool DocReportEcho::insertingDocumentDataIntoTables(QString &details_error)
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    try {
        qry.prepare("INSERT INTO reportEcho ("
                    "id,"
                    "deletionMark,"
                    "numberDoc,"
                    "dateDoc,"
                    "id_pacients,"
                    "id_orderEcho,"
                    "t_organs_internal,"
                    "t_urinary_system,"
                    "t_prostate,"
                    "t_gynecology,"
                    "t_breast,"
                    "t_thyroid,"
                    "t_gestation0,"
                    "t_gestation1,"
                    "t_gestation2,"
                    "t_gestation3,"
                    "id_users,"
                    "concluzion,"
                    "comment,"
                    "attachedImages) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
        qry.addBindValue(m_id);
        qry.addBindValue(m_post);
        qry.addBindValue(ui->editDocNumber->text());
        qry.addBindValue(ui->editDocDate->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
        qry.addBindValue(m_idPacient);
        qry.addBindValue(m_id_docOrderEcho);
        if (globals::thisMySQL){
            qry.addBindValue((m_organs_internal) ? true : false);
            qry.addBindValue((m_urinary_system) ? true : false);
            qry.addBindValue((m_prostate) ? true : false);
            qry.addBindValue((m_gynecology) ? true : false);
            qry.addBindValue((m_breast) ? true : false);
            qry.addBindValue((m_thyroide) ? true : false);
            qry.addBindValue((m_gestation0) ? true : false);
            qry.addBindValue((m_gestation1) ? true : false);
            qry.addBindValue((m_gestation2) ? true : false);
            qry.addBindValue((m_gestation3) ? true : false);
        } else {
            qry.addBindValue((m_organs_internal) ? 1 : 0);
            qry.addBindValue((m_urinary_system) ? 1 : 0);
            qry.addBindValue((m_prostate) ? 1 : 0);
            qry.addBindValue((m_gynecology) ? 1 : 0);
            qry.addBindValue((m_breast) ? 1 : 0);
            qry.addBindValue((m_thyroide) ? 1 : 0);
            qry.addBindValue((m_gestation0) ? 1 : 0);
            qry.addBindValue((m_gestation1) ? 1 : 0);
            qry.addBindValue((m_gestation2) ? 1 : 0);
            qry.addBindValue((m_gestation3) ? 1 : 0);
        }
        qry.addBindValue(m_idUser);
        qry.addBindValue(ui->concluzion->toPlainText());
        qry.addBindValue((ui->comment->toPlainText().isEmpty()) ? QVariant() : ui->comment->toPlainText());
        qry.addBindValue((m_count_images == 0) ? m_count_images : 1);
        if (! qry.exec())
            details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'reportEcho' - %2")
                                .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

        if (m_organs_internal) {
            //*********************************************
            // -- tableLiver

            qry.prepare("INSERT INTO tableLiver ("
                        "id_reportEcho,"
                        "`left`,"
                        "`right`,"
                        "contur,"
                        "parenchim,"
                        "ecogenity,"
                        "formations,"
                        "ductsIntrahepatic,"
                        "porta,"
                        "lienalis,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableLiver' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tableCholecist
            qry.prepare("INSERT INTO tableCholecist ("
                        "id_reportEcho,"
                        "form,"
                        "dimens,"
                        "walls,"
                        "choledoc,"
                        "formations) VALUES (?,?,?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->cholecist_form->text());
            qry.addBindValue(ui->cholecist_dimens->text());
            qry.addBindValue(ui->cholecist_walls->text());
            qry.addBindValue(ui->cholecist_coledoc->text());
            qry.addBindValue(ui->cholecist_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableCholecist' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tablePancreas
            qry.prepare("INSERT INTO tablePancreas ("
                        "id_reportEcho,"
                        "cefal,"
                        "corp,"
                        "tail,"
                        "texture,"
                        "ecogency,"
                        "formations) VALUES (?,?,?,?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->pancreas_cefal->text());
            qry.addBindValue(ui->pancreas_corp->text());
            qry.addBindValue(ui->pancreas_tail->text());
            qry.addBindValue(ui->pancreas_parenchyma->text());
            qry.addBindValue(ui->pancreas_ecogenity->text());
            qry.addBindValue(ui->pancreas_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tablePancreas' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tableSpleen
            qry.prepare("INSERT INTO tableSpleen ("
                        "id_reportEcho,"
                        "dimens,"
                        "contur,"
                        "parenchim,"
                        "formations) VALUES (?,?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->spleen_dimens->text());
            qry.addBindValue(ui->spleen_contour->text());
            qry.addBindValue(ui->spleen_parenchyma->text());
            qry.addBindValue(ui->spleen_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableSpleen' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_urinary_system) {
            //*********************************************
            // -- tableKidney

            qry.prepare("INSERT INTO tableKidney ("
                        "id_reportEcho,"
                        "dimens_right,"
                        "dimens_left,"
                        "corticomed_right,"
                        "corticomed_left,"
                        "pielocaliceal_right,"
                        "pielocaliceal_left,"
                        "formations,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->kidney_right->text());
            qry.addBindValue(ui->kidney_left->text());
            qry.addBindValue(ui->kidney_corticomed_right->text());
            qry.addBindValue(ui->kidney_corticomed_left->text());
            qry.addBindValue(ui->kidney_pielocaliceal_right->text());
            qry.addBindValue(ui->kidney_pielocaliceal_left->text());
            qry.addBindValue(ui->kidney_formations->text());
            qry.addBindValue(ui->urinary_system_concluzion->toPlainText());
            qry.addBindValue((ui->urinary_system_recommendation->text().isEmpty()) ? QVariant() : ui->urinary_system_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableKidney' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tableBladder

            qry.prepare("INSERT INTO tableBladder ("
                        "id_reportEcho,"
                        "volum,"
                        "walls,"
                        "formations) VALUES (?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->bladder_volum->text());
            qry.addBindValue(ui->bladder_walls->text());
            qry.addBindValue(ui->bladder_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableBladder' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_prostate) {
            qry.prepare("INSERT INTO tableProstate ("
                        "id_reportEcho,"
                        "dimens,"
                        "volume,"
                        "ecostructure,"
                        "contour,"
                        "ecogency,"
                        "formations,"
                        "transrectal,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?);");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->prostate_dimens->text());
            qry.addBindValue(ui->prostate_volum->text());
            qry.addBindValue(ui->prostate_ecostructure->text());
            qry.addBindValue(ui->prostate_contur->text());
            qry.addBindValue(ui->prostate_ecogency->text());
            qry.addBindValue(ui->prostate_formations->text());
            if (globals::thisMySQL)
                qry.addBindValue((ui->prostate_radioBtn_transrectal->isChecked()) ? true : false);
            else
                qry.addBindValue((ui->prostate_radioBtn_transrectal->isChecked()) ? 1 : 0);
            qry.addBindValue(ui->prostate_concluzion->toPlainText());
            qry.addBindValue((ui->prostate_recommendation->text().isEmpty()) ? QVariant() : ui->prostate_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableProstate' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gynecology) {
            qry.prepare("INSERT INTO tableGynecology ("
                        "id_reportEcho,"
                        "transvaginal,"
                        "dateMenstruation,"
                        "antecedent,"
                        "uterus_dimens,"
                        "uterus_pozition,"
                        "uterus_ecostructure,"
                        "uterus_formations,"
                        "ecou_dimens,"
                        "ecou_ecostructure,"
                        "cervix_dimens,"
                        "cervix_ecostructure,"
                        "douglas,"
                        "plex_venos,"
                        "ovary_right_dimens,"
                        "ovary_left_dimens,"
                        "ovary_right_volum,"
                        "ovary_left_volum,"
                        "ovary_right_follicle,"
                        "ovary_left_follicle,"
                        "ovary_right_formations,"
                        "ovary_left_formations,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
            qry.addBindValue(m_id);
            if (globals::thisMySQL)
                qry.addBindValue((ui->gynecology_btn_transvaginal->isChecked()) ? true : false);
            else
                qry.addBindValue((ui->gynecology_btn_transvaginal->isChecked()) ? 1 : 0);
            qry.addBindValue(ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
            qry.addBindValue((ui->gynecology_antecedent->text().isEmpty()) ? QVariant() : ui->gynecology_antecedent->text());
            qry.addBindValue(ui->gynecology_uterus_dimens->text());
            qry.addBindValue(ui->gynecology_uterus_pozition->text());
            qry.addBindValue(ui->gynecology_uterus_ecostructure->text());
            qry.addBindValue(ui->gynecology_uterus_formations->toPlainText());
            qry.addBindValue(ui->gynecology_ecou_dimens->text());
            qry.addBindValue(ui->gynecology_ecou_ecostructure->text());
            qry.addBindValue(ui->gynecology_cervix_dimens->text());
            qry.addBindValue(ui->gynecology_cervix_ecostructure->text());
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
            qry.addBindValue(ui->gynecology_concluzion->toPlainText());
            qry.addBindValue((ui->gynecology_recommendation->text().isEmpty()) ? QVariant() : ui->gynecology_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGynecology' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_breast) {
            qry.prepare("INSERT INTO tableBreast ("
                        "id_reportEcho,"
                        "breast_right_ecostrcture,"
                        "breast_right_duct,"
                        "breast_right_ligament,"
                        "breast_right_formations,"
                        "breast_right_ganglions,"
                        "breast_left_ecostrcture,"
                        "breast_left_duct,"
                        "breast_left_ligament,"
                        "breast_left_formations,"
                        "breast_left_ganglions,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableBreast' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_thyroide) {
            qry.prepare("INSERT INTO tableThyroid ("
                        "id_reportEcho,"
                        "thyroid_right_dimens,"
                        "thyroid_right_volum,"
                        "thyroid_left_dimens,"
                        "thyroid_left_volum,"
                        "thyroid_istm,"
                        "thyroid_ecostructure,"
                        "thyroid_formations,"
                        "thyroid_ganglions,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableThyroid' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation0) {
            qry.prepare("INSERT INTO tableGestation0 ("
                        "id_reportEcho,"
                        "view_examination,"
                        "antecedent,"
                        "gestation_age,"
                        "GS,"
                        "GS_age,"
                        "CRL,"
                        "CRL_age,"
                        "BCF,"
                        "liquid_amniotic,"
                        "miometer,"
                        "cervix,"
                        "ovary,"
                        "concluzion,"
                        "recommendation) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation0' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation1) {
            qry.prepare("INSERT INTO tableGestation1 ("
                        "id_reportEcho,"
                        "view_examination,"
                        "antecedent,"
                        "gestation_age,"
                        "CRL,"
                        "CRL_age,"
                        "BPD,"
                        "BPD_age,"
                        "NT,"
                        "NT_percent,"
                        "BN,"
                        "BN_percent,"
                        "BCF,"
                        "FL,"
                        "FL_age,"
                        "callote_cranium,"
                        "plex_choroid,"
                        "vertebral_column,"
                        "stomach,"
                        "bladder,"
                        "diaphragm,"
                        "abdominal_wall,"
                        "location_placenta,"
                        "sac_vitelin,"
                        "amniotic_liquid,"
                        "miometer,"
                        "cervix,"
                        "ovary,"
                        "concluzion,"
                        "recommendation) "
                        " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation1' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation2) {

            // main table
            qry.prepare("INSERT INTO tableGestation2 ("
                        "id_reportEcho, "
                        "gestation_age, "
                        "trimestru, "
                        "dateMenstruation, "
                        "view_examination, "
                        "single_multiple_pregnancy, "
                        "single_multiple_pregnancy_description, "
                        "antecedent, "
                        "comment, "
                        "concluzion, "
                        "recommendation) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // biometry
            qry.prepare("INSERT INTO tableGestation2_biometry ("
                        "id_reportEcho, "
                        "BPD, "
                        "BPD_age, "
                        "HC, "
                        "HC_age, "
                        "AC, "
                        "AC_age, "
                        "FL, "
                        "FL_age, "
                        "FetusCorresponds) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_biometry' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // cranium
            qry.prepare("INSERT INTO tableGestation2_cranium ("
                        "id_reportEcho, "
                        "calloteCranium, "
                        "facialeProfile, "
                        "nasalBones, "
                        "nasalBones_dimens, "
                        "eyeball, "
                        "eyeball_desciption, "
                        "nasolabialTriangle, "
                        "nasolabialTriangle_description, "
                        "nasalFold) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_cranium' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // SNC
            qry.prepare("INSERT INTO tableGestation2_SNC ("
                       "id_reportEcho, "
                        "hemispheres, "
                        "fissureSilvius, "
                        "corpCalos, "
                        "ventricularSystem, "
                        "ventricularSystem_description, "
                        "cavityPellucidSeptum, "
                        "choroidalPlex, "
                        "choroidalPlex_description, "
                        "cerebellum, "
                        "cerebellum_description, "
                        "vertebralColumn, "
                        "vertebralColumn_description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_SNC' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // heart
            qry.prepare("INSERT INTO tableGestation2_heart ("
                        "id_reportEcho, "
                        "`position`, "
                        "heartBeat, "
                        "heartBeat_frequency, "
                        "heartBeat_rhythm, "
                        "pericordialCollections, "
                        "planPatruCamere, "
                        "planPatruCamere_description, "
                        "ventricularEjectionPathLeft, "
                        "ventricularEjectionPathLeft_description, "
                        "ventricularEjectionPathRight, "
                        "ventricularEjectionPathRight_description, "
                        "intersectionVesselMagistral, "
                        "intersectionVesselMagistral_description, "
                        "planTreiVase, "
                        "planTreiVase_description, "
                        "archAorta, "
                        "planBicav) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_heart' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // thorax
            qry.prepare("INSERT INTO tableGestation2_thorax ("
                        "id_reportEcho, "
                        "pulmonaryAreas, "
                        "pulmonaryAreas_description, "
                        "pleuralCollections, "
                        "diaphragm) VALUES (?, ?, ?, ?, ?)"
                        ";");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->gestation2_pulmonaryAreas->currentIndex());
            qry.addBindValue((ui->gestation2_pulmonaryAreas_description->text().isEmpty()) ? QVariant() : ui->gestation2_pulmonaryAreas_description->text());
            qry.addBindValue(ui->gestation2_pleuralCollections->currentIndex());
            qry.addBindValue(ui->gestation2_diaphragm->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_thorax' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // abdomen
            qry.prepare("INSERT INTO tableGestation2_abdomen ("
                        "id_reportEcho, "
                        "abdominalWall, "
                        "abdominalCollections, "
                        "stomach, "
                        "stomach_description, "
                        "abdominalOrgans, "
                        "cholecist, "
                        "cholecist_description, "
                        "intestine, "
                        "intestine_description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_abdomen' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // s.urinar
            qry.prepare("INSERT INTO tableGestation2_urinarySystem ("
                        "id_reportEcho, "
                        "kidneys, "
                        "kidneys_descriptions, "
                        "ureter, "
                        "ureter_descriptions, "
                        "bladder) VALUES (?, ?, ?, ?, ?, ?)"
                        ";");
            qry.addBindValue(m_id);
            qry.addBindValue(ui->gestation2_kidneys->currentIndex());
            qry.addBindValue((ui->gestation2_kidneys_description->text().isEmpty()) ? QVariant() : ui->gestation2_kidneys_description->text());
            qry.addBindValue(ui->gestation2_ureter->currentIndex());
            qry.addBindValue((ui->gestation2_ureter_description->text().isEmpty()) ? QVariant() : ui->gestation2_ureter_description->text());
            qry.addBindValue(ui->gestation2_bladder->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_urinarySystem' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // other
            qry.prepare("INSERT INTO tableGestation2_other ("
                        "id_reportEcho, "
                        "externalGenitalOrgans, "
                        "externalGenitalOrgans_aspect, "
                        "extremities, "
                        "extremities_descriptions, "
                        "fetusMass, "
                        "placenta, "
                        "placentaLocalization, "
                        "placentaDegreeMaturation, "
                        "placentaDepth, "
                        "placentaStructure, "
                        "placentaStructure_descriptions, "
                        "umbilicalCordon, "
                        "umbilicalCordon_description, "
                        "insertionPlacenta, "
                        "amnioticIndex, "
                        "amnioticIndexAspect, "
                        "amnioticBedDepth, "
                        "cervix, "
                        "cervix_description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_other' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // doppler
            qry.prepare("INSERT INTO tableGestation2_doppler ("
                        "id_reportEcho, "
                        "ombilic_PI, "
                        "ombilic_RI, "
                        "ombilic_SD, "
                        "ombilic_flux, "
                        "cerebral_PI, "
                        "cerebral_RI, "
                        "cerebral_SD, "
                        "cerebral_flux, "
                        "uterRight_PI, "
                        "uterRight_RI, "
                        "uterRight_SD, "
                        "uterRight_flux, "
                        "uterLeft_PI, "
                        "uterLeft_RI, "
                        "uterLeft_SD, "
                        "uterLeft_flux, "
                        "ductVenos) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        ";");
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
            if (! qry.exec())
                details_error = tr("Eroarea inserarii datelor documentului nr.%1 din tabela 'tableGestation2_doppler' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

        }

        if (details_error.isEmpty()){
            return db->getDatabase().commit();
        } else {
            db->getDatabase().rollback();
            return false;
        }

    } catch (...) {
        db->getDatabase().rollback();
        throw;
    }
}

bool DocReportEcho::updatingDocumentDataIntoTables(QString &details_error)
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    try {

        qry.prepare("UPDATE reportEcho "
                    "SET deletionMark  = :deletionMark,"
                    "numberDoc         = :numberDoc,"
                    "dateDoc           = :dateDoc,"
                    "id_pacients       = :id_pacients,"
                    "id_orderEcho      = :id_orderEcho,"
                    "t_organs_internal = :t_organs_internal,"
                    "t_urinary_system  = :t_urinary_system,"
                    "t_prostate        = :t_prostate,"
                    "t_gynecology      = :t_gynecology,"
                    "t_breast          = :t_breast,"
                    "t_thyroid         = :t_thyroid,"
                    "t_gestation0      = :t_gestation0,"
                    "t_gestation1      = :t_gestation1,"
                    "t_gestation2      = :t_gestation2,"
                    "t_gestation3      = :t_gestation3,"
                    "id_users          = :id_users,"
                    "concluzion        = :concluzion,"
                    "comment           = :comment,"
                    "attachedImages    = :attachedImages WHERE id = :id;");
        qry.bindValue(":id",           m_id);
        qry.bindValue(":deletionMark", m_post);
        qry.bindValue(":numberDoc",    ui->editDocNumber->text());
        qry.bindValue(":dateDoc",      ui->editDocDate->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
        qry.bindValue(":id_pacients",  m_idPacient);
        qry.bindValue(":id_orderEcho", m_id_docOrderEcho);
        if (globals::thisMySQL) {
            qry.bindValue(":t_organs_internal", (m_organs_internal) ? true : false);
            qry.bindValue(":t_urinary_system",  (m_urinary_system) ? true : false);
            qry.bindValue(":t_prostate",        (m_prostate) ? true : false);
            qry.bindValue(":t_gynecology",      (m_gynecology) ? true : false);
            qry.bindValue(":t_breast",          (m_breast) ? true : false);
            qry.bindValue(":t_thyroid",         (m_thyroide) ? true : false);
            qry.bindValue(":t_gestation0",      (m_gestation0) ? true : false);
            qry.bindValue(":t_gestation1",      (m_gestation1) ? true : false);
            qry.bindValue(":t_gestation2",      (m_gestation2) ? true : false);
            qry.bindValue(":t_gestation3",      (m_gestation3) ? true : false);
        } else {
            qry.bindValue(":t_organs_internal", (m_organs_internal) ? 1 : 0);
            qry.bindValue(":t_urinary_system",  (m_urinary_system) ? 1 : 0);
            qry.bindValue(":t_prostate",        (m_prostate) ? 1 : 0);
            qry.bindValue(":t_gynecology",      (m_gynecology) ? 1 : 0);
            qry.bindValue(":t_breast",          (m_breast) ? 1 : 0);
            qry.bindValue(":t_thyroid",         (m_thyroide) ? 1 : 0);
            qry.bindValue(":t_gestation0",      (m_gestation0) ? 1 : 0);
            qry.bindValue(":t_gestation1",      (m_gestation1) ? 1 : 0);
            qry.bindValue(":t_gestation2",      (m_gestation2) ? 1 : 0);
            qry.bindValue(":t_gestation3",      (m_gestation3) ? 1 : 0);
        }
        qry.bindValue(":id_users",       m_idUser);
        qry.bindValue(":concluzion",     ui->concluzion->toPlainText());
        qry.bindValue(":comment",        (ui->comment->toPlainText().isEmpty()) ? QVariant() : ui->comment->toPlainText());
        qry.bindValue(":attachedImages", (m_count_images == 0) ? m_count_images : 1);
        if (! qry.exec())
            details_error = tr("Eroarea de actualizare datelor documentului nr.%1 din tabela 'reportEcho' - %2")
                                .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

        if (m_organs_internal)
        {

            //*********************************************
            // -- tableLiver
            qry.prepare("UPDATE tableLiver SET "
                        "  `left`            = :left,"
                        "  `right`           = :right,"
                        "  contur            = :contur,"
                        "  parenchim         = :parenchim,"
                        "  ecogenity         = :ecogenity,"
                        "  formations        = :formations,"
                        "  ductsIntrahepatic = :ductsIntrahepatic,"
                        "  porta             = :porta,"
                        "  lienalis          = :lienalis,"
                        "  concluzion        = :concluzion,"
                        "  recommendation    = :recommendation "
                        " WHERE "
                        "  id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",     m_id);
            qry.bindValue(":left",              ui->liver_left->text());
            qry.bindValue(":right",             ui->liver_right->text());
            qry.bindValue(":contur",            ui->liver_contour->text());
            qry.bindValue(":parenchim",         ui->liver_parenchyma->text());
            qry.bindValue(":ecogenity",         ui->liver_ecogenity->text());
            qry.bindValue(":formations",        ui->liver_formations->text());
            qry.bindValue(":ductsIntrahepatic", ui->liver_duct_hepatic->text());
            qry.bindValue(":porta",             ui->liver_porta->text());
            qry.bindValue(":lienalis",          ui->liver_lienalis->text());
            qry.bindValue(":concluzion",        ui->organsInternal_concluzion->toPlainText());
            qry.bindValue(":recommendation",    (ui->organsInternal_recommendation->text().isEmpty()) ? QVariant() : ui->organsInternal_recommendation->text());
            if (! qry.exec()){
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableLiver' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
            }

            //*********************************************
            // -- tableCholecist
            qry.prepare("UPDATE tableCholecist "
                        "SET form   = :form,"
                        "dimens     = :dimens,"
                        "walls      = :walls,"
                        "choledoc   = :choledoc,"
                        "formations = :formations WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":form",          ui->cholecist_form->text());
            qry.bindValue(":dimens",        ui->cholecist_dimens->text());
            qry.bindValue(":walls",         ui->cholecist_walls->text());
            qry.bindValue(":choledoc",      ui->cholecist_coledoc->text());
            qry.bindValue(":formations",    ui->cholecist_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableCholecist' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tablePancreas
            qry.prepare("UPDATE tablePancreas SET "
                        "cefal      = :cefal,"
                        "corp       = :corp,"
                        "tail       = :tail,"
                        "texture    = :texture,"
                        "ecogency   = :ecogency,"
                        "formations = :formations WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":cefal",         ui->pancreas_cefal->text());
            qry.bindValue(":corp",          ui->pancreas_corp->text());
            qry.bindValue(":tail",          ui->pancreas_tail->text());
            qry.bindValue(":texture",       ui->pancreas_parenchyma->text());
            qry.bindValue(":ecogency",      ui->pancreas_ecogenity->text());
            qry.bindValue(":formations",    ui->pancreas_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tablePancreas' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tableSpleen
            qry.prepare("UPDATE tableSpleen "
                        "SET dimens = :dimens,"
                        "contur =     :contur,"
                        "parenchim =  :parenchim,"
                        "formations = :formations WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":dimens",        ui->spleen_dimens->text());
            qry.bindValue(":contur",        ui->spleen_contour->text());
            qry.bindValue(":parenchim",     ui->spleen_parenchyma->text());
            qry.bindValue(":formations",    ui->spleen_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableSpleen' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_urinary_system)
        {

            //*********************************************
            // -- tableKidney
            qry.prepare("UPDATE tableKidney SET "
                        "dimens_right        = :dimens_right,"
                        "dimens_left         = :dimens_left,"
                        "corticomed_right    = :corticomed_right,"
                        "corticomed_left     = :corticomed_left,"
                        "pielocaliceal_right = :pielocaliceal_right,"
                        "pielocaliceal_left  = :pielocaliceal_left,"
                        "formations          = :formations,"
                        "concluzion          = :concluzion,"
                        "recommendation      = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",       m_id);
            qry.bindValue(":dimens_right",        ui->kidney_right->text());
            qry.bindValue(":dimens_left",         ui->kidney_left->text()),
                qry.bindValue(":corticomed_right",    ui->kidney_corticomed_right->text());
            qry.bindValue(":corticomed_left",     ui->kidney_corticomed_left->text());
            qry.bindValue(":pielocaliceal_right", ui->kidney_pielocaliceal_right->text());
            qry.bindValue(":pielocaliceal_left",  ui->kidney_pielocaliceal_left->text());
            qry.bindValue(":formations",          ui->kidney_formations->text());
            qry.bindValue(":concluzion",          ui->urinary_system_concluzion->toPlainText());
            qry.bindValue(":recommendation",      (ui->urinary_system_recommendation->text().isEmpty()) ? QVariant() : ui->urinary_system_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableKidney' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            //*********************************************
            // -- tableBladder
            qry.prepare("UPDATE tableBladder SET "
                        "volum = :volum,"
                        "walls = :walls,"
                        "formations = :formations WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",  m_id);
            qry.bindValue(":volum",          ui->bladder_volum->text());
            qry.bindValue(":walls",          ui->bladder_walls->text());
            qry.bindValue(":formations",     ui->bladder_formations->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableBladder' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_prostate)
        {
            qry.prepare("UPDATE tableProstate SET "
                        "dimens         = :dimens,"
                        "volume         = :volume,"
                        "ecostructure   = :ecostructure,"
                        "contour        = :contour,"
                        "ecogency       = :ecogency,"
                        "formations     = :formations,"
                        "transrectal    = :transrectal,"
                        "concluzion     = :concluzion,"
                        "recommendation = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":dimens",        ui->prostate_dimens->text());
            qry.bindValue(":volume",        ui->prostate_volum->text());
            qry.bindValue(":ecostructure",  ui->prostate_ecostructure->text());
            qry.bindValue(":contour",       ui->prostate_contur->text());
            qry.bindValue(":ecogency",      ui->prostate_ecogency->text());
            qry.bindValue(":formations",    ui->prostate_formations->text());
            if (globals::thisMySQL)
                qry.bindValue(":transrectal", (ui->prostate_radioBtn_transrectal->isChecked()) ? true : false);
            else
                qry.bindValue(":transrectal", (ui->prostate_radioBtn_transrectal->isChecked()) ? 1 : 0);
            qry.bindValue(":concluzion",     ui->prostate_concluzion->toPlainText());
            qry.bindValue(":recommendation", (ui->prostate_recommendation->text().isEmpty()) ? QVariant() : ui->prostate_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableProstate' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gynecology)
        {
            qry.prepare("UPDATE tableGynecology SET "
                        "transvaginal           = :transvaginal,"
                        "dateMenstruation       = :dateMenstruation,"
                        "antecedent             = :antecedent,"
                        "uterus_dimens          = :uterus_dimens,"
                        "uterus_pozition        = :uterus_pozition,"
                        "uterus_ecostructure    = :uterus_ecostructure,"
                        "uterus_formations      = :uterus_formations,"
                        "ecou_dimens            = :ecou_dimens,"
                        "ecou_ecostructure      = :ecou_ecostructure,"
                        "cervix_dimens          = :cervix_dimens,"
                        "cervix_ecostructure    = :cervix_ecostructure,"
                        "douglas                = :douglas,"
                        "plex_venos             = :plex_venos,"
                        "ovary_right_dimens     = :ovary_right_dimens,"
                        "ovary_left_dimens      = :ovary_left_dimens,"
                        "ovary_right_volum      = :ovary_right_volum,"
                        "ovary_left_volum       = :ovary_left_volum,"
                        "ovary_right_follicle   = :ovary_right_follicle,"
                        "ovary_left_follicle    = :ovary_left_follicle,"
                        "ovary_right_formations = :ovary_right_formations,"
                        "ovary_left_formations  = :ovary_left_formations,"
                        "concluzion             = :concluzion,"
                        "recommendation         = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho", m_id);
            if (globals::thisMySQL)
                qry.bindValue(":transvaginal", (ui->gynecology_btn_transvaginal->isChecked()) ? true : false);
            else
                qry.bindValue(":transvaginal", (ui->gynecology_btn_transvaginal->isChecked()) ? 1 : 0);
            qry.bindValue(":dateMenstruation",       ui->gynecology_dateMenstruation->date().toString("yyyy-MM-dd"));
            qry.bindValue(":antecedent",             (ui->gynecology_antecedent->text().isEmpty()) ? QVariant() : ui->gynecology_antecedent->text());
            qry.bindValue(":uterus_dimens",          ui->gynecology_uterus_dimens->text());
            qry.bindValue(":uterus_pozition",        ui->gynecology_uterus_pozition->text());
            qry.bindValue(":uterus_ecostructure",    ui->gynecology_uterus_ecostructure->text());
            qry.bindValue(":uterus_formations",      ui->gynecology_uterus_formations->toPlainText());
            qry.bindValue(":ecou_dimens",            ui->gynecology_ecou_dimens->text());
            qry.bindValue(":ecou_ecostructure",      ui->gynecology_ecou_ecostructure->text());
            qry.bindValue(":cervix_dimens",          ui->gynecology_cervix_dimens->text());
            qry.bindValue(":cervix_ecostructure",    ui->gynecology_cervix_ecostructure->text());
            qry.bindValue(":douglas",                ui->gynecology_douglas->text());
            qry.bindValue(":plex_venos",             ui->gynecology_plex_venos->text());
            qry.bindValue(":ovary_right_dimens",     ui->gynecology_ovary_right_dimens->text());
            qry.bindValue(":ovary_left_dimens",      ui->gynecology_ovary_left_dimens->text());
            qry.bindValue(":ovary_right_volum",      ui->gynecology_ovary_right_volum->text());
            qry.bindValue(":ovary_left_volum",       ui->gynecology_ovary_left_volum->text());
            qry.bindValue(":ovary_right_follicle",   ui->gynecology_follicule_right->text());
            qry.bindValue(":ovary_left_follicle",    ui->gynecology_follicule_left->text());
            qry.bindValue(":ovary_right_formations", ui->gynecology_ovary_formations_right->toPlainText());
            qry.bindValue(":ovary_left_formations",  ui->gynecology_ovary_formations_left->toPlainText());
            qry.bindValue(":concluzion",             ui->gynecology_concluzion->toPlainText());
            qry.bindValue(":recommendation",         (ui->gynecology_recommendation->text().isEmpty()) ? QVariant() : ui->gynecology_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGynecology' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_breast)
        {
            qry.prepare("UPDATE tableBreast SET "
                        "breast_right_ecostrcture = :breast_right_ecostrcture,"
                        "breast_right_duct        = :breast_right_duct,"
                        "breast_right_ligament    = :breast_right_ligament,"
                        "breast_right_formations  = :breast_right_formations,"
                        "breast_right_ganglions   = :breast_right_ganglions,"
                        "breast_left_ecostrcture  = :breast_left_ecostrcture,"
                        "breast_left_duct         = :breast_left_duct,"
                        "breast_left_ligament     = :breast_left_ligament,"
                        "breast_left_formations   = :breast_left_formations,"
                        "breast_left_ganglions    = :breast_left_ganglions,"
                        "concluzion               = :concluzion,"
                        "recommendation           = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",            m_id);
            qry.bindValue(":breast_right_ecostrcture", ui->breast_right_ecostructure->text());
            qry.bindValue(":breast_right_duct",        ui->breast_right_duct->text());
            qry.bindValue(":breast_right_ligament",    ui->breast_right_ligament->text());
            qry.bindValue(":breast_right_formations",  ui->breast_right_formations->toPlainText());
            qry.bindValue(":breast_right_ganglions",   ui->breast_right_ganglions->text());
            qry.bindValue(":breast_left_ecostrcture",  ui->breast_left_ecostructure->text());
            qry.bindValue(":breast_left_duct",         ui->breast_left_duct->text());
            qry.bindValue(":breast_left_ligament",     ui->breast_left_ligament->text());
            qry.bindValue(":breast_left_formations",   ui->breast_left_formations->toPlainText());
            qry.bindValue(":breast_left_ganglions",    ui->breast_left_ganglions->text());
            qry.bindValue(":concluzion",               ui->breast_concluzion->toPlainText());
            qry.bindValue(":recommendation",           (ui->breast_recommendation->text().isEmpty()) ? QVariant() : ui->breast_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableBreast' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_thyroide)
        {
            qry.prepare("UPDATE tableThyroid SET "
                        "thyroid_right_dimens = :thyroid_right_dimens,"
                        "thyroid_right_volum  = :thyroid_right_volum,"
                        "thyroid_left_dimens  = :thyroid_left_dimens,"
                        "thyroid_left_volum   = :thyroid_left_volum,"
                        "thyroid_istm         = :thyroid_istm,"
                        "thyroid_ecostructure = :thyroid_ecostructure,"
                        "thyroid_formations   = :thyroid_formations,"
                        "thyroid_ganglions    = :thyroid_ganglions,"
                        "concluzion           = :concluzion,"
                        "recommendation       = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",        m_id);
            qry.bindValue(":thyroid_right_dimens", ui->thyroid_right_dimens->text());
            qry.bindValue(":thyroid_right_volum",  ui->thyroid_right_volum->text());
            qry.bindValue(":thyroid_left_dimens",  ui->thyroid_left_dimens->text());
            qry.bindValue(":thyroid_left_volum",   ui->thyroid_left_volum->text());
            qry.bindValue(":thyroid_istm",         ui->thyroid_istm->text());
            qry.bindValue(":thyroid_ecostructure", ui->thyroid_ecostructure->text());
            qry.bindValue(":thyroid_formations",   ui->thyroid_formations->toPlainText());
            qry.bindValue(":thyroid_ganglions",    ui->thyroid_ganglions->text());
            qry.bindValue(":concluzion",           ui->thyroid_concluzion->toPlainText());
            qry.bindValue(":recommendation",       (ui->thyroid_recommendation->text().isEmpty()) ? QVariant() : ui->thyroid_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableThyroid' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation0)
        {
            qry.prepare("UPDATE tableGestation0 SET "
                        "view_examination = :view_examination,"
                        "antecedent       = :antecedent,"
                        "gestation_age    = :gestation_age,"
                        "GS               = :GS,"
                        "GS_age           = :GS_age,"
                        "CRL              = :CRL,"
                        "CRL_age          = :CRL_age,"
                        "BCF              = :BCF,"
                        "liquid_amniotic  = :liquid_amniotic,"
                        "miometer         = :miometer,"
                        "cervix           = :cervix,"
                        "ovary            = :ovary,"
                        "concluzion       = :concluzion,"
                        "recommendation   = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",    m_id);
            qry.bindValue(":view_examination", group_btn_gestation0->checkedId()),
            qry.bindValue(":antecedent",       (ui->gestation0_antecedent->text().isEmpty()) ? QVariant() : ui->gestation0_antecedent->text());
            qry.bindValue(":gestation_age",    ui->gestation0_gestation->text());
            qry.bindValue(":GS",               ui->gestation0_GS_dimens->text());
            qry.bindValue(":GS_age",           ui->gestation0_GS_age->text());
            qry.bindValue(":CRL",              ui->gestation0_CRL_dimens->text());
            qry.bindValue(":CRL_age",          ui->gestation0_CRL_age->text());
            qry.bindValue(":BCF",              ui->gestation0_BCF->text());
            qry.bindValue(":liquid_amniotic",  ui->gestation0_liquid_amniotic->text());
            qry.bindValue(":miometer",         ui->gestation0_miometer->text());
            qry.bindValue(":cervix",           ui->gestation0_cervix->text());
            qry.bindValue(":ovary",            ui->gestation0_ovary->text());
            qry.bindValue(":concluzion",       ui->gestation0_concluzion->toPlainText());
            qry.bindValue(":recommendation",   (ui->gestation0_recommendation->text().isEmpty()) ? QVariant() : ui->gestation0_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation0' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation1)
        {
            qry.prepare("UPDATE tableGestation1 SET "
                        "view_examination  = :view_examination,"
                        "antecedent        = :antecedent,"
                        "gestation_age     = :gestation_age,"
                        "CRL               = :CRL,"
                        "CRL_age           = :CRL_age,"
                        "BPD               = :BPD,"
                        "BPD_age           = :BPD_age,"
                        "NT                = :NT,"
                        "NT_percent        = :NT_percent,"
                        "BN                = :BN,"
                        "BN_percent        = :BN_percent,"
                        "BCF               = :BCF,"
                        "FL                = :FL,"
                        "FL_age            = :FL_age,"
                        "callote_cranium   = :callote_cranium,"
                        "plex_choroid      = :plex_choroid,"
                        "vertebral_column  = :vertebral_column,"
                        "stomach           = :stomach,"
                        "bladder           = :bladder,"
                        "diaphragm         = :diaphragm,"
                        "abdominal_wall    = :abdominal_wall,"
                        "location_placenta = :location_placenta,"
                        "sac_vitelin       = :sac_vitelin,"
                        "amniotic_liquid   = :amniotic_liquid,"
                        "miometer          = :miometer,"
                        "cervix            = :cervix,"
                        "ovary             = :ovary,"
                        "concluzion        = :concluzion,"
                        "recommendation    = :recommendation WHERE id_reportEcho = :id_reportEcho;");
            qry.bindValue(":id_reportEcho",     m_id);
            qry.bindValue(":view_examination",  group_btn_gestation1->checkedId()),
            qry.bindValue(":antecedent",        (ui->gestation1_antecedent->text().isEmpty()) ? QVariant() : ui->gestation1_antecedent->text());
            qry.bindValue(":gestation_age",     ui->gestation1_gestation->text());
            qry.bindValue(":CRL",               ui->gestation1_CRL_dimens->text());
            qry.bindValue(":CRL_age",           ui->gestation1_CRL_age->text());
            qry.bindValue(":BPD",               ui->gestation1_BPD_dimens->text());
            qry.bindValue(":BPD_age",           ui->gestation1_BPD_age->text());
            qry.bindValue(":NT",                ui->gestation1_NT_dimens->text());
            qry.bindValue(":NT_percent",        ui->gestation1_NT_percent->text());
            qry.bindValue(":BN",                ui->gestation1_BN_dimens->text());
            qry.bindValue(":BN_percent",        ui->gestation1_BN_percent->text());
            qry.bindValue(":BCF",               ui->gestation1_BCF->text());
            qry.bindValue(":FL",                ui->gestation1_FL_dimens->text());
            qry.bindValue(":FL_age",            ui->gestation1_FL_age->text());
            qry.bindValue(":callote_cranium",   ui->gestation1_callote_cranium->text());
            qry.bindValue(":plex_choroid",      ui->gestation1_plex_choroid->text());
            qry.bindValue(":vertebral_column",  ui->gestation1_vertebral_column->text());
            qry.bindValue(":stomach",           ui->gestation1_stomach->text());
            qry.bindValue(":bladder",           ui->gestation1_bladder->text());
            qry.bindValue(":diaphragm",         ui->gestation1_diaphragm->text());
            qry.bindValue(":abdominal_wall",    ui->gestation1_abdominal_wall->text());
            qry.bindValue(":location_placenta", ui->gestation1_location_placenta->text());
            qry.bindValue(":sac_vitelin",       ui->gestation1_sac_vitelin->text());
            qry.bindValue(":amniotic_liquid",   ui->gestation1_amniotic_liquid->text());
            qry.bindValue(":miometer",          ui->gestation1_miometer->text());
            qry.bindValue(":cervix",            ui->gestation1_cervix->text());
            qry.bindValue(":ovary",             ui->gestation1_ovary->text());
            qry.bindValue(":concluzion",        ui->gestation1_concluzion->toPlainText());
            qry.bindValue(":recommendation",    (ui->gestation1_recommendation->text().isEmpty()) ? QVariant() : ui->gestation1_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation1' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());
        }
        if (m_gestation2) {

            // main table
            qry.prepare("UPDATE tableGestation2 SET "
                        "gestation_age                         = :gestation_age, "
                        "trimestru                             = :trimestru, "
                        "dateMenstruation                      = :dateMenstruation, "
                        "view_examination                      = :view_examination, "
                        "single_multiple_pregnancy             = :single_multiple_pregnancy, "
                        "single_multiple_pregnancy_description = :single_multiple_pregnancy_description, "
                        "antecedent                            = :antecedent, "
                        "comment                               = :comment, "
                        "concluzion                            = :concluzion, "
                        "recommendation = :recommendation WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",                         m_id);
            qry.bindValue(":gestation_age",                         (ui->gestation2_gestation_age->text().isEmpty()) ? QVariant() : ui->gestation2_gestation_age->text());
            qry.bindValue(":trimestru",                             (ui->gestation2_trimestru2->isChecked()) ? 2 : 3);
            qry.bindValue(":dateMenstruation",                      ui->gestation2_dateMenstruation->date().toString("yyyy-MM-dd"));
            qry.bindValue(":view_examination",                      ui->gestation2_view_examination->currentIndex());
            qry.bindValue(":single_multiple_pregnancy",             ui->gestation2_pregnancy->currentIndex());
            qry.bindValue(":single_multiple_pregnancy_description", (ui->gestation2_pregnancy_description->text().isEmpty()) ? QVariant() : ui->gestation2_pregnancy_description->text());
            qry.bindValue(":antecedent",                            QVariant());
            qry.bindValue(":comment",                               (ui->gestation2_comment->toPlainText().isEmpty()) ? QVariant() : ui->gestation2_comment->toPlainText());
            qry.bindValue(":concluzion",                            ui->gestation2_concluzion->toPlainText());
            qry.bindValue(":recommendation",                        (ui->gestation2_recommendation->text().isEmpty()) ? QVariant() : ui->gestation2_recommendation->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // biometry
            qry.prepare("UPDATE tableGestation2_biometry SET "
                        "BPD     = :BPD, "
                        "BPD_age = :BPD_age, "
                        "HC      = :HC, "
                        "HC_age  = :HC_age, "
                        "AC      = :AC, "
                        "AC_age  = :AC_age, "
                        "FL      = :FL, "
                        "FL_age  = :FL_age, "
                        "FetusCorresponds = :FetusCorresponds WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",    m_id);
            qry.bindValue(":BPD",              ui->gestation2_bpd->text());
            qry.bindValue(":BPD_age",          ui->gestation2_bpd_age->text());
            qry.bindValue(":HC",               ui->gestation2_hc->text());
            qry.bindValue(":HC_age",           ui->gestation2_hc_age->text());
            qry.bindValue(":AC",               ui->gestation2_ac->text());
            qry.bindValue(":AC_age",           ui->gestation2_ac_age->text());
            qry.bindValue(":FL",               ui->gestation2_fl->text());
            qry.bindValue(":FL_age",           ui->gestation2_fl_age->text());
            qry.bindValue(":FetusCorresponds", ui->gestation2_fetus_age->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_biometry' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // cranium
            qry.prepare("UPDATE tableGestation2_cranium SET "
                        "calloteCranium                 = :calloteCranium, "
                        "facialeProfile                 = :facialeProfile, "
                        "nasalBones                     = :nasalBones, "
                        "nasalBones_dimens              = :nasalBones_dimens, "
                        "eyeball                        = :eyeball, "
                        "eyeball_desciption             = :eyeball_desciption, "
                        "nasolabialTriangle             = :nasolabialTriangle, "
                        "nasolabialTriangle_description = :nasolabialTriangle_description, "
                        "nasalFold = :nasalFold WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",      m_id);
            qry.bindValue(":calloteCranium",     ui->gestation2_calloteCranium->currentIndex());
            qry.bindValue(":facialeProfile",     ui->gestation2_facialeProfile->currentIndex());
            qry.bindValue(":nasalBones",         ui->gestation2_nasalBones->currentIndex());
            qry.bindValue(":nasalBones_dimens",  (ui->gestation2_nasalBones_dimens->text().isEmpty()) ? QVariant() : ui->gestation2_nasalBones_dimens->text());
            qry.bindValue(":eyeball",            ui->gestation2_eyeball->currentIndex());
            qry.bindValue(":eyeball_desciption", (ui->gestation2_eyeball_desciption->text().isEmpty()) ? QVariant() : ui->gestation2_eyeball_desciption->text());
            qry.bindValue(":nasolabialTriangle", ui->gestation2_nasolabialTriangle->currentIndex());
            qry.bindValue(":nasolabialTriangle_description", (ui->gestation2_nasolabialTriangle_description->text().isEmpty()) ? QVariant() : ui->gestation2_nasolabialTriangle_description->text());
            qry.bindValue(":nasalFold", (ui->gestation2_nasalFold->text().isEmpty()) ? QVariant() : ui->gestation2_nasalFold->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_cranium' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // SNC
            qry.prepare("UPDATE tableGestation2_SNC SET "
                        "hemispheres                   = :hemispheres, "
                        "fissureSilvius                = :fissureSilvius, "
                        "corpCalos                     = :corpCalos, "
                        "ventricularSystem             = :ventricularSystem, "
                        "ventricularSystem_description = :ventricularSystem_description, "
                        "cavityPellucidSeptum          = :cavityPellucidSeptum, "
                        "choroidalPlex                 = :choroidalPlex, "
                        "choroidalPlex_description     = :choroidalPlex_description, "
                        "cerebellum                    = :cerebellum, "
                        "cerebellum_description        = :cerebellum_description, "
                        "vertebralColumn               = :vertebralColumn, "
                        "vertebralColumn_description = :vertebralColumn_description WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":hemispheres",                   ui->gestation2_hemispheres->currentIndex());
            qry.bindValue(":fissureSilvius",                ui->gestation2_fissureSilvius->currentIndex());
            qry.bindValue(":corpCalos",                     ui->gestation2_corpCalos->currentIndex());
            qry.bindValue(":ventricularSystem",             ui->gestation2_ventricularSystem->currentIndex());
            qry.bindValue(":ventricularSystem_description", (ui->gestation2_ventricularSystem_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularSystem_description->text());
            qry.bindValue(":cavityPellucidSeptum",          ui->gestation2_cavityPellucidSeptum->currentIndex());
            qry.bindValue(":choroidalPlex",                 ui->gestation2_choroidalPlex->currentIndex());
            qry.bindValue(":choroidalPlex_description",     (ui->gestation2_choroidalPlex_description->text().isEmpty()) ? QVariant() : ui->gestation2_choroidalPlex_description->text());
            qry.bindValue(":cerebellum",                    ui->gestation2_cerebellum->currentIndex());
            qry.bindValue(":cerebellum_description",        (ui->gestation2_cerebellum_description->text().isEmpty()) ? QVariant() : ui->gestation2_cerebellum_description->text());
            qry.bindValue(":vertebralColumn",               ui->gestation2_vertebralColumn->currentIndex());
            qry.bindValue(":vertebralColumn_description",  (ui->gestation2_vertebralColumn_description->text().isEmpty()) ? QVariant() : ui->gestation2_vertebralColumn_description->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_SNC' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // heart
            qry.prepare("UPDATE tableGestation2_heart SET "
                        "`position`                              = :pos, "
                        "heartBeat                               = :heartBeat, "
                        "heartBeat_frequency                     = :heartBeat_frequency, "
                        "heartBeat_rhythm                        = :heartBeat_rhythm, "
                        "pericordialCollections                  = :pericordialCollections, "
                        "planPatruCamere                         = :planPatruCamere, "
                        "planPatruCamere_description             = :planPatruCamere_description, "
                        "ventricularEjectionPathLeft             = :ventricularEjectionPathLeft, "
                        "ventricularEjectionPathLeft_description = :ventricularEjectionPathLeft_description, "
                        "ventricularEjectionPathRight            = :ventricularEjectionPathRight, "
                        "ventricularEjectionPathRight_description = :ventricularEjectionPathRight_description, "
                        "intersectionVesselMagistral              = :intersectionVesselMagistral, "
                        "intersectionVesselMagistral_description  = :intersectionVesselMagistral_description, "
                        "planTreiVase                             = :planTreiVase, "
                        "planTreiVase_description                 = :planTreiVase_description, "
                        "archAorta                                = :archAorta, "
                        "planBicav = :planBicav WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",                m_id);
            qry.bindValue(":pos",                          (ui->gestation2_heartPosition->text().isEmpty()) ? QVariant() : ui->gestation2_heartPosition->text());
            qry.bindValue(":heartBeat",                    ui->gestation2_heartBeat->currentIndex());
            qry.bindValue(":heartBeat_frequency",          (ui->gestation2_heartBeat_frequency->text().isEmpty()) ? QVariant() : ui->gestation2_heartBeat_frequency->text());
            qry.bindValue(":heartBeat_rhythm",             ui->gestation2_heartBeat_rhythm->currentIndex());
            qry.bindValue(":pericordialCollections",       ui->gestation2_pericordialCollections->currentIndex());
            qry.bindValue(":planPatruCamere",              ui->gestation2_planPatruCamere->currentIndex());
            qry.bindValue(":planPatruCamere_description", (ui->gestation2_planPatruCamere_description->text().isEmpty()) ? QVariant() : ui->gestation2_planPatruCamere_description->text());
            qry.bindValue(":ventricularEjectionPathLeft",  ui->gestation2_ventricularEjectionPathLeft->currentIndex());
            qry.bindValue(":ventricularEjectionPathLeft_description", (ui->gestation2_ventricularEjectionPathLeft_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularEjectionPathLeft_description->text());
            qry.bindValue(":ventricularEjectionPathRight", ui->gestation2_ventricularEjectionPathRight->currentIndex());
            qry.bindValue(":ventricularEjectionPathRight_description", (ui->gestation2_ventricularEjectionPathRight_description->text().isEmpty()) ? QVariant() : ui->gestation2_ventricularEjectionPathRight_description->text());
            qry.bindValue(":intersectionVesselMagistral",  ui->gestation2_intersectionVesselMagistral->currentIndex());
            qry.bindValue(":intersectionVesselMagistral_description", (ui->gestation2_intersectionVesselMagistral_description->text().isEmpty()) ? QVariant() : ui->gestation2_intersectionVesselMagistral_description->text());
            qry.bindValue(":planTreiVase",                 ui->gestation2_planTreiVase->currentIndex());
            qry.bindValue(":planTreiVase_description",    (ui->gestation2_planTreiVase_description->text().isEmpty()) ? QVariant() : ui->gestation2_planTreiVase_description->text());
            qry.bindValue(":archAorta",                    ui->gestation2_archAorta->currentIndex());
            qry.bindValue(":planBicav",                    ui->gestation2_planBicav->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_heart' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // thorax
            qry.prepare("UPDATE tableGestation2_thorax SET "
                        "pulmonaryAreas             = :pulmonaryAreas, "
                        "pulmonaryAreas_description = :pulmonaryAreas_description, "
                        "pleuralCollections         = :pleuralCollections, "
                        "diaphragm = :diaphragm WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",               m_id);
            qry.bindValue(":pulmonaryAreas",              ui->gestation2_pulmonaryAreas->currentIndex());
            qry.bindValue(":pulmonaryAreas_description", (ui->gestation2_pulmonaryAreas_description->text().isEmpty()) ? QVariant() : ui->gestation2_pulmonaryAreas_description->text());
            qry.bindValue(":pleuralCollections",          ui->gestation2_pleuralCollections->currentIndex());
            qry.bindValue(":diaphragm",                   ui->gestation2_diaphragm->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_thorax' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // abdomen
            qry.prepare("UPDATE tableGestation2_abdomen SET "
                        "abdominalWall         = :abdominalWall, "
                        "abdominalCollections  = :abdominalCollections, "
                        "stomach               = :stomach, "
                        "stomach_description   = :stomach_description, "
                        "abdominalOrgans       = :abdominalOrgans, "
                        "cholecist             = :cholecist, "
                        "cholecist_description = :cholecist_description, "
                        "intestine             = :intestine, "
                        "intestine_description = :intestine_description WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",         m_id);
            qry.bindValue(":abdominalWall",         ui->gestation2_abdominalWall->currentIndex());
            qry.bindValue(":abdominalCollections",  ui->gestation2_abdominalCollections->currentIndex());
            qry.bindValue(":stomach",               ui->gestation2_stomach->currentIndex());
            qry.bindValue(":stomach_description",   (ui->gestation2_stomach_description->text().isEmpty()) ? QVariant() : ui->gestation2_stomach_description->text());
            qry.bindValue(":abdominalOrgans",       ui->gestation2_abdominalOrgans->currentIndex());
            qry.bindValue(":cholecist",             ui->gestation2_cholecist->currentIndex());
            qry.bindValue(":cholecist_description", (ui->gestation2_cholecist_description->text().isEmpty()) ? QVariant() : ui->gestation2_cholecist_description->text());
            qry.bindValue(":intestine",             ui->gestation2_intestine->currentIndex());
            qry.bindValue(":intestine_description", (ui->gestation2_intestine_description->text().isEmpty()) ? QVariant() : ui->gestation2_intestine_description->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_abdomen' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // s.urinar
            qry.prepare("UPDATE tableGestation2_urinarySystem SET "
                        "kidneys              = :kidneys, "
                        "kidneys_descriptions = :kidneys_descriptions, "
                        "ureter               = :ureter, "
                        "ureter_descriptions  = :ureter_descriptions, "
                        "bladder = :bladder WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho",        m_id);
            qry.bindValue(":kidneys",              ui->gestation2_kidneys->currentIndex());
            qry.bindValue(":kidneys_descriptions", (ui->gestation2_kidneys_description->text().isEmpty()) ? QVariant() : ui->gestation2_kidneys_description->text());
            qry.bindValue(":ureter",               ui->gestation2_ureter->currentIndex());
            qry.bindValue(":ureter_descriptions",  (ui->gestation2_ureter_description->text().isEmpty()) ? QVariant() : ui->gestation2_ureter_description->text());
            qry.bindValue(":bladder",              ui->gestation2_bladder->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_urinarySystem' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // other
            qry.prepare("UPDATE tableGestation2_other SET "
                        "externalGenitalOrgans          = :externalGenitalOrgans, "
                        "externalGenitalOrgans_aspect   = :externalGenitalOrgans_aspect, "
                        "extremities                    = :extremities, "
                        "extremities_descriptions       = :extremities_descriptions, "
                        "fetusMass                      = :fetusMass, "
                        "placenta                       = :placenta, "
                        "placentaLocalization           = :placentaLocalization, "
                        "placentaDegreeMaturation       = :placentaDegreeMaturation, "
                        "placentaDepth                  = :placentaDepth, "
                        "placentaStructure              = :placentaStructure, "
                        "placentaStructure_descriptions = :placentaStructure_descriptions, "
                        "umbilicalCordon                = :umbilicalCordon, "
                        "umbilicalCordon_description    = :umbilicalCordon_description, "
                        "insertionPlacenta              = :insertionPlacenta, "
                        "amnioticIndex                  = :amnioticIndex, "
                        "amnioticIndexAspect            = :amnioticIndexAspect, "
                        "amnioticBedDepth               = :amnioticBedDepth, "
                        "cervix                         = :cervix, "
                        "cervix_description = :cervix_description WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":externalGenitalOrgans",        ui->gestation2_fetusSex->currentIndex());
            qry.bindValue(":externalGenitalOrgans_aspect", QVariant());
            qry.bindValue(":extremities", ui->gestation2_extremities->currentIndex());
            qry.bindValue(":extremities_descriptions", (ui->gestation2_extremities_description->text().isEmpty()) ? QVariant() : ui->gestation2_extremities_description->text());
            qry.bindValue(":fetusMass",             (ui->gestation2_fetusMass->text().isEmpty()) ? QVariant() : ui->gestation2_fetusMass->text());
            qry.bindValue(":placenta",              ui->gestation2_placenta->currentIndex());
            qry.bindValue(":placentaLocalization", (ui->gestation2_placenta_localization->text().isEmpty()) ? QVariant() : ui->gestation1_location_placenta->text());
            qry.bindValue(":placentaDegreeMaturation", (ui->gestation2_placentaDegreeMaturation->text().isEmpty()) ? QVariant() : ui->gestation2_placentaDegreeMaturation->text());
            qry.bindValue(":placentaDepth",        (ui->gestation2_placentaDepth->text().isEmpty()) ? QVariant() : ui->gestation2_placentaDepth->text());
            qry.bindValue(":placentaStructure",     ui->gestation2_placentaStructure->currentIndex());
            qry.bindValue(":placentaStructure_descriptions", (ui->gestation2_placentaStructure_description->text().isEmpty()) ? QVariant() : ui->gestation2_placentaStructure_description->text());
            qry.bindValue(":umbilicalCordon",      ui->gestation2_umbilicalCordon->currentIndex());
            qry.bindValue(":umbilicalCordon_description", (ui->gestation2_umbilicalCordon_description->text().isEmpty()) ? QVariant() : ui->gestation2_umbilicalCordon_description->text());
            qry.bindValue(":insertionPlacenta",    ui->gestation2_insertionPlacenta->currentIndex());
            qry.bindValue(":amnioticIndex",        (ui->gestation2_amnioticIndex->text().isEmpty()) ? QVariant() : ui->gestation2_amnioticIndex->text());
            qry.bindValue(":amnioticIndexAspect",  ui->gestation2_amnioticIndexAspect->currentIndex());
            qry.bindValue(":amnioticBedDepth",     (ui->gestation2_amnioticBedDepth->text().isEmpty()) ? QVariant() : ui->gestation2_amnioticBedDepth->text());
            qry.bindValue(":cervix",               (ui->gestation2_cervix->text().isEmpty()) ? QVariant() : ui->gestation2_cervix->text());
            qry.bindValue(":cervix_description",   (ui->gestation2_cervix_description->text().isEmpty()) ? QVariant() : ui->gestation2_cervix_description->text());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_other' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

            // doppler
            qry.prepare("UPDATE tableGestation2_doppler SET "
                        "ombilic_PI     = :ombilic_PI, "
                        "ombilic_RI     = :ombilic_RI, "
                        "ombilic_SD     = :ombilic_SD, "
                        "ombilic_flux   = :ombilic_flux, "
                        "cerebral_PI    = :cerebral_PI, "
                        "cerebral_RI    = :cerebral_RI, "
                        "cerebral_SD    = :cerebral_SD, "
                        "cerebral_flux  = :cerebral_flux, "
                        "uterRight_PI   = :uterRight_PI, "
                        "uterRight_RI   = :uterRight_RI, "
                        "uterRight_SD   = :uterRight_SD, "
                        "uterRight_flux = :uterRight_flux, "
                        "uterLeft_PI    = :uterLeft_PI, "
                        "uterLeft_RI    = :uterLeft_RI, "
                        "uterLeft_SD    = :uterLeft_SD, "
                        "uterLeft_flux  = :uterLeft_flux, "
                        "ductVenos = :ductVenos WHERE id_reportEcho = :id_reportEcho"
                        ";");
            qry.bindValue(":id_reportEcho", m_id);
            qry.bindValue(":ombilic_PI",   (ui->gestation2_ombilic_PI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_PI->text());
            qry.bindValue(":ombilic_RI",   (ui->gestation2_ombilic_RI->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_RI->text());
            qry.bindValue(":ombilic_SD",   (ui->gestation2_ombilic_SD->text().isEmpty()) ? QVariant() : ui->gestation2_ombilic_SD->text());
            qry.bindValue(":ombilic_flux",  ui->gestation2_ombilic_flux->currentIndex());
            qry.bindValue(":cerebral_PI",  (ui->gestation2_cerebral_PI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_PI->text());
            qry.bindValue(":cerebral_RI",  (ui->gestation2_cerebral_RI->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_RI->text());
            qry.bindValue(":cerebral_SD",  (ui->gestation2_cerebral_SD->text().isEmpty()) ? QVariant() : ui->gestation2_cerebral_SD->text());
            qry.bindValue(":cerebral_flux", ui->gestation2_cerebral_flux->currentIndex());
            qry.bindValue(":uterRight_PI",  (ui->gestation2_uterRight_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_PI->text());
            qry.bindValue(":uterRight_RI",  (ui->gestation2_uterRight_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_RI->text());
            qry.bindValue(":uterRight_SD",  (ui->gestation2_uterRight_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterRight_SD->text());
            qry.bindValue(":uterRight_flux", ui->gestation2_uterRight_flux->currentIndex());
            qry.bindValue(":uterLeft_PI",   (ui->gestation2_uterLeft_PI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_PI->text());
            qry.bindValue(":uterLeft_RI",   (ui->gestation2_uterLeft_RI->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_RI->text());
            qry.bindValue(":uterLeft_SD",   (ui->gestation2_uterLeft_SD->text().isEmpty()) ? QVariant() : ui->gestation2_uterLeft_SD->text());
            qry.bindValue(":uterLeft_flux", ui->gestation2_uterLeft_flux->currentIndex());
            qry.bindValue(":ductVenos",     ui->gestation2_ductVenos->currentIndex());
            if (! qry.exec())
                details_error = tr("Eroarea actualizarii datelor documentului nr.%1 din tabela 'tableGestation2_doppler' - %2")
                                    .arg(ui->editDocNumber->text(), (qry.lastError().text().isEmpty()) ? tr("eroare indisponibila") : qry.lastError().text());

        }

        if (details_error.isEmpty()){
            return db->getDatabase().commit();
        } else {
            db->getDatabase().rollback();
            return false;
        }

    } catch (...) {
        db->getDatabase().rollback();
        throw;
    }
}

void DocReportEcho::updateDataDocOrderEcho()
{
    if (m_id_docOrderEcho == idx_unknow || m_id_docOrderEcho == 0)
        return;

    QString str = QString("UPDATE orderEcho SET attachedImages = '%1' WHERE id = '%2';")
            .arg((m_count_images == 0) ? QString::number(m_count_images) : QString::number(1),
                 QString::number(m_id_docOrderEcho));
    if (! db->execQuery(str))
        qCritical(logCritical()) << tr("Document 'Raport ecografic' cu id='%1', nr.='%2' - eroare la instalarea valorii 'attachedImages' in documentul parinte 'Comanda ecografica' cu id='%3'.")
                                        .arg(QString::number(m_id), ui->editDocNumber->text(), QString::number(m_id_docOrderEcho));
}

// *******************************************************************
// **************** EXTRAGEREA DATELOR DIN BD SI SETAREA IN FORMA ****

QString DocReportEcho::getStringTablesBySystems()
{
    QString str;
    str = "SELECT "
          "reportEcho.numberDoc,"
          "reportEcho.dateDoc,"
          "reportEcho.id_pacients,"
          "reportEcho.id_orderEcho,"
          "reportEcho.t_organs_internal,"
          "reportEcho.t_urinary_system,"
          "reportEcho.t_prostate,"
          "reportEcho.t_gynecology,"
          "reportEcho.t_breast,"
          "reportEcho.t_thyroid,"
          "reportEcho.t_gestation0,"
          "reportEcho.t_gestation1,"
          "reportEcho.t_gestation2,"
          "reportEcho.id_users,"
          "reportEcho.concluzion,"
          "reportEcho.comment,"
          "reportEcho.attachedImages,";
    if (m_organs_internal){
        str = str + "tableLiver.left AS liver_left,"                   // tableLiver
                    "tableLiver.right AS liver_right,"
                    "tableLiver.contur AS liver_contur,"
                    "tableLiver.parenchim AS liver_parenchim,"
                    "tableLiver.ecogenity AS liver_ecogenity,"
                    "tableLiver.formations AS liver_formations,"
                    "tableLiver.ductsIntrahepatic AS liver_ductsIntrahep,"
                    "tableLiver.porta AS liver_porta,"
                    "tableLiver.lienalis AS liver_lienalis,"
                    "tableLiver.concluzion AS liver_concluzion,"
                    "tableLiver.recommendation AS liver_recommendation,"
                    "tableCholecist.form AS cholecist_form,"          // tableCholecist
                    "tableCholecist.dimens AS cholecist_dimens,"
                    "tableCholecist.walls AS cholecist_walls,"
                    "tableCholecist.choledoc AS cholecist_choledoc,"
                    "tableCholecist.formations AS cholecist_formations,"
                    "tablePancreas.cefal AS pancreas_cefal,"          // tablePancreas
                    "tablePancreas.corp AS pancreas_corp,"
                    "tablePancreas.tail AS pancreas_tail,"
                    "tablePancreas.texture AS pancreas_texture,"
                    "tablePancreas.ecogency AS pancreas_ecogency,"
                    "tablePancreas.formations AS pancreas_formations,"
                    "tableSpleen.contur AS spleen_contur,"             // tableSpleen
                    "tableSpleen.dimens AS spleen_dimens,"
                    "tableSpleen.parenchim AS spleen_parenchim,"
                    "tableSpleen.formations AS spleen_formations";
    }
    if (m_urinary_system){
        if (m_organs_internal)
            str = str + ",";
        str = str + "tableKidney.dimens_right AS kidney_dimens_right,"         // tableKidney
                    "tableKidney.dimens_left AS kidney_dimens_left,"
                    "tableKidney.corticomed_right AS kidney_corticomed_right,"
                    "tableKidney.corticomed_left AS kidney_corticomed_left,"
                    "tableKidney.pielocaliceal_right AS kidney_pielocaliceal_right,"
                    "tableKidney.pielocaliceal_left AS kidney_pielocaliceal_left,"
                    "tableKidney.formations AS kidney_formations,"
                    "tableKidney.concluzion AS kidney_concluzion,"
                    "tableKidney.recommendation AS kidney_recommendation,"
                    "tableBladder.volum AS bladder_volum,"               // tableBladder
                    "tableBladder.walls AS bladder_walls,"
                    "tableBladder.formations AS bladder_formations";
    }
    if (m_prostate){
        if (m_organs_internal || m_urinary_system)
            str = str + ",";
        str = str + "tableProstate.transrectal AS prostate_transrectal,"
                    "tableProstate.dimens AS prostate_dimens,"
                    "tableProstate.volume AS prostate_volume,"
                    "tableProstate.contour AS prostate_contour,"
                    "tableProstate.ecostructure AS prostate_ecostructure,"
                    "tableProstate.ecogency AS prostate_ecogency,"
                    "tableProstate.formations AS prostate_formations,"
                    "tableProstate.concluzion AS prostate_concluzion,"
                    "tableProstate.recommendation AS prostate_recommendation";
    }
    if (m_gynecology){
        if (m_organs_internal || m_urinary_system) // m_prostate excludem
            str = str + ",";
        str = str + "tableGynecology.transvaginal,"
                    "tableGynecology.dateMenstruation,"
                    "tableGynecology.antecedent AS gynecology_antecedent,"
                    "tableGynecology.uterus_dimens,"
                    "tableGynecology.uterus_pozition,"
                    "tableGynecology.uterus_ecostructure,"
                    "tableGynecology.uterus_formations,"
                    "tableGynecology.ecou_dimens,"
                    "tableGynecology.ecou_ecostructure,"
                    "tableGynecology.cervix_dimens,"
                    "tableGynecology.cervix_ecostructure,"
                    "tableGynecology.douglas,"
                    "tableGynecology.plex_venos,"
                    "tableGynecology.ovary_right_dimens,"
                    "tableGynecology.ovary_left_dimens,"
                    "tableGynecology.ovary_right_volum,"
                    "tableGynecology.ovary_left_volum,"
                    "tableGynecology.ovary_right_follicle,"
                    "tableGynecology.ovary_left_follicle,"
                    "tableGynecology.ovary_right_formations,"
                    "tableGynecology.ovary_left_formations,"
                    "tableGynecology.concluzion AS gynecology_concluzion,"
                    "tableGynecology.recommendation AS gynecology_recommendation";
    }
    if (m_breast){
        if (m_organs_internal || m_urinary_system || m_prostate || m_gynecology)
            str = str + ",";
        str = str + "tableBreast.breast_right_ecostrcture,"
                    "tableBreast.breast_right_duct,"
                    "tableBreast.breast_right_ligament,"
                    "tableBreast.breast_right_formations,"
                    "tableBreast.breast_right_ganglions,"
                    "tableBreast.breast_left_ecostrcture,"
                    "tableBreast.breast_left_duct,"
                    "tableBreast.breast_left_ligament,"
                    "tableBreast.breast_left_formations,"
                    "tableBreast.breast_left_ganglions,"
                    "tableBreast.concluzion AS breast_concluzion,"
                    "tableBreast.recommendation  AS breast_recommendation";
    }
    if (m_thyroide){
        if (m_organs_internal || m_urinary_system || m_prostate || m_gynecology || m_breast)
            str = str + ",";
        str = str + "tableThyroid.thyroid_right_dimens,"
                    "tableThyroid.thyroid_right_volum,"
                    "tableThyroid.thyroid_left_dimens,"
                    "tableThyroid.thyroid_left_volum,"
                    "tableThyroid.thyroid_istm,"
                    "tableThyroid.thyroid_ecostructure,"
                    "tableThyroid.thyroid_formations,"
                    "tableThyroid.thyroid_ganglions,"
                    "tableThyroid.concluzion AS thyroide_concluzion,"
                    "tableThyroid.recommendation  AS thyroide_recommendation";
    }

    str = str + " FROM reportEcho ";

    if (m_organs_internal){
        str = str + "INNER JOIN tableLiver ON tableLiver.id_reportEcho = reportEcho.id "
                    "INNER JOIN tableCholecist ON tableCholecist.id_reportEcho = reportEcho.id "
                    "INNER JOIN tablePancreas ON tablePancreas.id_reportEcho = reportEcho.id "
                    "INNER JOIN tableSpleen ON tableSpleen.id_reportEcho = reportEcho.id ";
    }
    if (m_urinary_system){
        str = str + "INNER JOIN tableKidney ON tableKidney.id_reportEcho = reportEcho.id "
                    "INNER JOIN tableBladder ON tableBladder.id_reportEcho = reportEcho.id ";
    }
    if (m_prostate)
        str = str + "INNER JOIN tableProstate ON tableProstate.id_reportEcho = reportEcho.id ";
    if (m_gynecology)
        str = str + "INNER JOIN tableGynecology ON tableGynecology.id_reportEcho = reportEcho.id ";
    if (m_breast)
        str = str + "INNER JOIN tableBreast ON tableBreast.id_reportEcho = reportEcho.id ";
    if (m_thyroide)
        str = str + "INNER JOIN tableThyroid ON tableThyroid.id_reportEcho = reportEcho.id ";

    str = str + "WHERE reportEcho.id = :id;";
    return str;
}

void DocReportEcho::processingRequest()
{
    QSqlQuery qry;
    qry.prepare(getStringTablesBySystems());
    qry.bindValue(":id", m_id);
    if (qry.exec()){
        if (qry.next()){
            QSqlRecord rec = qry.record();

            //*************************************************************
            // setam data si numarul
            disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::dataWasModified);
            disconnect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);
            disconnections_tags();

            ui->editDocNumber->setText(qry.value(rec.indexOf("numberDoc")).toString());
            ui->editDocNumber->setDisabled(! ui->editDocNumber->text().isEmpty());
            if (globals::thisMySQL){
                QString str_date = qry.value(rec.indexOf("dateDoc")).toString();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                str_date = str_date.replace(QRegExp("T"), " ").replace(".000","");
#else
                str_date = str_date.replace(QRegularExpression("T"), " ").replace(".000","");
#endif
                ui->editDocDate->setDateTime(QDateTime::fromString(str_date, "yyyy-MM-dd hh:mm:ss"));
            } else
                ui->editDocDate->setDateTime(QDateTime::fromString(qry.value(rec.indexOf("dateDoc")).toString(), "yyyy-MM-dd hh:mm:ss"));

            connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::dataWasModified);
            connect(ui->editDocDate, &QDateTimeEdit::dateTimeChanged, this, &DocReportEcho::onDateTimeChanged);

            //************************************************************
            // setam id_pacients, id_orderEcho si tag-utile sistemelor
            int id_pacient = qry.value(rec.indexOf("id_pacients")).toInt();
            setIdPacient(id_pacient);

            int id_orderEcho = qry.value(rec.indexOf("id_orderEcho")).toInt();
            setIdDocOrderEcho(id_orderEcho);

            set_t_organs_internal(qry.value(rec.indexOf("t_organs_internal")).toInt());
            set_t_urinary_system(qry.value(rec.indexOf("t_urinary_system")).toInt());
            set_t_prostate(qry.value(rec.indexOf("t_prostate")).toInt());
            set_t_gynecology(qry.value(rec.indexOf("t_gynecology")).toInt());
            set_t_breast(qry.value(rec.indexOf("t_breast")).toInt());
            set_t_thyroide(qry.value(rec.indexOf("t_thyroid")).toInt());
            set_t_gestation0(qry.value(rec.indexOf("t_gestation0")).toInt());
            set_t_gestation1(qry.value(rec.indexOf("t_gestation1")).toInt());
            set_t_gestation2(qry.value(rec.indexOf("t_gestation2")).toInt());
            initEnableBtn();
            connections_tags();

            //************************************************************
            // setam date dupa sisteme

            if (m_organs_internal){

                disconnections_organs_internal(); // deconectam modificarea formei la modificarea textului

                ui->liver_left->setText(qry.value(rec.indexOf("liver_left")).toString());
                ui->liver_right->setText(qry.value(rec.indexOf("liver_right")).toString());
                ui->liver_contour->setText(qry.value(rec.indexOf("liver_contur")).toString());
                ui->liver_parenchyma->setText(qry.value(rec.indexOf("liver_parenchim")).toString());
                ui->liver_ecogenity->setText(qry.value(rec.indexOf("liver_ecogenity")).toString());
                ui->liver_formations->setText(qry.value(rec.indexOf("liver_formations")).toString());
                ui->liver_duct_hepatic->setText(qry.value(rec.indexOf("liver_ductsIntrahep")).toString());
                ui->liver_porta->setText(qry.value(rec.indexOf("liver_porta")).toString());
                ui->liver_lienalis->setText(qry.value(rec.indexOf("liver_lienalis")).toString());
                ui->organsInternal_concluzion->setPlainText(qry.value(rec.indexOf("liver_concluzion")).toString());
                ui->organsInternal_recommendation->setText(qry.value(rec.indexOf("liver_recommendation")).toString());

                ui->cholecist_form->setText(qry.value(rec.indexOf("cholecist_form")).toString());
                ui->cholecist_dimens->setText(qry.value(rec.indexOf("cholecist_dimens")).toString());
                ui->cholecist_walls->setText(qry.value(rec.indexOf("cholecist_walls")).toString());
                ui->cholecist_coledoc->setText(qry.value(rec.indexOf("cholecist_choledoc")).toString());
                ui->cholecist_formations->setText(qry.value(rec.indexOf("cholecist_formations")).toString());

                ui->pancreas_cefal->setText(qry.value(rec.indexOf("pancreas_cefal")).toString());
                ui->pancreas_corp->setText(qry.value(rec.indexOf("pancreas_corp")).toString());
                ui->pancreas_tail->setText(qry.value(rec.indexOf("pancreas_tail")).toString());
                ui->pancreas_parenchyma->setText(qry.value(rec.indexOf("pancreas_texture")).toString());
                ui->pancreas_ecogenity->setText(qry.value(rec.indexOf("pancreas_ecogency")).toString());
                ui->pancreas_formations->setText(qry.value(rec.indexOf("pancreas_formations")).toString());

                ui->spleen_contour->setText(qry.value(rec.indexOf("spleen_contur")).toString());
                ui->spleen_dimens->setText(qry.value(rec.indexOf("spleen_dimens")).toString());
                ui->spleen_parenchyma->setText(qry.value(rec.indexOf("spleen_parenchim")).toString());
                ui->spleen_formations->setText(qry.value(rec.indexOf("spleen_formations")).toString());

                connections_organs_internal(); // conectam modificarea formei la modificarea textului
            }
            if (m_urinary_system){

                disconnections_urinary_system();

                ui->kidney_right->setText(qry.value(rec.indexOf("kidney_dimens_right")).toString());
                ui->kidney_left->setText(qry.value(rec.indexOf("kidney_dimens_left")).toString());
                ui->kidney_corticomed_right->setText(qry.value(rec.indexOf("kidney_corticomed_right")).toString());
                ui->kidney_corticomed_left->setText(qry.value(rec.indexOf("kidney_corticomed_left")).toString());
                ui->kidney_pielocaliceal_right->setText(qry.value(rec.indexOf("kidney_pielocaliceal_right")).toString());
                ui->kidney_pielocaliceal_left->setText(qry.value(rec.indexOf("kidney_pielocaliceal_left")).toString());
                ui->kidney_formations->setText(qry.value(rec.indexOf("kidney_formations")).toString());
                ui->urinary_system_concluzion->setPlainText(qry.value(rec.indexOf("kidney_concluzion")).toString());
                ui->urinary_system_recommendation->setText(qry.value(rec.indexOf("kidney_recommendation")).toString());

                ui->bladder_volum->setText(qry.value(rec.indexOf("bladder_volum")).toString());
                ui->bladder_walls->setText(qry.value(rec.indexOf("bladder_walls")).toString());
                ui->bladder_formations->setText(qry.value(rec.indexOf("bladder_formations")).toString());

                connections_urinary_system();

            }
            if (m_prostate){

                disconnections_prostate();

                ui->prostate_radioBtn_transrectal->setChecked(qry.value(rec.indexOf("prostate_transrectal")).toInt());
                ui->prostate_dimens->setText(qry.value(rec.indexOf("prostate_dimens")).toString());
                ui->prostate_volum->setText(qry.value(rec.indexOf("prostate_volume")).toString());
                ui->prostate_contur->setText(qry.value(rec.indexOf("prostate_contour")).toString());
                ui->prostate_ecostructure->setText(qry.value(rec.indexOf("prostate_ecostructure")).toString());
                ui->prostate_ecogency->setText(qry.value(rec.indexOf("prostate_ecogency")).toString());
                ui->prostate_formations->setText(qry.value(rec.indexOf("prostate_formations")).toString());
                ui->prostate_concluzion->setPlainText(qry.value(rec.indexOf("prostate_concluzion")).toString());
                ui->prostate_recommendation->setText(qry.value(rec.indexOf("prostate_recommendation")).toString());

                connections_prostate();
            }
            if (m_gynecology){

                disconnections_gynecology();

                bool transvaginal_checked = qry.value(rec.indexOf("transvaginal")).toInt();
                ui->gynecology_btn_transvaginal->setChecked(transvaginal_checked);
                ui->gynecology_btn_transvaginal->setChecked(! transvaginal_checked);

                ui->gynecology_dateMenstruation->setDate(QDate::fromString(qry.value(rec.indexOf("dateMenstruation")).toString(), "yyyy-MM-dd"));
                ui->gynecology_antecedent->setText(qry.value(rec.indexOf("gynecology_antecedent")).toString());
                ui->gynecology_uterus_dimens->setText(qry.value(rec.indexOf("uterus_dimens")).toString());
                ui->gynecology_uterus_pozition->setText(qry.value(rec.indexOf("uterus_pozition")).toString());
                ui->gynecology_uterus_ecostructure->setText(qry.value(rec.indexOf("uterus_ecostructure")).toString());
                ui->gynecology_uterus_formations->setPlainText(qry.value(rec.indexOf("uterus_formations")).toString());
                ui->gynecology_ecou_dimens->setText(qry.value(rec.indexOf("ecou_dimens")).toString());
                ui->gynecology_ecou_ecostructure->setText(qry.value(rec.indexOf("ecou_ecostructure")).toString());
                ui->gynecology_cervix_dimens->setText(qry.value(rec.indexOf("cervix_dimens")).toString());
                ui->gynecology_cervix_ecostructure->setText(qry.value(rec.indexOf("cervix_ecostructure")).toString());
                ui->gynecology_douglas->setText(qry.value(rec.indexOf("douglas")).toString());
                ui->gynecology_plex_venos->setText(qry.value(rec.indexOf("plex_venos")).toString());
                ui->gynecology_ovary_right_dimens->setText(qry.value(rec.indexOf("ovary_right_dimens")).toString());
                ui->gynecology_ovary_left_dimens->setText(qry.value(rec.indexOf("ovary_left_dimens")).toString());
                ui->gynecology_ovary_right_volum->setText(qry.value(rec.indexOf("ovary_right_volum")).toString());
                ui->gynecology_ovary_left_volum->setText(qry.value(rec.indexOf("ovary_left_volum")).toString());
                ui->gynecology_follicule_right->setText(qry.value(rec.indexOf("ovary_right_follicle")).toString());
                ui->gynecology_follicule_left->setText(qry.value(rec.indexOf("ovary_left_follicle")).toString());
                ui->gynecology_ovary_formations_right->setPlainText(qry.value(rec.indexOf("ovary_right_formations")).toString());
                ui->gynecology_ovary_formations_left->setPlainText(qry.value(rec.indexOf("ovary_left_formations")).toString());
                ui->gynecology_concluzion->setPlainText(qry.value(rec.indexOf("gynecology_concluzion")).toString());
                ui->gynecology_recommendation->setText(qry.value(rec.indexOf("gynecology_recommendation")).toString());

                connections_gynecology();
            }
            if (m_breast){

                disconnections_breast();

                ui->breast_right_ecostructure->setText(qry.value(rec.indexOf("breast_right_ecostrcture")).toString());
                ui->breast_right_duct->setText(qry.value(rec.indexOf("breast_right_duct")).toString());
                ui->breast_right_ligament->setText(qry.value(rec.indexOf("breast_right_ligament")).toString());
                ui->breast_right_formations->setPlainText(qry.value(rec.indexOf("breast_right_formations")).toString());
                ui->breast_right_ganglions->setText(qry.value(rec.indexOf("breast_right_ganglions")).toString());

                ui->breast_left_ecostructure->setText(qry.value(rec.indexOf("breast_left_ecostrcture")).toString());
                ui->breast_left_duct->setText(qry.value(rec.indexOf("breast_left_duct")).toString());
                ui->breast_left_ligament->setText(qry.value(rec.indexOf("breast_left_ligament")).toString());
                ui->breast_left_formations->setPlainText(qry.value(rec.indexOf("breast_left_formations")).toString());
                ui->breast_left_ganglions->setText(qry.value(rec.indexOf("breast_left_ganglions")).toString());

                ui->breast_concluzion->setPlainText(qry.value(rec.indexOf("breast_concluzion")).toString());
                ui->breast_recommendation->setText(qry.value(rec.indexOf("breast_recommendation")).toString());

                connections_breast();
            }
            if (m_thyroide){

                disconnections_thyroid();

                ui->thyroid_right_dimens->setText(qry.value(rec.indexOf("thyroid_right_dimens")).toString());
                ui->thyroid_right_volum->setText(qry.value(rec.indexOf("thyroid_right_volum")).toString());
                ui->thyroid_left_dimens->setText(qry.value(rec.indexOf("thyroid_left_dimens")).toString());
                ui->thyroid_left_volum->setText(qry.value(rec.indexOf("thyroid_left_volum")).toString());
                ui->thyroid_istm->setText(qry.value(rec.indexOf("thyroid_istm")).toString());
                ui->thyroid_ecostructure->setText(qry.value(rec.indexOf("thyroid_ecostructure")).toString());
                ui->thyroid_formations->setPlainText(qry.value(rec.indexOf("thyroid_formations")).toString());
                ui->thyroid_ganglions->setText(qry.value(rec.indexOf("thyroid_ganglions")).toString());

                ui->thyroid_concluzion->setPlainText(qry.value(rec.indexOf("thyroide_concluzion")).toString());
                ui->thyroid_recommendation->setText(qry.value(rec.indexOf("thyroide_recommendation")).toString());

                connections_thyroid();
            }

            //************************************************************
            // setam id_users, concluzia si comentariu
            int id_user = qry.value(rec.indexOf("id_users")).toInt();
            setIdUser(id_user);

            ui->concluzion->setPlainText(qry.value(rec.indexOf("concluzion")).toString());
            ui->comment->setPlainText(qry.value(rec.indexOf("comment")).toString());
            ui->comment->setHidden(ui->comment->toPlainText().isEmpty());
        }
    } else {

    }
}

void DocReportEcho::setDataFromTableLiver()
{
//    auto str = getStringTablesBySystems();
//    qDebug() << str;
//    processingRequest();
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableLiver WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->liver_left->setText(items.constFind("left").value());
            ui->liver_right->setText(items.constFind("right").value());
            ui->liver_contour->setText(items.constFind("contur").value());
            ui->liver_parenchyma->setText(items.constFind("parenchim").value());
            ui->liver_ecogenity->setText(items.constFind("ecogenity").value());
            ui->liver_formations->setText(items.constFind("formations").value());
            ui->liver_duct_hepatic->setText(items.constFind("ductsIntrahepatic").value());
            ui->liver_porta->setText(items.constFind("porta").value());
            ui->liver_lienalis->setText(items.constFind("lienalis").value());
            ui->organsInternal_concluzion->setPlainText(items.constFind("concluzion").value());
        }
    }
}

void DocReportEcho::setDataFromTableCholecist()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableCholecist WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->cholecist_form->setText(items.constFind("form").value());
            ui->cholecist_dimens->setText(items.constFind("dimens").value());
            ui->cholecist_walls->setText(items.constFind("walls").value());
            ui->cholecist_coledoc->setText(items.constFind("choledoc").value());
            ui->cholecist_formations->setText(items.constFind("formations").value());
        }
    }
}

void DocReportEcho::setDataFromTablePancreas()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tablePancreas WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->pancreas_cefal->setText(items.constFind("cefal").value());
            ui->pancreas_corp->setText(items.constFind("corp").value());
            ui->pancreas_tail->setText(items.constFind("tail").value());
            ui->pancreas_parenchyma->setText(items.constFind("texture").value());
            ui->pancreas_ecogenity->setText(items.constFind("ecogency").value());
            ui->pancreas_formations->setText(items.constFind("formations").value());
        }
    }
}

void DocReportEcho::setDataFromTableSpleen()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableSpleen WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->spleen_contour->setText(items.constFind("contur").value());
            ui->spleen_dimens->setText(items.constFind("dimens").value());
            ui->spleen_parenchyma->setText(items.constFind("parenchim").value());
            ui->spleen_formations->setText(items.constFind("formations").value());
        }
    }
}

void DocReportEcho::setDataFromTableKidney()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableKidney WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->kidney_right->setText(items.constFind("dimens_right").value());
            ui->kidney_left->setText(items.constFind("dimens_left").value());
            ui->kidney_corticomed_right->setText(items.constFind("corticomed_right").value());
            ui->kidney_corticomed_left->setText(items.constFind("corticomed_left").value());
            ui->kidney_pielocaliceal_right->setText(items.constFind("pielocaliceal_right").value());
            ui->kidney_pielocaliceal_left->setText(items.constFind("pielocaliceal_left").value());
            ui->kidney_formations->setText(items.constFind("formations").value());
            ui->urinary_system_concluzion->setPlainText(items.constFind("concluzion").value());
        }
    }
}

void DocReportEcho::setDataFromTableBladder()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableBladder WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->bladder_volum->setText(items.constFind("volum").value());
            ui->bladder_walls->setText(items.constFind("walls").value());
            ui->bladder_formations->setText(items.constFind("formations").value());
        }
    }
}

void DocReportEcho::setDataFromTableProstate()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableProstate WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->prostate_radioBtn_transrectal->setChecked(items.constFind("transrectal").value().toInt());
            ui->prostate_dimens->setText(items.constFind("dimens").value());
            ui->prostate_volum->setText(items.constFind("volume").value());
            ui->prostate_contur->setText(items.constFind("contour").value());
            ui->prostate_ecostructure->setText(items.constFind("ecostructure").value());
            ui->prostate_ecogency->setText(items.constFind("ecogency").value());
            ui->prostate_formations->setText(items.constFind("formations").value());
            ui->prostate_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->prostate_recommendation->setText(items.constFind("recommendation").value());
        }
    }
}

void DocReportEcho::setDataFromTableGynecology()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGynecology WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            bool transvaginal_checked = items.constFind("transvaginal").value().toInt();
            ui->gynecology_btn_transvaginal->setChecked(transvaginal_checked);
            ui->gynecology_btn_transabdom->setChecked(!transvaginal_checked);

            ui->gynecology_dateMenstruation->setDate(QDate::fromString(items.constFind("dateMenstruation").value(), "yyyy-MM-dd"));
            ui->gynecology_antecedent->setText(items.constFind("antecedent").value());
            ui->gynecology_uterus_dimens->setText(items.constFind("uterus_dimens").value());
            ui->gynecology_uterus_pozition->setText(items.constFind("uterus_pozition").value());
            ui->gynecology_uterus_ecostructure->setText(items.constFind("uterus_ecostructure").value());
            ui->gynecology_uterus_formations->setPlainText(items.constFind("uterus_formations").value());
            ui->gynecology_ecou_dimens->setText(items.constFind("ecou_dimens").value());
            ui->gynecology_ecou_ecostructure->setText(items.constFind("ecou_ecostructure").value());
            ui->gynecology_cervix_dimens->setText(items.constFind("cervix_dimens").value());
            ui->gynecology_cervix_ecostructure->setText(items.constFind("cervix_ecostructure").value());
            ui->gynecology_douglas->setText(items.constFind("douglas").value());
            ui->gynecology_plex_venos->setText(items.constFind("plex_venos").value());
            ui->gynecology_ovary_right_dimens->setText(items.constFind("ovary_right_dimens").value());
            ui->gynecology_ovary_left_dimens->setText(items.constFind("ovary_left_dimens").value());
            ui->gynecology_ovary_right_volum->setText(items.constFind("ovary_right_volum").value());
            ui->gynecology_ovary_left_volum->setText(items.constFind("ovary_left_volum").value());
            ui->gynecology_follicule_right->setText(items.constFind("ovary_right_follicle").value());
            ui->gynecology_follicule_left->setText(items.constFind("ovary_left_follicle").value());
            ui->gynecology_ovary_formations_right->setPlainText(items.constFind("ovary_right_formations").value());
            ui->gynecology_ovary_formations_left->setPlainText(items.constFind("ovary_left_formations").value());
            ui->gynecology_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->gynecology_recommendation->setText(items.constFind("recommendation").value());
        }
    }
}

void DocReportEcho::setDataFromTableBreast()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableBreast WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->breast_right_ecostructure->setText(items.constFind("breast_right_ecostrcture").value());
            ui->breast_right_duct->setText(items.constFind("breast_right_duct").value());
            ui->breast_right_ligament->setText(items.constFind("breast_right_ligament").value());
            ui->breast_right_formations->setPlainText(items.constFind("breast_right_formations").value());
            ui->breast_right_ganglions->setText(items.constFind("breast_right_ganglions").value());
            ui->breast_left_ecostructure->setText(items.constFind("breast_left_ecostrcture").value());
            ui->breast_left_duct->setText(items.constFind("breast_left_duct").value());
            ui->breast_left_ligament->setText(items.constFind("breast_left_ligament").value());
            ui->breast_left_formations->setPlainText(items.constFind("breast_left_formations").value());
            ui->breast_left_ganglions->setText(items.constFind("breast_left_ganglions").value());
            ui->breast_concluzion->setPlainText(items.constFind("concluzion").value());
        }
    }
}

void DocReportEcho::setDataFromTableThyroid()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableThyroid WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->thyroid_right_dimens->setText(items.constFind("thyroid_right_dimens").value());
            ui->thyroid_right_volum->setText(items.constFind("thyroid_right_volum").value());
            ui->thyroid_left_dimens->setText(items.constFind("thyroid_left_dimens").value());
            ui->thyroid_left_volum->setText(items.constFind("thyroid_left_volum").value());
            ui->thyroid_istm->setText(items.constFind("thyroid_istm").value());
            ui->thyroid_ecostructure->setText(items.constFind("thyroid_ecostructure").value());
            ui->thyroid_formations->setPlainText(items.constFind("thyroid_formations").value());
            ui->thyroid_ganglions->setText(items.constFind("thyroid_ganglions").value());
            ui->thyroid_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->thyroid_recommendation->setText(items.constFind("recommendation").value());
        }
    }
}

void DocReportEcho::setDataFromTableGestation0()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation0 WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation0_view_good->setChecked(items.constFind("view_examination").value().toInt() == 0);
            ui->gestation0_view_medium->setChecked(items.constFind("view_examination").value().toInt() == 1);
            ui->gestation0_view_difficult->setChecked(items.constFind("view_examination").value().toInt() == 2);

            ui->gestation0_antecedent->setText(items.constFind("antecedent").value());
            ui->gestation0_gestation->setText(items.constFind("gestation_age").value());
            ui->gestation0_GS_dimens->setText(items.constFind("GS").value());
            ui->gestation0_GS_age->setText(items.constFind("GS_age").value());
            ui->gestation0_CRL_dimens->setText(items.constFind("CRL").value());
            ui->gestation0_CRL_age->setText(items.constFind("CRL_age").value());
            ui->gestation0_BCF->setText(items.constFind("BCF").value());
            ui->gestation0_liquid_amniotic->setText(items.constFind("liquid_amniotic").value());
            ui->gestation0_miometer->setText(items.constFind("miometer").value());
            ui->gestation0_cervix->setText(items.constFind("cervix").value());
            ui->gestation0_ovary->setText(items.constFind("ovary").value());
            ui->gestation0_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->gestation0_recommendation->setText(items.constFind("recommendation").value());
        }
    }
}

void DocReportEcho::setDataFromTableGestation1()
{
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation1 WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation1_view_good->setChecked(items.constFind("view_examination").value().toInt() == 0);
            ui->gestation1_view_medium->setChecked(items.constFind("view_examination").value().toInt() == 1);
            ui->gestation1_view_difficult->setChecked(items.constFind("view_examination").value().toInt() == 2);

            ui->gestation1_antecedent->setText(items.constFind("antecedent").value());
            ui->gestation1_gestation->setText(items.constFind("gestation_age").value());
            ui->gestation1_CRL_dimens->setText(items.constFind("CRL").value());
            ui->gestation1_CRL_age->setText(items.constFind("CRL_age").value());
            ui->gestation1_BPD_dimens->setText(items.constFind("BPD").value());
            ui->gestation1_BPD_age->setText(items.constFind("BPD_age").value());
            ui->gestation1_NT_dimens->setText(items.constFind("NT").value());
            ui->gestation1_NT_percent->setText(items.constFind("NT_percent").value());
            ui->gestation1_BN_dimens->setText(items.constFind("BN").value());
            ui->gestation1_BN_percent->setText(items.constFind("BN_percent").value());
            ui->gestation1_BCF->setText(items.constFind("BCF").value());
            ui->gestation1_FL_dimens->setText(items.constFind("FL").value());
            ui->gestation1_FL_age->setText(items.constFind("FL_age").value());
            ui->gestation1_callote_cranium->setText(items.constFind("callote_cranium").value());
            ui->gestation1_plex_choroid->setText(items.constFind("plex_choroid").value());
            ui->gestation1_vertebral_column->setText(items.constFind("vertebral_column").value());
            ui->gestation1_stomach->setText(items.constFind("stomach").value());
            ui->gestation1_bladder->setText(items.constFind("bladder").value());
            ui->gestation1_diaphragm->setText(items.constFind("diaphragm").value());
            ui->gestation1_abdominal_wall->setText(items.constFind("abdominal_wall").value());
            ui->gestation1_location_placenta->setText(items.constFind("location_placenta").value());
            ui->gestation1_sac_vitelin->setText(items.constFind("sac_vitelin").value());
            ui->gestation1_amniotic_liquid->setText(items.constFind("amniotic_liquid").value());
            ui->gestation1_miometer->setText(items.constFind("miometer").value());
            ui->gestation1_cervix->setText(items.constFind("cervix").value());
            ui->gestation1_ovary->setText(items.constFind("ovary").value());
            ui->gestation1_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->gestation1_recommendation->setText(items.constFind("recommendation").value());
        }
    }
}

void DocReportEcho::setDataFromTableGestation2()
{
    // main
    QMap<QString, QString> items;
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2 WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_dateMenstruation->setDate(QDate::fromString(items.constFind("dateMenstruation").value(), "yyyy-MM-dd"));
            ui->gestation2_gestation_age->setText(items.constFind("gestation_age").value());
            ui->gestation2_view_examination->setCurrentIndex(items.constFind("view_examination").value().toInt());
            if (items.constFind("trimestru").value().toInt() == 2){
                ui->gestation2_trimestru2->setChecked(true);
                clickedGestation2Trimestru(0);
            } else {
                ui->gestation2_trimestru3->setChecked(true);
                clickedGestation2Trimestru(1);
            }
            ui->gestation2_pregnancy->setCurrentIndex(items.constFind("single_multiple_pregnancy").value().toInt());
            ui->gestation2_pregnancy_description->setText(items.constFind("single_multiple_pregnancy_description").value());
            ui->gestation2_comment->setPlainText(items.constFind("comment").value());
            ui->gestation2_concluzion->setPlainText(items.constFind("concluzion").value());
            ui->gestation2_recommendation->setText(items.constFind("recommendation").value());
        }
    }

    // biometry
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_biometry WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_bpd->setText(items.constFind("BPD").value());
            ui->gestation2_bpd_age->setText(items.constFind("BPD_age").value());
            ui->gestation2_hc->setText(items.constFind("HC").value());
            ui->gestation2_hc_age->setText(items.constFind("HC_age").value());
            ui->gestation2_ac->setText(items.constFind("AC").value());
            ui->gestation2_ac_age->setText(items.constFind("AC_age").value());
            ui->gestation2_fl->setText(items.constFind("FL").value());
            ui->gestation2_fl_age->setText(items.constFind("FL_age").value());
            ui->gestation2_fetus_age->setText(items.constFind("FetusCorresponds").value());
        }
    }

    // cranium
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_cranium WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_calloteCranium->setCurrentIndex(items.constFind("calloteCranium").value().toInt());
            ui->gestation2_facialeProfile->setCurrentIndex(items.constFind("facialeProfile").value().toInt());
            ui->gestation2_nasalBones->setCurrentIndex(items.constFind("nasalBones").value().toInt());
            ui->gestation2_nasalBones_dimens->setText(items.constFind("nasalBones_dimens").value());
            ui->gestation2_eyeball->setCurrentIndex(items.constFind("eyeball").value().toInt());
            ui->gestation2_eyeball_desciption->setText(items.constFind("eyeball_desciption").value());
            ui->gestation2_nasolabialTriangle->setCurrentIndex(items.constFind("nasolabialTriangle").value().toInt());
            ui->gestation2_nasolabialTriangle_description->setText(items.constFind("nasolabialTriangle_description").value());
            ui->gestation2_nasalFold->setText(items.constFind("nasalFold").value());
        }
    }

    // SNC
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_SNC WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_hemispheres->setCurrentIndex(items.constFind("hemispheres").value().toInt());
            ui->gestation2_fissureSilvius->setCurrentIndex(items.constFind("fissureSilvius").value().toInt());
            ui->gestation2_corpCalos->setCurrentIndex(items.constFind("corpCalos").value().toInt());
            ui->gestation2_ventricularSystem->setCurrentIndex(items.constFind("ventricularSystem").value().toInt());
            ui->gestation2_ventricularSystem_description->setText(items.constFind("ventricularSystem_description").value());
            ui->gestation2_cavityPellucidSeptum->setCurrentIndex(items.constFind("cavityPellucidSeptum").value().toInt());
            ui->gestation2_choroidalPlex->setCurrentIndex(items.constFind("choroidalPlex").value().toInt());
            ui->gestation2_choroidalPlex_description->setText(items.constFind("choroidalPlex_description").value());
            ui->gestation2_cerebellum->setCurrentIndex(items.constFind("cerebellum").value().toInt());
            ui->gestation2_cerebellum_description->setText(items.constFind("cerebellum_description").value());
            ui->gestation2_vertebralColumn->setCurrentIndex(items.constFind("vertebralColumn").value().toInt());
            ui->gestation2_vertebralColumn_description->setText(items.constFind("vertebralColumn_description").value());
        }
    }

    // heart
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_heart WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_heartPosition->setText(items.constFind("position").value());
            ui->gestation2_heartBeat->setCurrentIndex(items.constFind("heartBeat").value().toInt());
            ui->gestation2_heartBeat_frequency->setText(items.constFind("heartBeat_frequency").value());
            ui->gestation2_heartBeat_rhythm->setCurrentIndex(items.constFind("heartBeat_rhythm").value().toInt());
            ui->gestation2_pericordialCollections->setCurrentIndex(items.constFind("pericordialCollections").value().toInt());
            ui->gestation2_planPatruCamere->setCurrentIndex(items.constFind("planPatruCamere").value().toInt());
            ui->gestation2_planPatruCamere_description->setText(items.constFind("planPatruCamere_description").value());
            ui->gestation2_ventricularEjectionPathLeft->setCurrentIndex(items.constFind("ventricularEjectionPathLeft").value().toInt());
            ui->gestation2_ventricularEjectionPathLeft_description->setText(items.constFind("ventricularEjectionPathLeft_description").value());
            ui->gestation2_ventricularEjectionPathRight->setCurrentIndex(items.constFind("ventricularEjectionPathRight").value().toInt());
            ui->gestation2_ventricularEjectionPathRight_description->setText(items.constFind("ventricularEjectionPathRight_description").value());
            ui->gestation2_intersectionVesselMagistral->setCurrentIndex(items.constFind("intersectionVesselMagistral").value().toInt());
            ui->gestation2_intersectionVesselMagistral_description->setText(items.constFind("intersectionVesselMagistral_description").value());
            ui->gestation2_planTreiVase->setCurrentIndex(items.constFind("planTreiVase").value().toInt());
            ui->gestation2_planTreiVase_description->setText(items.constFind("planTreiVase_description").value());
            ui->gestation2_archAorta->setCurrentIndex(items.constFind("archAorta").value().toInt());
            ui->gestation2_planBicav->setCurrentIndex(items.constFind("planBicav").value().toInt());
        }
    }

    // thorax
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_thorax WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_pulmonaryAreas->setCurrentIndex(items.constFind("pulmonaryAreas").value().toInt());
            ui->gestation2_pulmonaryAreas_description->setText(items.constFind("pulmonaryAreas_description").value());
            ui->gestation2_pleuralCollections->setCurrentIndex(items.constFind("pleuralCollections").value().toInt());
            ui->gestation2_diaphragm->setCurrentIndex(items.constFind("diaphragm").value().toInt());
        }
    }

    // abdomen
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_abdomen WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation1_abdominal_wall->setText(items.constFind("abdominalWall").value());
            ui->gestation2_abdominalCollections->setCurrentIndex(items.constFind("abdominalCollections").value().toInt());
            ui->gestation2_stomach->setCurrentIndex(items.constFind("stomach").value().toInt());
            ui->gestation2_stomach_description->setText(items.constFind("stomach_description").value());
            ui->gestation2_abdominalOrgans->setCurrentIndex(items.constFind("abdominalOrgans").value().toInt());
            ui->gestation2_cholecist->setCurrentIndex(items.constFind("cholecist").value().toInt());
            ui->gestation2_cholecist_description->setText(items.constFind("cholecist_description").value());
            ui->gestation2_intestine->setCurrentIndex(items.constFind("intestine").value().toInt());
            ui->gestation2_intestine_description->setText(items.constFind("intestine_description").value());
        }
    }

    // s.urinar
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_urinarySystem WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_kidneys->setCurrentIndex(items.constFind("kidneys").value().toInt());
            ui->gestation2_kidneys_description->setText(items.constFind("kidneys_descriptions").value());
            ui->gestation2_ureter->setCurrentIndex(items.constFind("ureter").value().toInt());
            ui->gestation2_ureter_description->setText(items.constFind("ureter_descriptions").value());
            ui->gestation2_bladder->setCurrentIndex(items.constFind("bladder").value().toInt());
        }
    }

    // other
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_other WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_extremities->setCurrentIndex(items.constFind("extremities").value().toInt());
            ui->gestation2_extremities_description->setText(items.constFind("extremities_descriptions").value());
            ui->gestation2_placenta->setCurrentIndex(items.constFind("placenta").value().toInt());
            ui->gestation2_placenta_localization->setText(items.constFind("placentaLocalization").value());
            ui->gestation2_placentaDegreeMaturation->setText(items.constFind("placentaDegreeMaturation").value());
            ui->gestation2_placentaDepth->setText(items.constFind("placentaDepth").value());
            ui->gestation2_placentaStructure->setCurrentIndex(items.constFind("placentaStructure").value().toInt());
            ui->gestation2_placentaStructure_description->setText(items.constFind("placentaStructure_descriptions").value());
            ui->gestation2_umbilicalCordon->setCurrentIndex(items.constFind("umbilicalCordon").value().toInt());
            ui->gestation2_umbilicalCordon_description->setText(items.constFind("umbilicalCordon_description").value());
            ui->gestation2_insertionPlacenta->setCurrentIndex(items.constFind("insertionPlacenta").value().toInt());
            ui->gestation2_amnioticIndex->setText(items.constFind("amnioticIndex").value());
            ui->gestation2_amnioticIndexAspect->setCurrentIndex(items.constFind("amnioticIndexAspect").value().toInt());
            ui->gestation2_amnioticBedDepth->setText(items.constFind("amnioticBedDepth").value());
            ui->gestation2_cervix->setText(items.constFind("cervix").value());
            ui->gestation2_cervix_description->setText(items.constFind("cervix_description").value());
            ui->gestation2_fetusMass->setText(items.constFind("fetusMass").value());
            ui->gestation2_fetusSex->setCurrentIndex(items.constFind("externalGenitalOrgans").value().toInt());
        }
    }

    // doppler
    items.clear();
    if (db->getDataFromQueryByRecord(QString("SELECT * FROM tableGestation2_doppler WHERE id_reportEcho = '%1';").arg(m_id), items)){
        if (items.count() > 0){
            ui->gestation2_ombilic_PI->setText(items.constFind("ombilic_PI").value());
            ui->gestation2_ombilic_RI->setText(items.constFind("ombilic_RI").value());
            ui->gestation2_ombilic_SD->setText(items.constFind("ombilic_SD").value());
            ui->gestation2_ombilic_flux->setCurrentIndex(items.constFind("ombilic_flux").value().toInt());
            ui->gestation2_cerebral_PI->setText(items.constFind("cerebral_PI").value());
            ui->gestation2_cerebral_RI->setText(items.constFind("cerebral_RI").value());
            ui->gestation2_cerebral_SD->setText(items.constFind("cerebral_SD").value());
            ui->gestation2_cerebral_flux->setCurrentIndex(items.constFind("cerebral_flux").value().toInt());
            ui->gestation2_uterRight_PI->setText(items.constFind("uterRight_PI").value());
            ui->gestation2_uterRight_RI->setText(items.constFind("uterRight_RI").value());
            ui->gestation2_uterRight_SD->setText(items.constFind("uterRight_SD").value());
            ui->gestation2_uterRight_flux->setCurrentIndex(items.constFind("uterRight_flux").value().toInt());
            ui->gestation2_uterLeft_PI->setText(items.constFind("uterLeft_PI").value());
            ui->gestation2_uterLeft_RI->setText(items.constFind("uterLeft_RI").value());
            ui->gestation2_uterLeft_SD->setText(items.constFind("uterLeft_SD").value());
            ui->gestation2_uterLeft_flux->setCurrentIndex(items.constFind("uterLeft_flux").value().toInt());
            ui->gestation2_ductVenos->setCurrentIndex(items.constFind("ductVenos").value().toInt());
        }
    }
}

// *******************************************************************
// **************** EVENIMENTE FORMEI ********************************

void DocReportEcho::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        const QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Modificarea datelor"),
                                                                        tr("Datele au fost modificate.\n"
                                                                           "Doriți să salvați aceste modificări ?"),
                                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes){
            onWritingDataClose();
            event->accept();
        } else if (answer == QMessageBox::Cancel){
            event->ignore();
        }
    } else {
        event->accept();
    }
}

bool DocReportEcho::eventFilter(QObject *obj, QEvent *event)
{
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
#if defined(Q_OS_LINUX)
            popUp->setPopupText(tr("Deschide documentul<br>\"Comanda ecografică\".")); // setam textul
#elif defined(Q_OS_MACOS)
            popUp->setPopupText(tr("Deschide documentul<br>\"Comanda ecografică\".")); // setam textul
#elif defined(Q_OS_WIN)
            popUp->setPopupText(tr("Deschide documentul<br>\"Comanda ecografic\304\203\".")); // setam textul
#endif
            popUp->showFromGeometryTimer(p);                     // realizam vizualizarea notei timp de 5 sec.
            return true;
        } else if (event->type() == QEvent::Leave){
            popUp->hidePop();                           // ascundem nota
            return true;
        }
    }

    return false;
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
