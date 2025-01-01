#include "catgeneral.h"
#include "ui_catgeneral.h"

#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>

static const int max_length_comment = 255; // lungimea maxima a cometariului

CatGeneral::CatGeneral(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CatGeneral)
{
    ui->setupUi(this);

    db    = new DataBase(this); // alocam memoria p-u salvarea datelor in BD
    popUp = new PopUp(this);    // alocam memoria p-u mesaje

    connect(this, &CatGeneral::typeCatalogChanged, this, &CatGeneral::slot_typeCatalogChanged);
    connect(this, &CatGeneral::IdChanged, this, &CatGeneral::slot_IdChanged);
    connect(this, &CatGeneral::ItNewChanged, this, &CatGeneral::slot_ItNewChanged);

    connect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    connect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::controlLengthComment); // limitarea caracterilor la comentariu

    ui->tabWidget->setTabVisible(1, false);

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
        connect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    ui->btnOK->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));  // comenzi rapide la tastatura
    ui->btnWrite->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    ui->btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));

    connect(ui->btnOK, &QAbstractButton::clicked, this, &CatGeneral::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &CatGeneral::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &CatGeneral::close);
}

CatGeneral::~CatGeneral()
{
    delete db;
    delete popUp;
    delete ui;
}

// *******************************************************************
// **************** INSERAREA, ACTUALIZAREA DATELOR IN TABELE ********

bool CatGeneral::controlRequiredObjects()
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

bool CatGeneral::insertDataIntoTableByNameTable(const QString name_table)
{
    QSqlQuery qry;
    if (name_table == "pacients"){
        qry.prepare("INSERT INTO " + name_table + " ("
                                                  "id,"
                                                  "deletionMark,"
                                                  "IDNP,"
                                                  "name,"
                                                  "fName,"
                                                  "mName,"
                                                  "medicalPolicy,"
                                                  "birthday,"
                                                  "address,"
                                                  "telephone,"
                                                  "email,"
                                                  "comment) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
        qry.addBindValue(m_Id);
        qry.addBindValue(0);
        qry.addBindValue(ui->editIDNP->text());
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text());
        qry.addBindValue(ui->editPoliceMed->text());
        qry.addBindValue(ui->dateEdit->date().toString("yyyy-MM-dd"));
        qry.addBindValue(ui->editAddress->text());
        qry.addBindValue(ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText());
    } else {
        qry.prepare("INSERT INTO " + name_table + " ("
                                                  "id,"
                                                  "deletionMark,"
                                                  "name,"
                                                  "fName,"
                                                  "mName,"
                                                  "telephone,"
                                                  "email,"
                                                  "comment) VALUES (?,?,?,?,?,?,?,?);");
        qry.addBindValue(m_Id);
        qry.addBindValue(0);
        qry.addBindValue(ui->editName->text());
        qry.addBindValue(ui->editPrenume->text());
        qry.addBindValue(ui->editPatronimic->text());
        qry.addBindValue(ui->editTelephone->text());
        qry.addBindValue(ui->editEmail->text());
        qry.addBindValue(ui->editComment->toPlainText());
    }
    if (qry.exec()){
        qInfo(logInfo()) << tr("Datele obiectului '%1' cu id='%2' au fost salvate cu succes in tabela '%3'.")
                                .arg(ui->editFullName->text(), QString::number(m_Id), name_table);
        return true;
    } else {
        qCritical(logCritical()) << tr("Eroare la inserarea datelor obiectului '%1' cu id='%2' in tabela '%3'- %4")
                                        .arg(ui->editFullName->text(), QString::number(m_Id), name_table, qry.lastError().text());
        return false;
    }
}

bool CatGeneral::updateDataIntoTableByNameTable(const QString name_table)
{
    QSqlQuery qry;
    if (name_table == "pacients"){
        qry.prepare("UPDATE " + name_table + " SET "
                                             "deletionMark  = :deletionMark,"
                                             "IDNP          = :IDNP,"
                                             "name          = :name,"
                                             "fName         = :fName,"
                                             "mName         = :mName,"
                                             "medicalPolicy = :medicalPolicy,"
                                             "birthday      = :birthday,"
                                             "address       = :address,"
                                             "telephone     = :telephone,"
                                             "email         = :email,"
                                             "comment       = :comment "
                                             "WHERE id = :id;");
        qry.bindValue(":id",            m_Id);
        qry.bindValue(":deletionMark",  0);
        qry.bindValue(":IDNP",          ui->editIDNP->text());
        qry.bindValue(":name",          ui->editName->text());
        qry.bindValue(":fName",         ui->editPrenume->text());
        qry.bindValue(":mName",         ui->editPatronimic->text());
        qry.bindValue(":medicalPolicy", ui->editPoliceMed->text());
        qry.bindValue(":birthday",      ui->dateEdit->date().toString("yyyy-MM-dd"));
        qry.bindValue(":address",       ui->editAddress->text());
        qry.bindValue(":telephone",     ui->editTelephone->text());
        qry.bindValue(":email",         ui->editEmail->text());
        qry.bindValue(":comment",       ui->editComment->toPlainText());
    } else {
        qry.prepare("UPDATE " + name_table + " SET "
                                             "deletionMark = :deletionMark,"
                                             "name         = :name,"
                                             "fName        = :fName,"
                                             "mName        = :mName,"
                                             "telephone    = :telephone,"
                                             "email        = :email,"
                                             "comment      = :comment "
                                             "WHERE id = :id;");
        qry.bindValue(":id",           m_Id);
        qry.bindValue(":deletionMark", 0);
        qry.bindValue(":name",         ui->editName->text());
        qry.bindValue(":fName",        ui->editPrenume->text());
        qry.bindValue(":mName",        ui->editPatronimic->text());
        qry.bindValue(":telephone",    ui->editTelephone->text());
        qry.bindValue(":email",        ui->editEmail->text());
        qry.bindValue(":comment",      ui->editComment->toPlainText());
    }

    if (qry.exec()){
        qInfo(logInfo()) << tr("Datele obiectului '%1' cu id='%2' au fost modificate cu succes in tabela '%3'.")
                                .arg(ui->editFullName->text(), QString::number(m_Id), name_table);
        return true;
    } else {
        qCritical(logCritical()) << tr("Eroare la modificarea datelor obiectului '%1' cu id='%2' in tabela '%3' - %4")
                                        .arg(ui->editFullName->text(), QString::number(m_Id), name_table, qry.lastError().text());
        return false;
    }
}

bool CatGeneral::objectExistsInTableByName(const QString name_table)
{
    QSqlQuery qry;
    qry.prepare("SELECT COUNT(name) FROM " + name_table + " WHERE "
                "name =  :name AND "
                "fName = :fName AND "
                "mName = :mName AND "
                "deletionMark = 0;");
    qry.bindValue(":name",  ui->editName->text());
    qry.bindValue(":fName", ui->editPrenume->text());
    qry.bindValue(":mName", ui->editPatronimic->text());
    if (qry.exec() && qry.next()){
        int _bool = qry.value(0).toInt();
        return (_bool > 0) ? true : false;
    } else {
        return false;
    }
}

// *******************************************************************
// **************** INSERAREA IMAGINEI SI ALTELE *********************

void CatGeneral::loadImageOpeningCatalog()
{
    QByteArray outByteArray = db->getOutByteArrayImage("doctors", "signature", "id", m_Id);
    QPixmap outPixmap;
    if (! outByteArray.isEmpty() && outPixmap.loadFromData(outByteArray))
        ui->imageSignature->setPixmap(outPixmap.scaled(200,200));

    QByteArray outByteArray_stamp = db->getOutByteArrayImage("doctors", "stamp", "id", m_Id);
    QPixmap outPixmap_stamp;
    if (! outByteArray_stamp.isEmpty() && outPixmap_stamp.loadFromData(outByteArray_stamp))
        ui->imageStamp->setPixmap(outPixmap_stamp.scaled(200,200));
}

void CatGeneral::controlLengthComment()
{
    if (ui->editComment->toPlainText().length() > max_length_comment)
        ui->editComment->textCursor().deletePreviousChar();
}

void CatGeneral::dataWasModified()
{
    setWindowModified(true);
}

void CatGeneral::clearImageSignature()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET signature = :value WHERE id = :id;");
    qry.bindValue(":id",    m_Id);
    qry.bindValue(":value", QVariant());
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Semnatura este eliminat din baza de date."));
        popUp->show();
    } else {
        qCritical(logCritical()) << tr("Eroare la eliminarea semnaturei din baza de date: ") << qry.lastError().text();
    }
}

void CatGeneral::clearImageStamp()
{
    QSqlQuery qry;
    qry.prepare("UPDATE doctors SET stamp = :value WHERE id = :id;");
    qry.bindValue(":id",    m_Id);
    qry.bindValue(":value", QVariant());
    if (qry.exec()){
        ui->imageSignature->setText("<a href=\"#LoadImageStamp\">Apasa pentru a alege imaginea</a>");
        ui->imageSignature->setTextFormat(Qt::RichText);
        ui->imageSignature->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Imaginea este eliminat din baza de date."));
        popUp->show();
    } else {
        qCritical(logCritical()) << tr("Eroare la eliminarea imaginei din baza de date: ") << qry.lastError().text();
    }
}

bool CatGeneral::loadFile(const QString &fileName, const QString &link)
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

    if (link == "#LoadImage")
        ui->imageSignature->setPixmap(QPixmap::fromImage(newImage).scaled(200,200));
    else
        ui->imageStamp->setPixmap(QPixmap::fromImage(newImage).scaled(200,200));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return true;
    QByteArray inByteArray = file.readAll();

    QSqlQuery qry;
    qry.prepare(QString("UPDATE doctors SET %1 = :value WHERE id = :id;").arg((link == "#LoadImage") ? "signature" : "stamp"));
    qry.bindValue(":id",    m_Id);
    qry.bindValue(":value", inByteArray.toBase64());
    if (qry.exec()){
        popUp->setPopupText(tr("Imaginea este salvat cu succes în baza de date."));
        popUp->show();
    } else {
        qDebug() << tr("Eroare de inserare imaginei in baza de date: ") << qry.lastError().text();
    }
    file.close();

    return true;
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
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void CatGeneral::onLinkActivatedForOpenImage(const QString &link)
{
    if (m_Id == -1){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Verificarea validării"),
                                 tr("Pentru a încărca logotipul este necesar de salvat datele.<br>"
                                    "Doriți să salvați datele ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        QPushButton *cancelButton = messange_box.addButton(tr("Anulare"), QMessageBox::RejectRole);
        yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
        noButton->setStyleSheet(db->getStyleForButtonMessageBox());
        cancelButton->setStyleSheet(db->getStyleForButtonMessageBox());
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

// *******************************************************************
// **************** PROCESAREA SLOT-URILOR ***************************

void CatGeneral::slot_typeCatalogChanged()
{
    switch (m_typeCatalog) {
    case Doctors:
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
                QOverload<const QString &>::of(&CatGeneral::onLinkActivatedForOpenImage));
        connect(ui->imageStamp, QOverload<const QString &>::of(&QLabel::linkActivated), this,
                QOverload<const QString &>::of(&CatGeneral::onLinkActivatedForOpenImage));
        connect(ui->btnClearSignature, &QToolButton::clicked, this, &CatGeneral::clearImageSignature);
        connect(ui->btnClearStamp, &QToolButton::clicked, this, &CatGeneral::clearImageStamp);
        break;
    case Nurses:
        ui->editPoliceMed->setVisible(false);
        ui->labelPoliceMed->setVisible(false);
        ui->editIDNP->setVisible(false);
        ui->labelIDNP->setVisible(false);
        ui->editAddress->setVisible(false);
        ui->labelAddress->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->labelDateEdit->setVisible(false);
        break;
    case Pacients:
        break;
    default:
        qWarning(logWarning()) << this->metaObject()->className()
            << tr(": nu a fost determinata proprietatea 'typeCatalog' !!!");
        break;
    }
    slot_ItNewChanged();
}

void CatGeneral::slot_IdChanged()
{
    if (m_itNew)
        return;

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    for (int n = 0; n < list.count(); n++) {
        disconnect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    disconnect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    disconnect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    disconnect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    disconnect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);

    QMap<QString, QString> _items;
    switch (m_typeCatalog) {
    case Doctors:
        if (db->getObjectDataById("doctors", m_Id, _items)){
            ui->editName->setText(_items.constFind("name").value());           // determinam datele obiectului
            ui->editPrenume->setText(_items.constFind("fName").value());
            ui->editPatronimic->setText(_items.constFind("mName").value());
            ui->editTelephone->setText(_items.constFind("telephone").value());
            ui->editEmail->setText(_items.constFind("email").value());
            ui->editComment->setText(_items.constFind("comment").value());
            ui->editFullName->setText(ui->editName->text() + " " + ui->editPrenume->text() + " " + ui->editPatronimic->text());
        }
        loadImageOpeningCatalog();
        break;
    case Nurses:
        if (db->getObjectDataById("nurses", m_Id, _items)){
            ui->editName->setText(_items.constFind("name").value());           // determinam datele obiectului
            ui->editPrenume->setText(_items.constFind("fName").value());
            ui->editPatronimic->setText(_items.constFind("mName").value());
            ui->editTelephone->setText(_items.constFind("telephone").value());
            ui->editEmail->setText(_items.constFind("email").value());
            ui->editComment->setText(_items.constFind("comment").value());
            ui->editFullName->setText(ui->editName->text() + " " + ui->editPrenume->text() + " " + ui->editPatronimic->text());
        }
        break;
    case Pacients:
        if (db->getObjectDataById("pacients", m_Id, _items)){
            ui->editIDNP->setText(_items.constFind("IDNP").value());     // determinam datele obiectului
            ui->editName->setText(_items.constFind("name").value());
            ui->editPrenume->setText(_items.constFind("fName").value());
            ui->editPatronimic->setText(_items.constFind("mName").value());
            ui->editPoliceMed->setText(_items.constFind("medicalPolicy").value());
            ui->dateEdit->setDate(QDate::fromString(_items.constFind("birthday").value(), "yyyy-MM-dd"));
            ui->editAddress->setText(_items.constFind("address").value());
            ui->editTelephone->setText(_items.constFind("telephone").value());
            ui->editEmail->setText(_items.constFind("email").value());
            ui->editComment->setText(_items.constFind("comment").value());
            ui->editFullName->setText(ui->editName->text() + " " + ui->editPrenume->text() + " " + ui->editPatronimic->text());
        }
        break;
    default:
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": nu a fost determinanta proprietatea 'typeCatalog' !!!");
        break;
    }

    ui->editName->setFocus(); // modifica forma - isWindowModified() = true
                              // cand este focusat initial pe fullName
    setFullName(ui->editFullName->text());

    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QLineEdit::textChanged, this, &CatGeneral::dataWasModified);
    }
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &CatGeneral::dataWasModified);
    connect(ui->editComment, &QTextEdit::textChanged, this, &CatGeneral::dataWasModified);

    connect(ui->editFullName, &QLineEdit::textChanged, this, &CatGeneral::fullNameChanged);
    connect(ui->editFullName, &QLineEdit::editingFinished, this, &CatGeneral::fullNameSplit);
}

void CatGeneral::slot_ItNewChanged()
{
    if (m_itNew){
        switch (m_typeCatalog) {
        case Doctors:
            setWindowTitle(tr("Doctor (crearea) %1").arg("[*]"));
            break;
        case Nurses:
            setWindowTitle(tr("As.medicală (crearea) %1").arg("[*]"));
            break;
        case Pacients:
            setWindowTitle(tr("Pacient (crearea) %1").arg("[*]"));
            break;
        default:
            break;
        }
    } else {
        switch (m_typeCatalog) {
        case Doctors:
            setWindowTitle(tr("Doctor (salvat) %1").arg("[*]"));
            break;
        case Nurses:
            setWindowTitle(tr("As.medicală (salvată) %1").arg("[*]"));
            break;
        case Pacients:
            setWindowTitle(tr("Pacient (salvat) %1").arg("[*]"));
            break;
        default:
            break;
        }
    }
}

void CatGeneral::fullNameChanged()
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

void CatGeneral::fullNameSplit()
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

bool CatGeneral::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    bool returnBool;

    switch (m_typeCatalog) {
    case Doctors:

        if (m_itNew){ // daca itNew = true:

            if (objectExistsInTableByName("doctors")){     // 1. verificam daca este obiectul in BD dupa rechizutul 'nume'
                QMessageBox messange_box(QMessageBox::Question,
                                         tr("Verificarea datelor"),
                                         tr("Doctor exist\304\203 \303\256n baza de date:<br>"
                                            " - nume: <b>%1</b><br>"
                                            " - prenume: <b>%2</b><br>"
                                            " - patronimic: <b>%3</b><br>"
                                            ". <br>Dori\310\233i s\304\203 continua\310\233i validarea ?")
                                             .arg(ui->editName->text(), ui->editPrenume->text(), ui->editPatronimic->text()),
                                        QMessageBox::NoButton, this);
                QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
                QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
                yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
                noButton->setStyleSheet(db->getStyleForButtonMessageBox());
                messange_box.exec();

                if (messange_box.clickedButton() == yesButton){
                } else if (messange_box.clickedButton() == noButton) {
                    returnBool = false;
                    break;
                }
            }

            if (m_Id == -1 || m_Id == 0)
                setId(db->getLastIdForTable("doctors") + 1);

            if (insertDataIntoTableByNameTable("doctors")){
                popUp->setPopupText(tr("Obiectul <b>%1</b><br> a fost creat cu succes.").arg(ui->editFullName->text()));
                popUp->show();
                setItNew(false);
                returnBool = true;
            } else {
                QMessageBox::warning(this, tr("Crearea obiectului."),
                                     tr("Salvarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                setId(-1);
                returnBool = false;
            }

            if (returnBool == true)
                emit createCatGeneral();

        } else { // daca itNew = false

            if (updateDataIntoTableByNameTable("doctors")){ // modificam datele obiectului
                popUp->setPopupText(tr("Obiectul <b>%1</b><br> a fost modificat cu succes.").arg(ui->editFullName->text()));
                popUp->show();
                returnBool = true;
            } else {
                QMessageBox::warning(this, tr("Modificarea datelor."),
                                     tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                returnBool = false;
            }

            if (returnBool == true)
                emit changedCatGeneral();
        }

        break;

    case Nurses:

        if (m_itNew){

            if (objectExistsInTableByName("nurses")){     // 1. verificam daca este obiectul in BD dupa rechizutul 'nume'
                QMessageBox messange_box(QMessageBox::Question,
                                         tr("Verificarea datelor"),
                                         tr("As.medicala exist\304\203 \303\256n baza de date:<br>"
                                            " - nume: <b>%1</b><br>"
                                            " - prenume: <b>%2</b><br>"
                                            " - patronimic: <b>%3</b><br>"
                                            ". <br>Dori\310\233i s\304\203 continua\310\233i validarea ?")
                                             .arg(ui->editName->text(), ui->editPrenume->text(), ui->editPatronimic->text()),
                                        QMessageBox::NoButton, this);
                QPushButton *yesButton  = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
                QPushButton *noButton   = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
                yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
                noButton->setStyleSheet(db->getStyleForButtonMessageBox());
                messange_box.exec();

                if (messange_box.clickedButton() == yesButton){
                } else if (messange_box.clickedButton() == noButton) {
                    returnBool = false;
                    break;
                }
            }

            if (m_Id == -1 || m_Id == 0)
                setId(db->getLastIdForTable("nurses") + 1);

            if (insertDataIntoTableByNameTable("nurses")){
                popUp->setPopupText(tr("Obiectul <b>%1</b><br> a fost creat cu succes.").arg(ui->editFullName->text()));
                popUp->show();
                setItNew(false);
                setWindowTitle(tr("As.medicală %1 %2").arg((m_itNew) ? tr("(crearea)"): tr("(salvat)"), "[*]"));
                returnBool = true;
            } else {
                QMessageBox::warning(this, tr("Crearea obiectului."),
                                     tr("Salvarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                setId(-1);
                returnBool = false;
            }

            if (returnBool == true)
                emit createCatGeneral();

        } else {

            if (updateDataIntoTableByNameTable("nurses")){           // modificam datele obiectului
                popUp->setPopupText(tr("Datele obiectului <b>%1</b><br> au fost modificate cu succes.").arg(ui->editFullName->text()));
                popUp->show();
                returnBool = true;
            } else {
                QMessageBox::warning(this, tr("Modificarea datelor."),
                                     tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                returnBool = false;
            }

            if (returnBool == true)
                emit changedCatGeneral();
        }

        break;

    case Pacients:

        if (m_itNew){

            if (objectExistsInTableByName("pacients")){
                QMessageBox messange_box(QMessageBox::Question,
                                         tr("Verificarea datelor"),
                                         tr("Pacientul exist\304\203 \303\256n baza de date:<br>"
                                            " - nume: <b>%1</b><br>"
                                            " - prenume: <b>%2</b><br>"
                                            " - patronimic: <b>%3</b><br>"
                                            " - anul na\310\231terii: <b>%4</b><br>"
                                            ". <br>Dori\310\233i s\304\203 continua\310\233i validarea ?")
                                        .arg(ui->editName->text(), ui->editPrenume->text(), ui->editPatronimic->text(), ui->dateEdit->date().toString("dd.MM.yyyy")),
                                        QMessageBox::NoButton, this);
                QPushButton *yesButton = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
                QPushButton *noButton  = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
                yesButton->setStyleSheet(db->getStyleForButtonMessageBox());
                noButton->setStyleSheet(db->getStyleForButtonMessageBox());
                messange_box.exec();

                if (messange_box.clickedButton() == yesButton){
                } else if (messange_box.clickedButton() == noButton) {
                    returnBool = false;
                    break;
                }
            }

            if (m_Id == -1 || m_Id == 0)
                setId(db->getLastIdForTable("pacients") + 1);

            if (insertDataIntoTableByNameTable("pacients")){
                setItNew(false);
                setWindowTitle(tr("Pacientul %1 %2").arg((m_itNew) ? tr("(crearea)"): tr("(salvat)"), "[*]"));
                returnBool = true;
                popUp->setPopupText(tr("Obiectul <b>%1</b><br> a fost creat cu succes.").arg(ui->editFullName->text()));
                popUp->show();
            } else {
                QMessageBox::warning(this,
                                     tr("Crearea obiectului."),
                                     tr("Salvarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                setId(-1);
                returnBool = false;
            }

            if (returnBool == true)
                emit createCatGeneral();

        } else {

            if (updateDataIntoTableByNameTable("pacients")){           // modificam datele obiectului
                returnBool = true;
            } else {
                QMessageBox::warning(this,
                                     tr("Modificarea datelor."),
                                     tr("Modificarea datelor obiectului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                        "Adresați-vă administratorului aplicației.")
                                     .arg(ui->editFullName->text()), QMessageBox::Ok);
                returnBool = false;
            }

            if (returnBool == true)
                emit changedCatGeneral();
        }

        break;

    default:
        qWarning(logWarning()) << this->metaObject()->className()
                               << tr(": nu a fost determinanta proprietatea 'typeCatalog' !!!");
        returnBool = false;
        break;
    }

    setWindowModified(!returnBool); // schimbam proprietatea de modificare a datelor formai

    return returnBool;
}

void CatGeneral::onWritingDataClose()
{
   if (onWritingData())
       QDialog::accept();
}

// *******************************************************************
// **************** EVENIMENTELE FORMEI ******************************

void CatGeneral::closeEvent(QCloseEvent *event)
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

void CatGeneral::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Creiază utilizator nou %1").arg("[*]"));
    }
}

void CatGeneral::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
      this->focusNextChild();
    }
}
