#include "userpreferences.h"
#include "ui_userpreferences.h"
#include "version.h"

#include <QImageReader>
#include <QImageWriter>

UserPreferences::UserPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserPreferences)
{
    ui->setupUi(this);

    setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    setWindowIcon(QIcon(":/img/settings_x32.png"));

    // alocam memoria
    db     = new DataBase(this);
    popUp  = new PopUp(this);

    setListWidget(); // setam list widget

    ui->brandUSG->setMaxLength(200);
    ui->versionApp->setText(VER);
    ui->versionApp->setEnabled(false);

    ui->dockWidget->close(); // ascundem panel pu informatii

    initSetModels(); // setam modelurile

    initConnections(); // connection

    ui->btnClearLogo->setVisible(false);
    ui->image_logo->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea<br>(se recomanda 400x50 px)</a>");
    ui->image_logo->setTextFormat(Qt::RichText);
    ui->image_logo->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->image_logo, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&UserPreferences::onLinkActivatedForOpenImage));

#if defined(Q_OS_WIN)
    ui->frame->setStyle(style_fusion);
    ui->frame_2->setStyle(style_fusion);
#endif
}

UserPreferences::~UserPreferences()
{
    delete popUp;
    delete model_users;
    delete model_doctors;
    delete model_nurses;
    delete model_organizations;
    delete db;
    delete ui;
}

// **********************************************************************************
// --- procesarea modificarii formei si a validarii datelor

void UserPreferences::dataWasModified()
{
    setWindowModified(true);
}

bool UserPreferences::onWritingData()
{
    if (! controlRequiredObjects())
        return false;

    if (! existRecordInTable()){
        if (! insertDataIntoTable())
            return false;
    } else {
        if (! updateDataIntoTable())
            return false;
    }

    return true;
}

void UserPreferences::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

// **********************************************************************************
// --- procesarea slot-urilor

void UserPreferences::slot_IdChanged()
{
    if (m_Id == idx_unknow)
        return;

    //--------------------------------------------------------------
    // deconectarea pu a nu modifica forma

    disconnectCombo();
    disconnectionCheckBox();

    //--------------------------------------------------------------
    // setam 'comboUsers' dupa ID

    auto index_user = model_users->match(model_users->index(0, 0), Qt::UserRole, m_Id, 1, Qt::MatchExactly);
    if(!index_user.isEmpty())
        ui->comboUsers->setCurrentIndex(index_user.first().row());

    //--------------------------------------------------------------
    // tabela 'constants'

    QMap<QString, QString> items;
    if (db->getObjectDataByMainId("constants", "id_users", m_Id, items)){
        int id_doctor        = items.constFind("id_doctors").value().toInt();
        int id_nurse         = items.constFind("id_nurses").value().toInt();
        int id_organization  = items.constFind("id_organizations").value().toInt();

        if (id_doctor > idx_write)
            setIdDoctor(id_doctor);
        if (id_nurse > idx_write)
            setIdNurse(id_nurse);
        if (id_organization > idx_write)
            setIdOrganization(id_organization);

        QString brandUSG = items.constFind("brandUSG").value();
        if (! brandUSG.isEmpty())
            ui->brandUSG->setText(brandUSG);
    }

    //--------------------------------------------------------------
    // tabela 'userPreferences'

    items.clear();
    if (db->getObjectDataByMainId("userPreferences", "id_users", m_Id, items)){
        if (items.count() > 0){
            ui->check_showQuestionClosingApp->setChecked(items.constFind("showQuestionCloseApp").value().toInt());
            ui->check_showDesignerMenuPrint->setChecked(items.constFind("showDesignerMenuPrint").value().toInt());
            ui->check_showUserManual->setChecked(items.constFind("showUserManual").value().toInt());
            ui->check_splitFullNamePatient->setChecked(items.constFind("order_splitFullName").value().toInt());
            ui->check_newVersion->setChecked(items.constFind("checkNewVersionApp").value().toInt());
            ui->check_databasesArchiving->setChecked(items.constFind("databasesArchiving").value().toInt());
            ui->showAsistantHelper->setChecked(items.constFind("showAsistantHelper").value().toInt());
            ui->updateListDoc->setValue(items.constFind("updateListDoc").value().toInt());
        }
    }

    //--------------------------------------------------------------
    // setam logotipul

    QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", m_Id);
    QPixmap outPixmap = QPixmap();
    if (outPixmap.loadFromData(outByteArray)){
        ui->btnClearLogo->setVisible(true);
        ui->image_logo->setPixmap(outPixmap.scaled(400, 50));
    } else {
        ui->btnClearLogo->setVisible(false);
    }

    //--------------------------------------------------------------
    // conectarea

    connectionsCombo();
    connectionCheckBox();
}

void UserPreferences::slot_IdDoctorChanged()
{
    auto index_doctor = model_doctors->match(model_doctors->index(0, 0), Qt::UserRole, m_idDoctor, 1, Qt::MatchExactly);
    if (! index_doctor.isEmpty())
        ui->comboDoctors->setCurrentIndex(index_doctor.first().row());
}

void UserPreferences::slot_IdNurseChanged()
{
    auto index_nurse  = model_nurses->match(model_nurses->index(0, 0), Qt::UserRole, m_idNurse, 1, Qt::MatchExactly);
    if (! index_nurse.isEmpty())
        ui->comboNurses->setCurrentIndex(index_nurse.first().row());
}

void UserPreferences::slot_IdChangedOrganization()
{
    auto index_organization = model_organizations->match(model_organizations->index(0, 0), Qt::UserRole, m_IdOrganization, 1, Qt::MatchExactly);
    if (! index_organization.isEmpty()){
        ui->comboOrganizations->setCurrentIndex(index_organization.first().row());
    }
}

// **********************************************************************************
// --- procesarea signalurilor combobox-urilor si a listWidget

void UserPreferences::activatedComboUsers(const int index)
{
    const int id_user = ui->comboUsers->itemData(index, Qt::UserRole).toInt();
    setId(id_user);
    dataWasModified();
}

void UserPreferences::activatedComboDoctors(const int index)
{
    const int id_doctor = ui->comboDoctors->itemData(index, Qt::UserRole).toInt();
    setIdDoctor(id_doctor);
    dataWasModified();
}

void UserPreferences::activatedComboNurses(const int index)
{
    const int id_nurse = ui->comboNurses->itemData(index, Qt::UserRole).toInt();
    setIdNurse(id_nurse);
    dataWasModified();
}

void UserPreferences::activatedComboOrganizations(const int index)
{
    const int id_organization = ui->comboOrganizations->itemData(index, Qt::UserRole).toInt();
    setIdOrganization(id_organization);
    dataWasModified();
}

// **********************************************************************************
// --- procesarea logotipului, inserarea in BD

bool UserPreferences::loadFile(const QString &fileName)
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
    ui->image_logo->setPixmap(QPixmap::fromImage(newImage).scaled(400,50));
    ui->btnClearLogo->setVisible(true);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return true;
    QByteArray inByteArray = file.readAll();

    QSqlQuery qry;
    qry.prepare(QString("UPDATE constants SET logo = '%2' WHERE id_users = '%1';")
                .arg(QString::number(m_Id), inByteArray.toBase64()));
    if (qry.exec()){
        popUp->setPopupText(tr("Logotipul este salvat cu succes în baza de date."));
        popUp->show();
    } else {
        qDebug() << tr("Eroare de inserare a logotipului in baza de date:\n") << qry.lastError();
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

void UserPreferences::onLinkActivatedForOpenImage(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_Id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea validării"),
                                     tr("Pentru a încărca logotipul este necesar de salvat datele.<br>Doriți să salvați datele ?"),
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
            loadFile(dialog.selectedFiles().constFirst());
    dialog.close();
}

void UserPreferences::clearImageLogo()
{
    QMessageBox messange_box(QMessageBox::Question,
                             tr("Eliminarea logotipului"),
                             tr("Doriți să eliminați logotipul din baza de date ?"),
                             QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
    messange_box.setButtonText(QMessageBox::No, tr("Nu"));
#endif
    if (messange_box.exec() == QMessageBox::No)
        return;

    QSqlQuery qry;
    qry.prepare(QString("UPDATE constants SET logo = NULL WHERE id_users = '%1';").arg(m_Id));
    if (qry.exec()){
        ui->image_logo->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea<br>(se recomanda 400x50 px)</a>");
        ui->image_logo->setTextFormat(Qt::RichText);
        ui->image_logo->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Logotipul este eliminat din baza de date."));
        popUp->show();
    } else {
        qCritical(logCritical()) << tr("Eroare la eliminarea logotipului din baza de date %1").arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
    }
    ui->btnClearLogo->setVisible(false);
}

// **********************************************************************************
// --- procesarea listView

void UserPreferences::onClickedListView(const QModelIndex index)
{
    if (! index.isValid())
        return;

    if (index.row() == page_general)
        ui->stackedWidget->setCurrentIndex(page_general);
    else if (index.row() == page_launch)
        ui->stackedWidget->setCurrentIndex(page_launch);
    else if (index.row() == page_document)
        ui->stackedWidget->setCurrentIndex(page_document);
    else if (index.row() == page_notedit)
        ui->stackedWidget->setCurrentIndex(page_notedit);
}

void UserPreferences::onChangedValueUpdateListDoc(const int value)
{
    Q_UNUSED(value)
    dataWasModified();
}

// **********************************************************************************
// --- procesarea slot-urilor btn pu deschiderea cataloagelor

void UserPreferences::onOpenCatUsers()
{
    if (m_Id == idx_unknow)
        return;

    CatUsers* cat_user = new CatUsers(this);
    cat_user->setAttribute(Qt::WA_DeleteOnClose);
    cat_user->setProperty("Id", m_Id);
    cat_user->show();
}

void UserPreferences::onOpenCatDoctors()
{
    if (m_idDoctor == idx_unknow)
        return;

    CatGeneral* cat_doctor = new CatGeneral(this);
    cat_doctor->setAttribute(Qt::WA_DeleteOnClose);
    cat_doctor->setProperty("typeCatalog", CatGeneral::TypeCatalog::Doctors);
    cat_doctor->setProperty("itNew", false);
    cat_doctor->setProperty("Id", m_idDoctor);
    cat_doctor->show();
}

void UserPreferences::onOpenCatNurses()
{
    if (m_idNurse == idx_unknow)
        return;

    CatGeneral* cat_nurses = new CatGeneral(this);
    cat_nurses->setAttribute(Qt::WA_DeleteOnClose);
    cat_nurses->setProperty("typeCatalog", CatGeneral::TypeCatalog::Nurses);
    cat_nurses->setProperty("itNew", false);
    cat_nurses->setProperty("Id", m_idNurse);
    cat_nurses->show();
}

void UserPreferences::onOpenCatOrganizations()
{
    if (m_IdOrganization == idx_unknow)
        return;

    CatOrganizations* cat_organization = new CatOrganizations(this);
    cat_organization->setAttribute(Qt::WA_DeleteOnClose);
    cat_organization->setProperty("Id", m_IdOrganization);
    cat_organization->show();
}

// **********************************************************************************
// --- initierea si setarea modelelor si listWidget

void UserPreferences::setListWidget()
{
    const QStringList LIST_ITEMS = QStringList()
                                   << tr("Setările generale")
                                   << tr("Lansarea/închiderea")
                                   << tr("Setările documentelor")
                                   << tr("Setările neredactabile");

    ui->listWidget->setIconSize(QSize(18,18));
    int n = -1;
    foreach( const QString& item, LIST_ITEMS ) {
        n += 1;
        QListWidgetItem* listItem = new QListWidgetItem(item);
#if defined(Q_OS_LINUX)
        if (n == page_general)
            listItem->setIcon(QIcon::fromTheme("preferences-system"));//listItem->setIcon(QIcon(":/img/settings_x32.png"));
        else if (n == page_launch)
            listItem->setIcon(QIcon::fromTheme("applications-internet"));//listItem->setIcon(QIcon(":/img/update_app.png"));
        else if (n == page_document)
            listItem->setIcon(QIcon::fromTheme("edit-paste"));//listItem->setIcon(QIcon(":/img/orderEcho_x32.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon::fromTheme("emblem-important"));//listItem->setIcon(QIcon(":/img/not-editable.png"));
#elif defined(Q_OS_WIN)
        if (n == page_general)
            listItem->setIcon(QIcon(":/img/settings_x32.png"));
        else if (n == page_launch)
            listItem->setIcon(QIcon(":/img/update_app.png"));
        else if (n == page_document)
            listItem->setIcon(QIcon(":/img/orderEcho_x32.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon(":/img/not-editable.png"));
#endif

        ui->listWidget->addItem(listItem);

    }
    connect(ui->listWidget, &QListWidget::clicked, this, &UserPreferences::onClickedListView);
}

void UserPreferences::initSetModels()
{
    QString qry_users = "SELECT id,name FROM users WHERE deletionMark = 0;";
    model_users = new BaseSqlQueryModel(qry_users, ui->comboUsers);
    model_users->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboUsers->setModel(model_users);
    if (model_users->rowCount() > 20){
        ui->comboUsers->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboUsers->setStyleSheet("combobox-popup: 0;");
        ui->comboUsers->setMaxVisibleItems(15);
    }

    //-----------------------------------------------------------------------------
    // set model_organizations
    QString qry_organizations = "SELECT id, name FROM organizations WHERE deletionMark = 0;";
    model_organizations = new BaseSqlQueryModel(qry_organizations, ui->comboOrganizations);
    model_organizations->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboOrganizations->setModel(model_organizations);
    if (model_organizations->rowCount() > 20){
        ui->comboOrganizations->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboOrganizations->setStyleSheet("combobox-popup: 0;");
        ui->comboOrganizations->setMaxVisibleItems(15);
    }

    //-----------------------------------------------------------------------------
    // set model_doctors
    QString qry_doctors = "SELECT doctors.id, "
                          "fullNameDoctors.nameAbbreviated AS FullName "
                          "FROM doctors INNER JOIN fullNameDoctors ON fullNameDoctors.id_doctors = doctors.id WHERE doctors.deletionMark = 0;";
    model_doctors = new BaseSqlQueryModel(qry_doctors, ui->comboDoctors);
    model_doctors->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboDoctors->setModel(model_doctors);
    if (model_doctors->rowCount() > 20){
        ui->comboDoctors->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboDoctors->setStyleSheet("combobox-popup: 0;");
        ui->comboDoctors->setMaxVisibleItems(15);
    }

    //-----------------------------------------------------------------------------
    // set model_nurses
    QString qry_nurses = "SELECT nurses.id, "
                         "fullNameNurses.nameAbbreviated AS FullName "
                         "FROM nurses INNER JOIN fullNameNurses ON fullNameNurses.id_nurses = nurses.id WHERE nurses.deletionMark = 0;";
    model_nurses = new BaseSqlQueryModel(qry_nurses, ui->comboNurses);
    model_nurses->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboNurses->setModel(model_nurses);
    if (model_nurses->rowCount() > 20){
        ui->comboNurses->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboNurses->setStyleSheet("combobox-popup: 0;");
        ui->comboNurses->setMaxVisibleItems(15);
    }
}

// **********************************************************************************
// --- initierea connectarilor si deconectarilor

void UserPreferences::initConnections()
{
    connect(this, &UserPreferences::IdChanged, this, &UserPreferences::slot_IdChanged);
    connect(this, &UserPreferences::IdDoctorChanged, this, &UserPreferences::slot_IdDoctorChanged);
    connect(this, &UserPreferences::IdNurseChanged, this, &UserPreferences::slot_IdNurseChanged);
    connect(this, &UserPreferences::IdChangedOrganization, this, &UserPreferences::slot_IdChangedOrganization);

    connect(ui->btnOpenOrganizations, &QToolButton::clicked, this, &UserPreferences::onOpenCatOrganizations);
    connect(ui->btnOpenDoctors, &QToolButton::clicked, this, &UserPreferences::onOpenCatDoctors);
    connect(ui->btnOpenNurses, &QToolButton::clicked, this, &UserPreferences::onOpenCatNurses);
    connect(ui->btnOpenUsers, &QToolButton::clicked, this, &UserPreferences::onOpenCatUsers);

    connect(ui->btnClearLogo, &QAbstractButton::clicked, this, &UserPreferences::clearImageLogo);

    connectionsCombo();
    connectionCheckBox();

    connect(ui->btnOk, &QAbstractButton::clicked, this, &UserPreferences::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &UserPreferences::onWritingData);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &UserPreferences::close);
}

void UserPreferences::connectionsCombo()
{
    connect(ui->comboUsers, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboUsers));
    connect(ui->comboDoctors, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboDoctors));
    connect(ui->comboNurses, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboNurses));
    connect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboOrganizations));

    connect(ui->brandUSG, &QLineEdit::textChanged, this, &UserPreferences::dataWasModified);
}

void UserPreferences::disconnectCombo()
{
    disconnect(ui->comboUsers, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboUsers));
    disconnect(ui->comboDoctors, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboDoctors));
    disconnect(ui->comboNurses, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboNurses));
    disconnect(ui->comboOrganizations, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserPreferences::activatedComboOrganizations));

    disconnect(ui->brandUSG, &QLineEdit::textChanged, this, &UserPreferences::dataWasModified);
}

void UserPreferences::connectionCheckBox()
{
    connect(ui->check_showUserManual, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    connect(ui->check_databasesArchiving, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    connect(ui->check_showDesignerMenuPrint, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    connect(ui->check_showQuestionClosingApp, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    connect(ui->check_newVersion, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    connect(ui->check_splitFullNamePatient, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);

    connect(ui->updateListDoc, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&UserPreferences::onChangedValueUpdateListDoc));
}

void UserPreferences::disconnectionCheckBox()
{
    disconnect(ui->check_showUserManual, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    disconnect(ui->check_databasesArchiving, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    disconnect(ui->check_showDesignerMenuPrint, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    disconnect(ui->check_showQuestionClosingApp, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    disconnect(ui->check_newVersion, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    disconnect(ui->check_splitFullNamePatient, &QCheckBox::clicked, this, &UserPreferences::dataWasModified);

    disconnect(ui->updateListDoc, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&UserPreferences::onChangedValueUpdateListDoc));
}

// **********************************************************************************
// --- inserarea si actualizarea datelor

bool UserPreferences::controlRequiredObjects()
{
    if (ui->comboUsers->currentIndex() == 0){
        ui->textBrowser->setHtml(tr("%1 Nu este indicat utilizatorul !!!").arg(db->getHTMLImageWarning()));
        ui->textBrowser->setFixedHeight(45);
        ui->dockWidget->setFixedHeight(90);
        ui->dockWidget->show();
        return false;
    }

    if (ui->comboOrganizations->currentIndex() == 0){
        ui->textBrowser->setHtml(tr("%1 Nu este indicată organizația !!!").arg(db->getHTMLImageWarning()));
        ui->textBrowser->setFixedHeight(45);
        ui->dockWidget->setFixedHeight(90);
        ui->dockWidget->show();
        return false;
    }

    if (ui->comboDoctors->currentIndex() == 0){
        ui->textBrowser->setHtml(tr("%1 Nu este indicat doctorul !!!").arg(db->getHTMLImageWarning()));
        ui->textBrowser->setFixedHeight(45);
        ui->dockWidget->setFixedHeight(90);
        ui->dockWidget->show();
        return false;
    }

    return true;

}

bool UserPreferences::existRecordInTable()
{
    QSqlQuery qry;
    qry.prepare("SELECT COUNT(id) FROM userPreferences WHERE id_users = :id_users;");
    qry.bindValue(":id_users", m_Id);
    if (qry.exec() && qry.next())
        return (qry.value(0).toInt() > 0);

    return false;
}

bool UserPreferences::insertDataIntoTable()
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    //----------------------------------------------
    // inserarea datelor in tabela 'constants'

    qry.prepare("INSERT INTO constants ("
                "id_users,"
                "id_organizations,"
                "id_doctors,"
                "id_nurses,"
                "brandUSG) VALUES (?,?,?,?,?);");
    qry.addBindValue(m_Id);
    qry.addBindValue((m_IdOrganization == idx_unknow) ? QVariant() : m_IdOrganization);
    qry.addBindValue((m_idDoctor == idx_unknow) ? QVariant() : m_idDoctor);
    qry.addBindValue((m_idNurse == idx_unknow) ? QVariant() : m_idNurse);
    qry.addBindValue(ui->brandUSG->text());
    if (qry.exec()){
        qInfo(logInfo()) << tr("Au fost inserate date in tabela 'constants' pentru utilizator '%1' cu id=%2.")
                                .arg(ui->comboUsers->currentText(), QString::number(m_Id));
    } else {
        qCritical(logCritical()) << tr("Nu au fost inserate date in tabela 'constants' pentru utilizator '%1' cu id=%2 %3")
                                        .arg(ui->comboUsers->currentText(), QString::number(m_Id),
                                             (qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
        return false;
    }

    //----------------------------------------------
    // inserarea datelor in tabela 'userPreferences'

    qry.prepare("INSERT INTO userPreferences ("
                "id,"
                "id_users,"
                "versionApp,"
                "showQuestionCloseApp,"
                "showUserManual,"
                "showHistoryVersion,"
                "order_splitFullName,"
                "updateListDoc,"
                "showDesignerMenuPrint,"
                "checkNewVersionApp,"
                "databasesArchiving,"
                "showAsistantHelper) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
    qry.addBindValue(m_Id);
    qry.addBindValue(m_Id);
    qry.addBindValue(VER);
    if(globals::thisMySQL){
        qry.addBindValue((ui->check_showQuestionClosingApp->isChecked() ? true : false));
        qry.addBindValue((ui->check_showUserManual->isChecked() ? true : false));
        qry.addBindValue(0);
        qry.addBindValue((ui->check_splitFullNamePatient->isChecked() ? true : false));
        qry.addBindValue(ui->updateListDoc->value());
        qry.addBindValue((ui->check_showDesignerMenuPrint->isChecked() ? true : false));
        qry.addBindValue((ui->check_newVersion->isChecked() ? true : false));
        qry.addBindValue((ui->check_databasesArchiving->isChecked() ? true : false));
    } else if (globals::thisSqlite){
        qry.addBindValue((ui->check_showQuestionClosingApp->isChecked() ? 1 : 0));
        qry.addBindValue((ui->check_showUserManual->isChecked() ? 1 : 0));
        qry.addBindValue(0);
        qry.addBindValue((ui->check_splitFullNamePatient->isChecked() ? 1 : 0));
        qry.addBindValue(ui->updateListDoc->value());
        qry.addBindValue((ui->check_showDesignerMenuPrint->isChecked() ? 1 : 0));
        qry.addBindValue((ui->check_newVersion->isChecked() ? 1 : 0));
        qry.addBindValue((ui->check_databasesArchiving->isChecked() ? 1 : 0));
    } else {
        qWarning(logWarning()) << tr("Nu a fost determinat tipul bazei de date - solicitarea de inserarea a datelor in tabela 'userPreferences' este nereusita !!!");
        db->getDatabase().rollback();
        return false;
    }

    if (qry.exec()){
        qInfo(logInfo()) << tr("Au fost inserate date in tabela 'userPreferences' pentru utilizator '%1' cu id=%2.")
                                .arg(ui->comboUsers->currentText(), QString::number(m_Id));
    } else {
        qWarning(logWarning()) << tr("Solicitarea de inserarea a datelor in tabela 'userPreferences' este nereusita !!!");
        db->getDatabase().rollback();
        return false;
    }

    db->getDatabase().commit();

    return true;
}

bool UserPreferences::updateDataIntoTable()
{
    db->getDatabase().transaction();

    QSqlQuery qry;

    //----------------------------------------------
    // actualizarea datelor in tabela 'constants'

    qry.prepare("UPDATE constants SET "
                "id_organizations = :id_organizations,"
                "id_doctors       = :id_doctors,"
                "id_nurses        = :id_nurses,"
                "brandUSG         = :brandUSG "
                "WHERE id_users = :id_users;");
    qry.bindValue(":id_users",         m_Id);
    qry.bindValue(":id_organizations", (m_IdOrganization == idx_unknow) ? QVariant() : m_IdOrganization);
    qry.bindValue(":id_doctors",       (m_idDoctor == idx_unknow) ? QVariant() : m_idDoctor);
    qry.bindValue(":id_nurses",        (m_idNurse == idx_unknow) ? QVariant() : m_idNurse);
    qry.bindValue(":brandUSG",         ui->brandUSG->text());
    if (qry.exec()){
        qInfo(logInfo()) << tr("Au fost actualizate date in tabela 'constants' pentru utilizator '%1' cu id=%2.")
                                .arg(ui->comboUsers->currentText(), QString::number(m_Id));
    } else {
        qCritical(logCritical()) << tr("Nu au fost actualizate date in tabela 'constants' pentru utilizator '%1' cu id=%2 %3")
                                        .arg(ui->comboUsers->currentText(), QString::number(m_Id),
                                             (qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
        return false;
    }

    //----------------------------------------------
    // actualizarea datelor in tabela 'userPreferences'

    qry.prepare("UPDATE userPreferences SET "
                "id_users              = :id_users,"
                "versionApp            = :versionApp,"
                "showQuestionCloseApp  = :showQuestionCloseApp,"
                "showUserManual        = :showUserManual,"
                "showHistoryVersion    = :showHistoryVersion,"
                "order_splitFullName   = :order_splitFullName,"
                "updateListDoc         = :updateListDoc,"
                "showDesignerMenuPrint = :showDesignerMenuPrint,"
                "checkNewVersionApp    = :checkNewVersionApp,"
                "databasesArchiving    = :databasesArchiving,"
                "showAsistantHelper    = :showAsistantHelper "
                "WHERE id = :id;");
    qry.bindValue(":id",                    m_Id);
    qry.bindValue(":id_users",              m_Id);
    qry.bindValue(":versionApp",            VER);
    if(globals::thisMySQL){
        qry.bindValue(":showQuestionCloseApp",  (ui->check_showQuestionClosingApp->isChecked() ? true : false));
        qry.bindValue(":showUserManual",        (ui->check_showUserManual->isChecked() ? true : false));
        qry.bindValue(":showHistoryVersion",    0);
        qry.bindValue(":order_splitFullName",   (ui->check_splitFullNamePatient->isChecked() ? true : false));
        qry.bindValue(":updateListDoc",         ui->updateListDoc->value());
        qry.bindValue(":showDesignerMenuPrint", (ui->check_showDesignerMenuPrint->isChecked() ? true : false));
        qry.bindValue(":checkNewVersionApp",    (ui->check_newVersion->isChecked() ? true : false));
        qry.bindValue(":databasesArchiving",    (ui->check_databasesArchiving->isChecked() ? true : false));
        qry.bindValue(":showAsistantHelper",    (ui->showAsistantHelper->isChecked() ? true : false));
    } else if(globals::thisSqlite){
        qry.bindValue(":showQuestionCloseApp",  (ui->check_showQuestionClosingApp->isChecked() ? 1 : 0));
        qry.bindValue(":showUserManual",        (ui->check_showUserManual->isChecked() ? 1 : 0));
        qry.bindValue(":showHistoryVersion",    0);
        qry.bindValue(":order_splitFullName",   (ui->check_splitFullNamePatient->isChecked() ? 1 : 0));
        qry.bindValue(":updateListDoc",         ui->updateListDoc->value());
        qry.bindValue(":showDesignerMenuPrint", (ui->check_showDesignerMenuPrint->isChecked() ? 1 : 0));
        qry.bindValue(":checkNewVersionApp",    (ui->check_newVersion->isChecked() ? 1 : 0));
        qry.bindValue(":databasesArchiving",    (ui->check_databasesArchiving->isChecked() ? 1 : 0));
        qry.bindValue(":showAsistantHelper",    (ui->showAsistantHelper->isChecked() ? 1 : 0));
    } else {
        qWarning(logWarning()) << tr("Nu a fost determinat tipul bazei de date - actualizarea datelor din tabela 'userPreferences' este nereusita !!!");
        db->getDatabase().rollback();
        return false;
    }

    if (qry.exec()){
        qInfo(logInfo()) << tr("Au fost actualizate date in tabela 'userPreferences' pentru utilizator '%1' cu id=%2.")
                                .arg(ui->comboUsers->currentText(), QString::number(m_Id));
    } else {
        qWarning(logWarning()) << tr("Solicitarea de actualizare a datelor in tabela 'userPreferences' este nereusita !!!");
        db->getDatabase().rollback();
        return false;
    }

    db->getDatabase().commit();

    return true;
}

// **********************************************************************************
// --- evenimentele formei

void UserPreferences::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Doriți să salvați aceste modificări ?"),
                                 QMessageBox::Yes | QMessageBox::No, this);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        messange_box.setButtonText(QMessageBox::Yes, tr("Da"));
        messange_box.setButtonText(QMessageBox::No, tr("Nu"));
#endif
        if (messange_box.exec() == QMessageBox::Yes){
            if (onWritingData())
                event->accept();
            else
                event->ignore();
        }
    } else {
        event->accept();
    }
}

void UserPreferences::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    }
}

void UserPreferences::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
        this->focusNextChild();
    }
}
