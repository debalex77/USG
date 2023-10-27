#include "usersettings.h"
#include "ui_usersettings.h"

UserSettings::UserSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserSettings)
{
    ui->setupUi(this);

    setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    setWindowIcon(QIcon(":/img/settings_x32.png"));

    db                = new DataBase(this);
    popUp             = new PopUp(this);
    completer         = new QCompleter(this);
    modelOrganization = new QStandardItemModel(completer);

    ui->editBrandUSG->setMaxLength(200);

    QString strQueryUser = "SELECT id,name FROM users WHERE deletionMark = 0;";
    modelUser = new BaseSqlQueryModel(strQueryUser, ui->comboUser);
    modelUser->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboUser->setModel(modelUser);

    //---------------------------------------------------------------------------------
    QString strQueryDoctor;
    strQueryDoctor = "SELECT doctors.id, "
                     "fullNameDoctors.nameAbbreviated AS FullName "
                     "FROM doctors INNER JOIN fullNameDoctors ON fullNameDoctors.id_doctors = doctors.id WHERE doctors.deletionMark = 0;";
    modelDoctor = new BaseSqlQueryModel(strQueryDoctor, ui->comboDoctor);
    modelDoctor->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboDoctor->setModel(modelDoctor);
    if (modelDoctor->rowCount() > 20){
        ui->comboDoctor->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboDoctor->setStyleSheet("combobox-popup: 0;");
        ui->comboDoctor->setMaxVisibleItems(15);
    }

    //---------------------------------------------------------------------------------
    QString strQueryNurse;
    strQueryNurse = "SELECT nurses.id, "
                    "fullNameNurses.nameAbbreviated AS FullName "
                    "FROM nurses INNER JOIN fullNameNurses ON fullNameNurses.id_nurses = nurses.id WHERE nurses.deletionMark = 0;";
    modelNurse = new BaseSqlQueryModel(strQueryNurse, ui->comboNurse);
    modelNurse->setProperty("modelParent", BaseSqlQueryModel::ModelParent::UserSettings);
    ui->comboNurse->setModel(modelNurse);
    if (modelNurse->rowCount() > 20){
        ui->comboNurse->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->comboNurse->setStyleSheet("combobox-popup: 0;");
        ui->comboNurse->setMaxVisibleItems(15);
    }

    //---------------------------------------------------------------------------------
    connect(ui->comboUser, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserSettings::activatedComboUser));
    connect(ui->comboDoctor, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserSettings::activatedComboDoctor));
    connect(ui->comboNurse, QOverload<int>::of(&QComboBox::activated), this, QOverload<int>::of(&UserSettings::activatedComboNurse));

    initSetCompleter();

    connect(ui->editBrandUSG, &QLineEdit::textChanged, this, &UserSettings::dataWasModified);
    connect(this, &UserSettings::IdChanged, this, &UserSettings::slot_IdChanged);

    connect(this, &UserSettings::IdChanged, this, &UserSettings::slot_IdChanged);
    connect(this, &UserSettings::IdDoctorChanged, this, &UserSettings::slot_IdDoctorChanged);
    connect(this, &UserSettings::IdNurseChanged, this, &UserSettings::slot_IdNurseChanged);
    connect(this, &UserSettings::IdChangedOrganization, this, &UserSettings::slot_IdChangedOrganization);

    ui->btnClearLogo->setStyleSheet("border: 1px solid #8f8f91; border-radius: 4px;");

    connect(ui->btnClearLogo, &QAbstractButton::clicked, this, &UserSettings::clearImageLogo);
    connect(ui->btnOK, &QAbstractButton::clicked, this, &UserSettings::onWritingDataClose);
    connect(ui->btnWrite, &QAbstractButton::clicked, this, &UserSettings::onWritingData);
    connect(ui->btnCancel, &QAbstractButton::clicked, this, &UserSettings::onClose);

    ui->imageLabel->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea<br>(se recomanda 400x50 px)</a>");
    ui->imageLabel->setTextFormat(Qt::RichText);
    ui->imageLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(ui->imageLabel, QOverload<const QString &>::of(&QLabel::linkActivated), this,
            QOverload<const QString &>::of(&UserSettings::onLinkActivatedForOpenImage));
}

UserSettings::~UserSettings()
{
    delete db;
    delete popUp;
    delete modelOrganization; // in primul rand modelul preconizat p-u completer
    delete completer;         // apoi completer
    delete modelUser;
    delete modelDoctor;
    delete modelNurse;
    delete ui;
}

void UserSettings::initSetCompleter()
{
    QString strQuery;
    if (globals::thisMySQL)
        strQuery = "SELECT organizations.id, CONCAT(organizations.name,', c/f: ', organizations.IDNP) AS FullName FROM organizations WHERE organizations.deletionMark = 0;";
    else
        strQuery = "SELECT organizations.id, organizations.name ||', c/f: '|| organizations.IDNP AS FullName FROM organizations WHERE organizations.deletionMark = 0;";
    QMap<int, QString> data = db->getMapDataQuery(strQuery);

    QMapIterator<int, QString> it(data);
    while (it.hasNext()) {
        it.next();
        int     _id   = it.key();
        QString _name = it.value();

        QStandardItem *item = new QStandardItem;
        item->setData(_id,   Qt::UserRole);
        item->setData(_name, Qt::DisplayRole);

        modelOrganization->appendRow(item);  // adaugam datele in model
    }
    completer->setModel(modelOrganization);             // setam model in completer
    completer->setCaseSensitivity(Qt::CaseInsensitive); // cautare insenseibila la registru

    ui->editOrganization->setCompleter(completer); // setam completer la Organizatia

    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, QOverload<const QModelIndex&>::of(&UserSettings::activatedItemCompleter));
}

void UserSettings::initSetTreeWidget()
{
    ui->treeWidget->setColumnCount(3);

    QStringList YesNo;
    YesNo << tr("Da") << tr("Nu");

    QStringList headers;
    headers << tr("Denumirea setării") << tr("Valoarea") << tr("id");
    ui->treeWidget->setHeaderLabels(headers);
    ui->treeWidget->setColumnWidth(0, 420);

    QTreeWidgetItem* root_item = new QTreeWidgetItem(ui->treeWidget);
    root_item->setText(0, "Setări");
    root_item->setIcon(0, QIcon(":/img/folder_yellow.png"));
    ui->treeWidget->addTopLevelItem(root_item);

    QSqlQuery qry;
    qry.prepare("SELECT owner FROM settingsUsers WHERE id_users = :id_users;");
    qry.bindValue(":id_users", m_Id);
    qry.exec();
    while (qry.next()) {
        const QString owner = qry.value(0).toString();
        if (! list_owners.contains(owner)){
            list_owners << owner;
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, owner);
            child->setIcon(0, QIcon(":/img/folder_yellow.png"));
            root_item->addChild(child);

            QSqlQuery qry_item;
            qry_item.prepare("SELECT nameOption, "
                             "versionApp,"
                             "showQuestionCloseApp,"
                             "showUserManual,"
                             "showHistoryVersion,"
                             "order_splitFullName,"
                             "updateListDoc,"
                             "showDesignerMenuPrint,"
                             "id FROM settingsUsers WHERE owner = :owner AND id_users = :id_users;");
            qry_item.bindValue(":owner", owner);
            qry_item.bindValue(":id_users", m_Id);
            qry_item.exec();
            while (qry_item.next()) {
                const QString name_option = qry_item.value(0).toString();

                QVariant value;
                if (! qry_item.value(1).isNull())
                    value = qry_item.value(1);
                if (! qry_item.value(2).isNull())
                    value = qry_item.value(2);
                if (! qry_item.value(3).isNull())
                    value = qry_item.value(3);
                if (! qry_item.value(4).isNull())
                    value = qry_item.value(4);
                if (! qry_item.value(5).isNull())
                    value = qry_item.value(5);
                if (! qry_item.value(6).isNull())
                    value = qry_item.value(6);
                if (! qry_item.value(7).isNull())
                    value = qry_item.value(7);

                QTreeWidgetItem* child_item = new QTreeWidgetItem();
                child_item->setText(0, name_option);
                child_item->setIcon(0, QIcon(":/img/element.png"));
                if (value.userType() == QMetaType::QString){
                    child_item->setText(1, value.toString());
                } else {
                    child_item->setCheckState(1, Qt::Unchecked);
                    (value.toInt() == 0) ? child_item->setCheckState(1, Qt::Unchecked) : child_item->setCheckState(1, Qt::Checked);
                }
                child_item->setText(2, qry_item.value(8).toString());
                child->addChild(child_item);
            }
        }
    }

    ui->treeWidget->hideColumn(2);
    ui->treeWidget->expandAll();

    connect(ui->treeWidget, QOverload<QTreeWidgetItem *, int>::of(&QTreeWidget::itemDoubleClicked),
            this, QOverload<QTreeWidgetItem *, int>::of(&UserSettings::onTreeWidgetItemDoubleClicked));
}

bool UserSettings::controlRequiredObjects()
{
    if (ui->comboUser->currentIndex() <= idx_write){
        QMessageBox::warning(this, tr("Controlul completării obiectelor"),
                             tr("Nu este selectat <b>'Utilizatorul'</b> !!!"),
                             QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }
    return true;
}

QMap<QString, QString> UserSettings::getDataObject()
{

    QMap<QString, QString> _items;
    _items.insert("id_users",         ui->comboUser->itemData(ui->comboUser->currentIndex(), Qt::UserRole).toString());
    _items.insert("id_organizations", QString::number(m_IdOrganization));
    _items.insert("id_doctors",       ui->comboDoctor->itemData(ui->comboDoctor->currentIndex(), Qt::UserRole).toString());
    _items.insert("id_nurses",        ui->comboNurse->itemData(ui->comboNurse->currentIndex(), Qt::UserRole).toString());
    _items.insert("brandUSG",         ui->editBrandUSG->text());

    return _items;
}

bool UserSettings::insertIntoTableConstants()
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO constants ("
                "id_users,"
                "id_organizations,"
                "id_doctors,"
                "id_nurses,"
                "brandUSG) VALUES (?,?,?,?,?);");
    qry.addBindValue(m_Id);
    qry.addBindValue((m_IdOrganization == -1) ? QVariant() : m_IdOrganization);
    qry.addBindValue((m_idDoctor == -1) ? QVariant() : m_idDoctor);
    qry.addBindValue((m_idNurse == -1) ? QVariant() : m_idNurse);
    qry.addBindValue(ui->editBrandUSG->text());
    if (! qry.exec()){
        qCritical(logCritical()) << tr("%1 - onWritingData()").arg(metaObject()->className())
                                 << tr("Nu s-a reusit de salvat setarile utilizatorului cu ID:%1").arg(m_Id);
        return false;
    }
    return true;
}

bool UserSettings::updateDataTableConstants()
{
    QSqlQuery qry;
    qry.prepare("UPDATE constants SET "
                "id_organizations = :id_organizations,"
                "id_doctors       = :id_doctors,"
                "id_nurses        = :id_nurses,"
                "brandUSG         = :brandUSG "
                "WHERE id_users = :id_users;");
    qry.bindValue(":id_users",         m_Id);
    qry.bindValue(":id_organizations", (m_IdOrganization == -1) ? QVariant() : m_IdOrganization);
    qry.bindValue(":id_doctors",       (m_idDoctor == -1) ? QVariant() : m_idDoctor);
    qry.bindValue(":id_nurses",        (m_idNurse == -1) ? QVariant() : m_idNurse);
    qry.bindValue(":brandUSG",         ui->editBrandUSG->text());
    if (! qry.exec()){
        qCritical(logCritical()) << tr("%1 - updateDataTableConstants()").arg(metaObject()->className())
                                 << tr("Nu s-a reusit actualizarea datelor setarilor utilizatorului cu ID:%1 !!!").arg(QString::number(m_Id));
        return false;
    }
    return true;
}

QString UserSettings::existSettingsForUser()
{
    return QString("SELECT * FROM constants WHERE id_users = '%1';").arg(QString::number(m_Id));
}

void UserSettings::setTableSettingsUsers()
{
    QSqlQuery qry;

    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {

        const int _id = (*it)->text(2).toInt();
        if (_id != 0){     // 0 = directoria 4 = versionApp

            if ((*it)->text(0) == "interogarea la închiderea aplicației"){ // showQuestionCloseApp

                qry.prepare("UPDATE settingsUsers SET showQuestionCloseApp = :showQuestionCloseApp WHERE id =:id AND id_users = :id_users;");
                if (globals::thisMySQL)
                    qry.bindValue(":showQuestionCloseApp", ((*it)->checkState(1) == Qt::Checked) ? true : false);
                else
                    qry.bindValue(":showQuestionCloseApp", ((*it)->checkState(1) == Qt::Checked) ? 1 : 0);
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::showQuestionCloseApp = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'showQuestionCloseApp' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            } else if ((*it)->text(0) == "prezentarea User manual (la lansare aplicației)"){ // showUserManual

                qry.prepare("UPDATE settingsUsers SET showUserManual = :showUserManual WHERE id =:id AND id_users = :id_users;");
                if (globals::thisMySQL)
                    qry.bindValue(":showUserManual", ((*it)->checkState(1) == Qt::Checked) ? true : false);
                else
                    qry.bindValue(":showUserManual", ((*it)->checkState(1) == Qt::Checked) ? 1 : 0);
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::showUserManual = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'showUserManual' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            } else if ((*it)->text(0) == "prezentarea Istoriei versiunilor (la lansare aplicației)"){ // showHistoryVersion

                qry.prepare("UPDATE settingsUsers SET showHistoryVersion = :showHistoryVersion WHERE id =:id AND id_users = :id_users;");
                if (globals::thisMySQL)
                    qry.bindValue(":showHistoryVersion", ((*it)->checkState(1) == Qt::Checked) ? true : false);
                else
                    qry.bindValue(":showHistoryVersion", ((*it)->checkState(1) == Qt::Checked) ? 1 : 0);
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::showHistoryVersion = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'showUserManual' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            } else if ((*it)->text(0) == "prezentarea descifrării NPP pacientului"){ // order_splitFullName

                qry.prepare("UPDATE settingsUsers SET order_splitFullName = :order_splitFullName WHERE id =:id AND id_users = :id_users;");
                if (globals::thisMySQL)
                    qry.bindValue(":order_splitFullName", ((*it)->checkState(1) == Qt::Checked) ? true : false);
                else
                    qry.bindValue(":order_splitFullName", ((*it)->checkState(1) == Qt::Checked) ? 1 : 0);
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::order_splitFullName = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'order_splitFullName' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            } else if ((*it)->text(0) == "actualizarea listei de documente (în secunde)"){ // updateListDoc

                qry.prepare("UPDATE settingsUsers SET updateListDoc = :updateListDoc WHERE id =:id AND id_users = :id_users;");
                qry.bindValue(":updateListDoc", (*it)->text(1).toInt());
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::updateIntervalListDoc = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'updateListDoc' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            } else if ((*it)->text(0) == "prezentarea designerului(LimeReport) în meniu de printare"){ // showDesignerMenuPrint

                qry.prepare("UPDATE settingsUsers SET showDesignerMenuPrint = :showDesignerMenuPrint WHERE id =:id AND id_users = :id_users;");
                if (globals::thisMySQL)
                    qry.bindValue(":showDesignerMenuPrint", ((*it)->checkState(1) == Qt::Checked) ? true : false);
                else
                    qry.bindValue(":showDesignerMenuPrint", ((*it)->checkState(1) == Qt::Checked) ? 1 : 0);
                qry.bindValue(":id", _id);
                qry.bindValue(":id_users", m_Id);
                if (qry.exec())
                    globals::showDesignerMenuPrint = (*it)->checkState(1);
                else
                    qCritical(logCritical()) << tr("Nu este actualizata variabila 'showDesignerMenuPrint' din tabela 'settingsUsers' - %1").arg(qry.lastError().text());

            }
        }
        ++it;
    }
}

void UserSettings::dataWasModified()
{
    setWindowModified(true);
}

void UserSettings::slot_IdChanged()
{
    if (m_Id == idx_unknow)
        return;

    auto indexUser = modelUser->match(modelUser->index(0, 0), Qt::UserRole, m_Id, 1, Qt::MatchExactly);
    if(!indexUser.isEmpty())
        ui->comboUser->setCurrentIndex(indexUser.first().row());

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
        if (!brandUSG.isEmpty())
            ui->editBrandUSG->setText(brandUSG);
    }

    QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", m_Id);
    QPixmap outPixmap = QPixmap();
    if (outPixmap.loadFromData(outByteArray)){
        ui->btnClearLogo->setVisible(true);
        ui->imageLabel->setPixmap(outPixmap.scaled(400, 50));
    } else {
        ui->btnClearLogo->setVisible(false);
    }
    initSetTreeWidget();
    setWindowModified(false);
}

void UserSettings::slot_IdDoctorChanged()
{
    auto indexDoctor = modelDoctor->match(modelDoctor->index(0, 0), Qt::UserRole, m_idDoctor, 1, Qt::MatchExactly);
    if (!indexDoctor.isEmpty())
        ui->comboDoctor->setCurrentIndex(indexDoctor.first().row());
}

void UserSettings::slot_IdNurseChanged()
{
    auto indexNurse  = modelNurse->match(modelNurse->index(0, 0), Qt::UserRole, m_idNurse, 1, Qt::MatchExactly);
    if (!indexNurse.isEmpty())
        ui->comboNurse->setCurrentIndex(indexNurse.first().row());
}

void UserSettings::slot_IdChangedOrganization()
{
    auto indexOrganization = modelOrganization->match(modelOrganization->index(0, 0), Qt::UserRole, m_IdOrganization, 1, Qt::MatchExactly);
    if (!indexOrganization.isEmpty()){
        completer->setCurrentRow(indexOrganization.first().row());
        ui->editOrganization->setText(completer->currentCompletion());
    }
}

void UserSettings::onTreeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QVariant value = item->data(0, Qt::DisplayRole);
    if (column == 1) {     
        if (value == "actualizarea listei de documente (în secunde)")
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->treeWidget->editItem(item, column);        
    }
}

void UserSettings::activatedItemCompleter(const QModelIndex &index)
{
    setIdOrganization(index.data(Qt::UserRole).toInt());
    emit IdChangedOrganization();
    if (index.data(Qt::UserRole).toInt() != 0) // index = 0 (please select)
        dataWasModified();
}

void UserSettings::activatedComboUser(const int index)
{
    int id_user = ui->comboUser->itemData(index, Qt::UserRole).toInt();
    setId(id_user);
    dataWasModified();
}

void UserSettings::activatedComboDoctor(const int index)
{
    int id_doctor = ui->comboDoctor->itemData(index, Qt::UserRole).toInt();
    setIdDoctor(id_doctor);
    dataWasModified();
}

void UserSettings::activatedComboNurse(const int index)
{
    int id_nurse = ui->comboNurse->itemData(index, Qt::UserRole).toInt();
    setIdNurse(id_nurse);
    dataWasModified();
}

bool UserSettings::loadFile(const QString &fileName)
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
    ui->imageLabel->setPixmap(QPixmap::fromImage(newImage).scaled(400,50));
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

void UserSettings::onLinkActivatedForOpenImage(const QString &link)
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

void UserSettings::clearImageLogo()
{
    QSqlQuery qry;
    qry.prepare(QString("UPDATE constants SET logo = NULL WHERE id_users = '%1';").arg(m_Id));
    if (qry.exec()){
        ui->imageLabel->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea<br>(se recomanda 400x50 px)</a>");
        ui->imageLabel->setTextFormat(Qt::RichText);
        ui->imageLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        popUp->setPopupText(tr("Logotipul este eliminat din baza de date."));
        popUp->show();
    } else {
        qCritical(logCritical()) << tr("Eroare la eliminarea logotipului din baza de date:\n") << qry.lastError();
    }
    ui->btnClearLogo->setVisible(false);
}

bool UserSettings::onWritingData()
{
    if (!controlRequiredObjects())
        return false;

    QMap<QString, QString> items;
    if (! db->getDataFromQuery(existSettingsForUser(), items))
        qDebug() << tr("Determinarea setărilor utilizatorului '%1' nu s-a reusit.").arg(ui->comboUser->currentText());

    if (items.count() > 0){
        if (! updateDataTableConstants()){
            QMessageBox::warning(this, tr("Crearea obiectului."),
                                 tr("Actualizarea setarilor utilizatorului \"<b>%1</b>\" nu s-a efectuat.<br>"
                                    "Adresați-vă administratorului aplicației.")
                                 .arg(ui->comboUser->currentText()), QMessageBox::Ok);
            return false;
        }
    } else {
        if (! insertIntoTableConstants()){
            QMessageBox::warning(this, tr("Crearea obiectului."),
                                 tr("Setarile utilizatorului \"<b>%1</b>\" nu au fost salvate in baza de date.<br>"
                                    "Adresați-vă administratorului aplicației.")
                                 .arg(ui->comboUser->currentText()), QMessageBox::Ok);
            return false;
        }
    }

    setTableSettingsUsers();

    return true;
}

void UserSettings::onWritingDataClose()
{
    if (onWritingData())
        QDialog::accept();
}

void UserSettings::onClose()
{
    this->close();
}

void UserSettings::closeEvent(QCloseEvent *event)
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

void UserSettings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        // traducem  titlu
        setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    }
}

void UserSettings::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return || event->key() == Qt::Key_Enter){
      this->focusNextChild();
    }
}
