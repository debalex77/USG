#include "catalogdialog.h"
#include "ui_catalogdialog.h"

static const int max_length_comment = 255; // lungimea maxima a cometariului

CatalogDialog::CatalogDialog(DataBase &db,
                             CatalogType::Type catalogType,
                             QWidget *parent)
    : QDialog(parent)
    , m_statusCatalog(StatusObject::Unknow) // initial
    , ui(new Ui::CatalogDialog)
    , m_typeCatalog(catalogType)
    , m_db(db)
    , popUp(new PopUp(this))
    , styleBtnMessageBox(m_db.getStyleForButtonMessageBox())
{
    ui->setupUi(this);

    catalogTypeChanged();

    connect(ui->editFullName, &QLineEdit::textChanged,
            this, &CatalogDialog::fullNameChanged, Qt::UniqueConnection);
    connect(ui->editFullName, &QLineEdit::editingFinished,
            this, &CatalogDialog::fullNameSplit, Qt::UniqueConnection);
    connect(ui->editComment, &QTextEdit::textChanged,
            this, &CatalogDialog::controlLengthComment, Qt::UniqueConnection); // limitarea caracterilor la comentariu

    // Tabul cu semnatura si stampila apartine catalogului Doctori.
    // catalogTypeChanged() il activa, dar aici era ascuns din nou pentru toate
    // tipurile de catalog, inclusiv la editarea unui doctor existent.
    ui->tabWidget->setTabVisible(
        1, m_typeCatalog == CatalogType::Type::Doctors);

    ui->editName->setMaxLength(80);        // limitarea caracterilor
    ui->editPrenume->setMaxLength(50);
    ui->editPatronimic->setMaxLength(50);
    ui->editIDNP->setMaxLength(20);
    ui->editPoliceMed->setMaxLength(20);
    ui->editAddress->setMaxLength(255);
    ui->editTelephone->setMaxLength(100);
    ui->editEmail->setMaxLength(100);

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged,
                this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->editComment, &QTextEdit::textChanged,
            this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);

    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(ui->btnOK, &QAbstractButton::clicked,
            this, &CatalogDialog::onWritingDataClose, Qt::UniqueConnection);
    connect(ui->btnWrite, &QAbstractButton::clicked,
            this, &CatalogDialog::onWritingData, Qt::UniqueConnection);
    connect(ui->btnCancel, &QAbstractButton::clicked,
            this, &CatalogDialog::close, Qt::UniqueConnection);
}

CatalogDialog::~CatalogDialog()
{
    delete ui;
}

bool CatalogDialog::setDeleteMarkCatalog(QString &err)
{
    return handleDeletionMark(err);
}

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void CatalogDialog::slot_IsNewChanged()
{
    if (m_isNew){
        setStatusCatalog(StatusObject::Unknow);
        slot_StatusCatalogChanged(); // fortam apelarea din cauza ca initial unknow
    }
}

void CatalogDialog::slot_IdChanged()
{
    if (m_isNew)
        return;

    // blocam signale pu modificarea formei
    QList<QLineEdit*> items = this->findChildren<QLineEdit*>();
    std::vector<QSignalBlocker> itemsBlockers;
    itemsBlockers.reserve(items.size());
    for (QLineEdit *item : std::as_const(items))
        itemsBlockers.emplace_back(item);

    QSignalBlocker dt(ui->dateEdit);
    QSignalBlocker cm(ui->editComment);

    // determinam solicitarea
    QString strQry;
    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        strQry = m_db.getTextSQL(":/sql/queries_catalog/doctors_all_byID.sql");
        break;
    case CatalogType::Type::Nurses:
        strQry = m_db.getTextSQL(":/sql/queries_catalog/nurses_all_byID.sql");
        break;
    case CatalogType::Type::Patients:
        strQry = m_db.getTextSQL(":/sql/queries_catalog/pacients_all_byID.sql");
        break;
    default:
        break;
    }

    QSqlQuery qry;
    qry.prepare(strQry);
    qry.addBindValue(m_id);
    if (!qry.exec()) {
        qCritical(logCritical()).noquote()
            << "SQL error:" << qry.lastError().text()
            << "\nQuery:" << qry.lastQuery();
        return;
    }

    if (qry.next()) {
        QSqlRecord record = qry.record();

        const bool isPatient = m_typeCatalog == CatalogType::Type::Patients;
        setStatusCatalog(StatusObject::determineStatusObject(
            record.value(isPatient ? "deletion_mark" : "deletionMark").toInt()));

        if (m_typeCatalog == CatalogType::Type::Doctors) {
            // signature
            QByteArray arr_signature = QByteArray::fromBase64(record.value("signature").toByteArray());
            QPixmap pix_signature;
            if (! arr_signature.isEmpty() && pix_signature.loadFromData(arr_signature))
                ui->imageSignature->setPixmap(pix_signature.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            // stamp
            QByteArray arr_stamp = QByteArray::fromBase64(record.value("stamp").toByteArray());
            QPixmap pix_stamp;
            if (! arr_stamp.isEmpty() && pix_stamp.loadFromData(arr_stamp))
                ui->imageStamp->setPixmap(pix_stamp.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        if (m_typeCatalog == CatalogType::Type::Patients) {
            ui->editIDNP->setText(record.value("idnp").toString());
            ui->editPoliceMed->setText(record.value("medical_policy").toString());
            ui->dateEdit->setDate(QDate::fromString(record.value("birthday").toString(), "yyyy-MM-dd"));
            ui->editAddress->setText(record.value("address").toString());
        }

        // valori comune
        ui->editName->setText(record.value(isPatient ? "last_name" : "name").toString());
        ui->editPrenume->setText(record.value(isPatient ? "first_name" : "fName").toString());
        ui->editPatronimic->setText(record.value(isPatient ? "middle_name" : "mName").toString());
        ui->editTelephone->setText(record.value("telephone").toString());
        ui->editEmail->setText(record.value("email").toString());
        ui->editComment->setText(record.value("comment").toString());
        ui->editFullName->setText(ui->editName->text() + " " + ui->editPrenume->text() + " " + ui->editPatronimic->text());
    }

    ui->editName->setFocus();
}

void CatalogDialog::slot_FullNameChanged()
{
    QString strFullName = ui->editFullName->text();
    if (firstSpace == 0){
        QString n1 = strFullName.right(1);
        if (n1 == " "){
            strName = strFullName.left(strFullName.size() - 1);
            firstSpace = 1;
        } else {
            strName = strFullName.right(strFullName.size());
        }
    } else if (firstSpace == 1){
        QString n2 = strFullName.right(1);
        if (n2 == " "){
            strPrenume = strFullName.right(strFullName.size() - strName.size() - 1);
            firstSpace = 2;
        } else {
            strPrenume = strFullName.right(strFullName.size() - (strName.size() + 1));
        }
    } else {
        strPatrimonic = strFullName.right(strFullName.size() - (strName.size() + 1) - strPrenume.size());
    }

    ui->editName->setText(strName);
    ui->editPrenume->setText(strPrenume);
    ui->editPatronimic->setText(strPatrimonic);
    setFullName(ui->editFullName->text());
}

void CatalogDialog::slot_StatusCatalogChanged()
{
    const QString titleTemplate = [this]() -> QString {
        switch (m_typeCatalog) {
        case CatalogType::Type::Doctors:
            return tr("Doctor (%1) %2");
        case CatalogType::Type::Nurses:
            return tr("As.medicală (%1) %2");
        case CatalogType::Type::Patients:
            return tr("Pacient (%1) %2");
        default:
            return QString();
        }
    }();

    if (titleTemplate.isEmpty())
        return;

    QString statusText;

    switch (m_statusCatalog) {
    case StatusObject::Unknow:
        statusText = tr("crearea");
        break;
    case StatusObject::ZeroWrite:
        statusText = tr("salvat");
        break;
    case StatusObject::DeletionMark:
        statusText = tr("marcat pentru eliminare");
        break;
    default:
        return;
    }

    setWindowTitle(titleTemplate.arg(statusText, "[*]"));
}

// *******************************************************************
// **************** VIZUALIZAREA ELEMETELOR FORMEI *******************

void CatalogDialog::catalogTypeChanged()
{
    if (m_typeCatalog == CatalogType::Type::Doctors) {
        ui->editPoliceMed->setVisible(false);
        ui->labelPoliceMed->setVisible(false);
        ui->editIDNP->setVisible(false);
        ui->labelIDNP->setVisible(false);
        ui->editAddress->setVisible(false);
        ui->labelAddress->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->labelDateEdit->setVisible(false);

        ui->tabWidget->setTabVisible(1, true);
        ui->imageSignature->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        ui->imageStamp->setText("<a href=\"#LoadImageStamp\">Apasa pentru a alege imaginea</a>");
        ui->imageStamp->setTextFormat(Qt::RichText);
        ui->imageStamp->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        connect(ui->imageSignature, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&CatalogDialog::onLinkActivatedForOpenImage), Qt::UniqueConnection);
        connect(ui->imageStamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&CatalogDialog::onLinkActivatedForOpenImage), Qt::UniqueConnection);
        connect(ui->btnClearSignature, &QToolButton::clicked,
                this, &CatalogDialog::clearImageSignature, Qt::UniqueConnection);
        connect(ui->btnClearStamp, &QToolButton::clicked,
                this, &CatalogDialog::clearImageStamp, Qt::UniqueConnection);
    }

    if (m_typeCatalog == CatalogType::Type::Nurses) {
        ui->editPoliceMed->setVisible(false);
        ui->labelPoliceMed->setVisible(false);
        ui->editIDNP->setVisible(false);
        ui->labelIDNP->setVisible(false);
        ui->editAddress->setVisible(false);
        ui->labelAddress->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->labelDateEdit->setVisible(false);
    }
}

// *******************************************************************
// **************** INSERAREA IMAGINEI SI ALTELE *********************

void CatalogDialog::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void CatalogDialog::dataWasModified()
{
    setWindowModified(true);
}

void CatalogDialog::clearImageSignature()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET signature = ? WHERE id = ?;");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_id);
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Semnatura este eliminat din baza de date."));
        popUp->show();
    } else {

        err.clear();
        err << tr("Eroare la eliminarea semnaturei din baza de date: ")
            << qry.lastError().text();

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Eliminarea semnaturei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }
}

void CatalogDialog::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET stamp = ? WHERE id = ?;");
    qry.addBindValue(QVariant());
    qry.addBindValue(m_id);
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImageStamp\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminat din baza de date."));
        popUp->show();
    } else {

        err.clear();
        err << tr("Eroare la eliminarea imaginei din baza de date: ")
            << qry.lastError().text();

        qCritical(logCritical()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Eliminarea imaginei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }
}

bool CatalogDialog::loadFile(const QString &fileName, const QString &link)
{
    // 1. Citim imaginea cu auto-transformare
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

    // 2. Convertim imaginea într-un QPixmap scalat o singură dată
    QPixmap scaledPixmap = QPixmap::fromImage(newImage)
                               .scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 3. Setăm imaginea în QLabel potrivit
    if (link == "#LoadImage")
        ui->imageSignature->setPixmap(scaledPixmap);
    else
        ui->imageStamp->setPixmap(scaledPixmap);

    // 4. Citim fișierul binar pentru stocare în baza de date
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << tr("Nu s-a putut deschide fișierul pentru citire: ") << fileName;
        return true; // Nu blocăm operația chiar dacă nu-l putem salva
    }

    QByteArray inByteArray = file.readAll();
    file.close();

    // 5. Pregătim interogarea SQL
    QString column = (link == "#LoadImage") ? "signature" : "stamp";

    QSqlQuery qry;
    qry.prepare(QStringLiteral("UPDATE doctors SET %1 = ? WHERE id = ?;").arg(column));
    qry.addBindValue(inByteArray.toBase64());
    qry.addBindValue(m_id);

    if (qry.exec()) {
        popUp->setPopupText(tr("Imaginea a fost salvată cu succes în baza de date."));
        popUp->show();
        qInfo(logInfo()) << QStringLiteral("A fost inserata imaginea - %1").arg(ui->editName->text());
    } else {

        err.clear();
        err << this->metaObject()->className()
            << "[loadFile]"
            << tr("Eroare la salvarea imaginii în baza de date %1")
                   .arg(qry.lastError().text().isEmpty()
                            ? ": eroarea indisponibila"
                            : ": " + qry.lastError().text());

        qWarning(logWarning()) << err;

        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea imaginei"));
        msg->setDetailedText(err.join("\n"));
        msg->exec();
        msg->deleteLater();
    }

    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty()
                                ? QDir::currentPath()
                                : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
                                                  ? QImageReader::supportedMimeTypes()
                                                  : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);

    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void CatalogDialog::onLinkActivatedForOpenImage(const QString &link)
{
    if (m_id == -1){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea validării"),
                                 tr("Pentru a încărca logotipul este necesar de salvat datele.<br>"
                                    "Doriți să salvați datele ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(styleBtnMessageBox);
        noButton->setStyleSheet(styleBtnMessageBox);
        cancelButton->setStyleSheet(styleBtnMessageBox);
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton)
            onWritingData();
        else if (messange_box.clickedButton() == noButton)
            return;
        else if (messange_box.clickedButton() == cancelButton)
            return;
    }

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
        if (! dialog.selectedFiles().constFirst().isEmpty())
            loadFile(dialog.selectedFiles().constFirst(), link);
    dialog.close();
}

void CatalogDialog::fullNameSplit()
{
    QString strFullName = ui->editFullName->text();
    static const QRegularExpression reg_listStr("\\s+");
    QStringList listStr = strFullName.split(reg_listStr);

    int n;

    ui->editName->clear();
    ui->editPrenume->clear();
    ui->editPatronimic->clear();

    for (n = 0; n < listStr.count(); ++ n) {
        if (n == 0){
            if (ui->editName->text() != listStr[n]){
                ui->editName->setText(listStr[n]);
            }
        } else if (n == 1){
            if (ui->editPrenume->text() != listStr[n]){
                ui->editPrenume->setText(listStr[n]);
            }
        } else if (n == 2){
            if (ui->editPatronimic->text() != listStr[n]){
                ui->editPatronimic->setText(listStr[n]);
            }
        }
    }
}

// *******************************************************************
// **************** VALIDAREA DATELOR ********************************

bool CatalogDialog::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    bool returnBool;

    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        returnBool = m_isNew
                         ? handleInsert("doctors", tr("Doctor"), "")
                         : handleUpdate("doctors", tr("Doctor"));
        break;
    case CatalogType::Type::Nurses:
        returnBool = m_isNew
                         ? handleInsert("nurses", tr("As.medicala"), "")
                         : handleUpdate("nurses", tr("As.medcala"));
        break;
    case CatalogType::Type::Patients:
        returnBool = m_isNew
                         ? handleInsert("patients", tr("Pacientul"), tr(" - anul na\310\231terii: %1")
                                                                         .arg(ui->dateEdit->date().toString("dd.MM.yyyy")))
                         : handleUpdate("patients", tr("Pacientul"));
        break;
    default:
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": nu a fost determinanta proprietatea 'typeCatalog' !!!");
        returnBool = false;
        break;
    }

    // emitem signal
    if (returnBool) {
        if (m_isNew) {
            emit catalogDialogCreated();
            emit catalogDialogCreatedReturnID(m_id); // in orderEcho crearea doctorului
            setIsNew(false); // setam itNew = false
        } else {
            emit catalogDialogChanged();
        }
    }

    // modificam forma
    setWindowModified(! returnBool); // schimbam proprietatea de modificare a datelor formai

    return returnBool;
}

void CatalogDialog::onWritingDataClose()
{
    if (m_statusCatalog == StatusObject::Unknow)
        setStatusCatalog(StatusObject::ZeroWrite);

    if (!onWritingData()){
        setStatusCatalog(StatusObject::Unknow);
        return;
    }
    QDialog::accept();
}

void CatalogDialog::connectionModified()
{
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged,
                this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);
    connect(ui->editComment, &QTextEdit::textChanged,
            this, &CatalogDialog::dataWasModified, Qt::UniqueConnection);

    connect(ui->editFullName, &QLineEdit::textChanged,
            this, &CatalogDialog::fullNameChanged, Qt::UniqueConnection);
    connect(ui->editFullName, &QLineEdit::editingFinished,
            this, &CatalogDialog::fullNameSplit, Qt::UniqueConnection);
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA DATELOR IN TABELE ********

bool CatalogDialog::controlRequiredObjects()
{
    if (ui->editFullName->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Nume, prenume, patronimic</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    if (ui->editName->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Nume</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    if (ui->editPrenume->text().isEmpty()){
        QMessageBox::warning(this,
                             tr("Verificarea datelor."),
                             tr("Nu este indicat \"<b>Prenume</b>\" obiectului !!!"),
                             QMessageBox::Ok);
        return false;
    }
    return true;
}

bool CatalogDialog::insertDataIntoTableByNameTable(const QString name_table)
{
    QVector<QVariant> data;

    /** introducem datele in container */
    if (name_table == "patients"){
        data.append(m_id);
        data.append(StatusObject::statusObjectToInt(m_statusCatalog));
        data.append(ui->editIDNP->text());
        data.append(ui->editName->text());
        data.append(ui->editPrenume->text());
        data.append(ui->editPatronimic->text().isEmpty()
                        ? QVariant()
                        : ui->editPatronimic->text());
        data.append(ui->editPoliceMed->text().isEmpty()
                        ? QVariant()
                        : ui->editPoliceMed->text());
        data.append(ui->dateEdit->date().toString("yyyy-MM-dd"));
        data.append(ui->editAddress->text().isEmpty()
                        ? QVariant()
                        : ui->editAddress->text());
    } else {
        data.append(m_id);
        data.append(0);
        data.append(ui->editName->text());
        data.append(ui->editPrenume->text());
        data.append(ui->editPatronimic->text().isEmpty()
                        ? QVariant()
                        : ui->editPatronimic->text());
    }

    /** comune */
    data.append(ui->editTelephone->text().isEmpty()
                    ? QVariant()
                    : ui->editTelephone->text());
    data.append(ui->editEmail->text().isEmpty()
                    ? QVariant()
                    : ui->editEmail->text());
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText());
    QUuid uuid = QUuid::createUuid();
    data.append(uuid.toRfc4122());

    /** determinam fisierul de solicitare */
    QString str_path;
    if (name_table == "patients") {
        str_path = ":/sql/queries/patients_insert.sql";
    } else {
        str_path = QString(":/sql/queries/%1_insert.sql").arg(name_table);
    }

    /** executam solicitarea */
    QString str_err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   str_path,
                                   data,
                                   &str_err))
    {
        // logarea
        qCritical(logCritical()) << str_err;

        // mesaj pu utilizator
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Inserarea datelor"));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();

        return false;
    }

    qInfo(logInfo()) << "CatalogDialog: inserare reușită; tabela=" << name_table
                     << "id=" << m_id;
    return true;
}

bool CatalogDialog::updateDataIntoTableByNameTable(const QString name_table)
{
    QVector<QVariant> data;

    /** introducem datele in container */
    if (name_table == "patients"){
        data.append(StatusObject::statusObjectToInt(m_statusCatalog)); // deletionMark
        data.append(ui->editIDNP->text());
        data.append(ui->editName->text());
        data.append(ui->editPrenume->text());
        data.append(ui->editPatronimic->text().isEmpty()
                        ? QVariant()
                        : ui->editPatronimic->text());
        data.append(ui->editPoliceMed->text().isEmpty()
                        ? QVariant()
                        : ui->editPoliceMed->text());
        data.append(ui->dateEdit->date().toString("yyyy-MM-dd"));
        data.append(ui->editAddress->text().isEmpty()
                        ? QVariant()
                        : ui->editAddress->text());
    } else {
        data.append(0);
        data.append(ui->editName->text());
        data.append(ui->editPrenume->text());
        data.append(ui->editPatronimic->text().isEmpty()
                        ? QVariant()
                        : ui->editPatronimic->text());
    }

    /** date comune */
    data.append(ui->editTelephone->text().isEmpty()
                    ? QVariant()
                    : ui->editTelephone->text());
    data.append(ui->editEmail->text().isEmpty()
                    ? QVariant()
                    : ui->editEmail->text());
    data.append(ui->editComment->toPlainText().isEmpty()
                    ? QVariant()
                    : ui->editComment->toPlainText());
    data.append(m_id);

    /** determinam fisierul de solicitare */
    QString str_path;
    if (name_table == "patients") {
        str_path = ":/sql/queries/patients_update.sql";
    } else {
        str_path = QString(":/sql/queries/%1_update.sql").arg(name_table);
    }

    /** executam solicitarea */
    QString str_err;
    if (! m_db.execPreparedFromFile(m_db.getDatabase(),
                                   str_path,
                                   data,
                                   &str_err))
    {
        // logarea
        qCritical(logCritical()) << str_err;

        // prezentarea msg utilizatorului
        CustomMessage *msg = new CustomMessage(this);
        msg->setWindowTitle(QGuiApplication::applicationDisplayName());
        msg->setTextTitle(tr("Actualizarea datelor"));
        msg->setDetailedText(str_err);
        msg->exec();
        msg->deleteLater();

        return false;
    }

    qInfo(logInfo()) << "CatalogDialog: actualizare reușită; tabela=" << name_table
                     << "id=" << m_id;
    return true;
}

bool CatalogDialog::objectExistsInTableByName(const QString name_table)
{
    QSqlQuery qry;
    QString str_qry = m_db.getTextSQL(":/sql/queries/check_person_exists.sql")
                          .arg(name_table);
    qry.prepare(str_qry);
    qry.addBindValue(ui->editName->text());
    qry.addBindValue(ui->editPrenume->text());
    qry.addBindValue(ui->editPatronimic->text());
    if (qry.exec() && qry.next()){
        int _bool = qry.value(0).toInt();
        return (_bool > 0) ? true : false;
    } else {
        return false;
    }
}

bool CatalogDialog::confirmIfDuplicateExist(const QString &name_table, const QString &type_label, const QString &extra_info)
{
    // verificam daca este persoana
    if (! objectExistsInTableByName(name_table))
        return true;

    // formam textul
    QString text = tr("%1 exist\304\203 \303\256n baza de date:<br>"
                      " - nume: <b>%2</b><br>"
                      " - prenume: <b>%3</b><br>"
                      " - patronimic: <b>%4</b><br>"
                      "%5 <br>"
                      "Dori\310\233i s\304\203 continua\310\233i validarea ?")
                       .arg(type_label,
                            ui->editName->text(),
                            ui->editPrenume->text(),
                            ui->editPatronimic->text(),
                            extra_info);

    // prezentam mesaj
    QMessageBox messange_box(QMessageBox::Question,
                             tr("Verificarea datelor"),
                             text, QMessageBox::NoButton, this);
    QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
    yesButton->setStyleSheet(styleBtnMessageBox);
    noButton->setStyleSheet(styleBtnMessageBox);
    messange_box.exec();

    return messange_box.clickedButton() == yesButton;
}

bool CatalogDialog::handleInsert(const QString &name_table, const QString &type_label, const QString &extra_info)
{
    // verificam daca este duplicat
    if (! confirmIfDuplicateExist(name_table, type_label, extra_info))
        return false;

    // ne determinam cu ID
    if (m_id <= 0)
        setId(m_db.getLastIdForTable(name_table) + 1);

    // inseram datele
    if (insertDataIntoTableByNameTable(name_table)) {
        // !!! logarea si mesajul de inserare cu succes vezi in [insertDataIntoTableByNameTable]
        return true; // returnam true
    } else {
        // !!! prezentarea mesajului de eroare in functia [insertDataIntoTableByNameTable]
        return false;
    }
}

bool CatalogDialog::handleUpdate(const QString &name_table, const QString &type_label)
{
    Q_UNUSED(type_label);

    if (updateDataIntoTableByNameTable(name_table)) {
        // !!! logarea si mesajul de modificarea datelor vezi in [updateDataIntoTableByNameTable]
        return true;
    } else {
        // !!! prezentarea mesajului de eroare in functia [updateDataIntoTableByNameTable]
        return false;
    }
}

bool CatalogDialog::handleDeletionMark(QString &err)
{
    err.clear();

    QString tableName;
    QString catalogName;

    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        tableName   = QStringLiteral("doctors");
        catalogName = tr("doctorului");
        break;
    case CatalogType::Type::Nurses:
        tableName   = QStringLiteral("nurses");
        catalogName = tr("asistentei medicale");
        break;
    case CatalogType::Type::Patients:
        tableName   = QStringLiteral("patients");
        catalogName = tr("pacientului");
        break;
    default:
        err = tr("Tipul catalogului nu este determinat.");
        return false;
    }

    if (m_id <= 0) {
        err = tr("Nu este determinat ID-ul %1.").arg(catalogName);
        return false;
    }

    QSqlQuery q;
    q.prepare(QStringLiteral("UPDATE %1 SET deletionMark = :deletionMark WHERE id = :id")
                  .arg(tableName));
    q.bindValue(QStringLiteral(":deletionMark"),
                StatusObject::statusObjectToInt(m_statusCatalog));
    q.bindValue(QStringLiteral(":id"), m_id);

    if (!q.exec()) {
        err = tr("Eroare SQL: %1").arg(q.lastError().text());

        qCritical(logCritical()).noquote()
            << "SQL error:" << q.lastError().text()
            << "\nQuery:" << q.lastQuery()
            << "\nTable:" << tableName
            << "\nID:" << m_id;

        return false;
    }

    emit catalogDialogDeletedMark();
    return true;
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void CatalogDialog::closeEvent(QCloseEvent *event)
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
        yesButton->setStyleSheet(styleBtnMessageBox);
        noButton->setStyleSheet(styleBtnMessageBox);
        cancelButton->setStyleSheet(styleBtnMessageBox);
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

void CatalogDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Creiază utilizator nou %1").arg("[*]"));
    }
}

void CatalogDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
    }
}
