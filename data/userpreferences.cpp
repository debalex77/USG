#include "userpreferences.h"
#include "ui_userpreferences.h"

UserPreferences::UserPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserPreferences)
{
    ui->setupUi(this);

    setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    setWindowIcon(QIcon(":/img/settings_x32.png"));

    db     = new DataBase(this);
    popUp  = new PopUp(this);
    model_listView = new QStringListModel(this);


    const QStringList LIST_ITEMS = QStringList()
                                   << tr("Setările generale")
                                   << tr("Lansarea/închiderea")
                                   << tr("Setările documentelor")
                                   << tr("Valori neredactabile");

    ui->listWidget->setIconSize(QSize(18,18));
    int n = -1;
    foreach( const QString& item, LIST_ITEMS ) {
        n += 1;
        QListWidgetItem* listItem = new QListWidgetItem(item);
        if (n == page_general)
            listItem->setIcon(QIcon(":/img/settings_x32.png"));
        else if (n == page_launch)
            listItem->setIcon(QIcon(":/img/update_app.png"));
        else if (n == page_document)
            listItem->setIcon(QIcon(":/img/orderEcho_x32.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon(":/img/not-editable.png"));

        ui->listWidget->addItem(listItem);

    }
    connect(ui->listWidget, &QListWidget::clicked, this, &UserPreferences::onClickedListView);

//    QStringList list;
//    list << tr("Setările generale") << tr("Lansarea/închiderea") << tr("Setările documentelor");
//    model_listView->setStringList(list);
//    ui->listView->setModel(model_listView);
//    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    connect(ui->listView, &QListView::clicked, this, &UserPreferences::onClickedListView);

    ui->brandUSG->setMaxLength(200);

    ui->dockWidget->close();

    //-----------------------------------------------------------------------------
    // set model_users
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

    initConnections();
}

UserPreferences::~UserPreferences()
{
    delete popUp;
    delete model_users;
    delete model_doctors;
    delete model_nurses;
    delete model_organizations;
    delete model_listView;
    delete db;
    delete ui;
}

void UserPreferences::dataWasModified()
{
    setWindowModified(true);
}

void UserPreferences::slot_IdChanged()
{
    if (m_Id == idx_unknow)
        return;

    disconnectCombo();

    auto index_user = model_users->match(model_users->index(0, 0), Qt::UserRole, m_Id, 1, Qt::MatchExactly);
    if(!index_user.isEmpty())
        ui->comboUsers->setCurrentIndex(index_user.first().row());

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

    QByteArray outByteArray = db->getOutByteArrayImage("constants", "logo", "id_users", m_Id);
    QPixmap outPixmap = QPixmap();
    if (outPixmap.loadFromData(outByteArray)){
//        ui->btnClearLogo->setVisible(true);
        ui->image_logo->setPixmap(outPixmap.scaled(400, 50));
    } else {
//        ui->btnClearLogo->setVisible(false);
    }

    connectionsCombo();
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

void UserPreferences::initConnections()
{
    connect(this, &UserPreferences::IdChanged, this, &UserPreferences::slot_IdChanged);
    connect(this, &UserPreferences::IdDoctorChanged, this, &UserPreferences::slot_IdDoctorChanged);
    connect(this, &UserPreferences::IdNurseChanged, this, &UserPreferences::slot_IdNurseChanged);
    connect(this, &UserPreferences::IdChangedOrganization, this, &UserPreferences::slot_IdChangedOrganization);

    connectionsCombo();
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

void UserPreferences::closeEvent(QCloseEvent *event)
{

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
