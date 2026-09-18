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

#include "userpreferences.h"
#include "ui_userpreferences.h"

#include <QImageReader>
#include <QImageWriter>
#include <QSignalBlocker>

#include <customs/custommessage.h>

UserPreferences::UserPreferences(DataBase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserPreferences)
    , m_db(db)
{
    ui->setupUi(this);

    setWindowTitle(tr("Setări utilizatorului %1").arg("[*]"));
    setWindowIcon(QIcon(":/img/catalogs/settings.png"));

    setListWidget(); // setam list widget

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(9);

    ui->brandUSG->setMaxLength(200);
    ui->versionApp->setText(USG_VERSION_FULL);
    ui->versionApp->setEnabled(false);

    ui->dockWidget->close(); // ascundem panel pu informatii

    initSetModels();            // setam modelurile
    m_showVideoMessage = globals().show_content_info_video;
    m_showReportsMessage = globals().show_info_reports;
    m_initialShowVideoMessage = m_showVideoMessage;
    m_initialShowReportsMessage = m_showReportsMessage;
    setValueIntoTableMessage(); // setam prezentarea mesajelor

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

    if (globals().isSystemThemeDark){
        ui->frame->setStyleSheet(R"("
            background-color: #2b2b2b;
            border: 1px solid #555;
            border-radius: 5px;
            )");
        ui->frame_2->setObjectName("customFrame");
        ui->frame_2->setStyleSheet(R"(
            QFrame#customFrame
            {
                background-color: #2b2b2b;
                border: 1px solid #555;    /* Linie subțire gri */
                border-radius: 5px;
            })");
    }

}

UserPreferences::~UserPreferences()
{
    delete ui;
    delete style_fusion;
}

// **********************************************************************************
// --- procesarea modificarii formei si a validarii datelor

void UserPreferences::dataWasModified()
{
    if (!m_loading)
        setWindowModified(true);
}

bool UserPreferences::onWritingData()
{
    // verificam daca este determinat corect ID
    if (m_Id == idx_unknow) {

        qWarning(logWarning()) << this->metaObject()->className()
                               << "[onWritingData]: "
                               << "Nu este transmis corect sau nu este determinat ID utilizatorului !!!";

        CustomMessage *message = new CustomMessage(this);
        message->setTextTitle(tr("Preferintele utilizatorului nu pot fi salvate !!!"));
        message->setDetailedText(tr("Nu este determinat ID utilizatorului. Adresati-va administratorului aplicatiei."));
        message->exec();
        message->deleteLater();

        return false;
    }

    // verificam completarea campurilor obligatorii
    if (! controlRequiredObjects())
        return false;

    QStringList err; // anuntam variabila pu erori

    /* cream o functie lambda locala in interiorul pu oprimizarea codului
     * si eliminarea dublajului */
    auto showError = [&](const QString &title) {
        CustomMessage *message = new CustomMessage(this);
        message->setTextTitle(title.arg(ui->comboUsers->currentText()));
        message->setDetailedText(err.join("\n"));
        message->exec();
        message->deleteLater();
    };

    QSqlDatabase database = m_db.getDatabase();
    if (!database.isValid() || !database.isOpen() || !database.transaction()) {
        err << tr("Nu a putut fi inițiată tranzacția pentru salvarea preferințelor: %1")
                   .arg(database.lastError().text());
        showError(tr("Preferințele utilizatorului '%1' nu pot fi salvate !!!"));
        return false;
    }

    auto rollbackAndShow = [&](const QString &title) {
        database.rollback();
        showError(title);
        return false;
    };

    bool constantsExists = false;
    if (!recordExists(QStringLiteral("constants"), constantsExists, err))
        return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi verificate !!!"));

    // inserarea/actualizarea datelor in tabela 'constants'
    if (!constantsExists) {
        if (! insertDataIntoTableConstants(err)){
            return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi salvate !!!"));
        }
    } else {
        if (! updateDataIntoTableConstants(err)) {
            return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi actualizate !!!"));
        }
    }

    bool preferencesExists = false;
    if (!recordExists(QStringLiteral("userPreferences"), preferencesExists, err))
        return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi verificate !!!"));

    // inserarea/actualizarea datelor in tabela 'userPreferences'
    if (!preferencesExists) {
        if (! insertDataIntoTableUserPreferences(err)) {
            return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi salvate !!!"));
        }
    } else {
        if (! updateDataIntoTableUserPreferences(err)) {
            return rollbackAndShow(tr("Preferințele utilizatorului '%1' nu pot fi salvate !!!"));
        }
    }

    if (!saveLocalMessagePreferences(err))
        return rollbackAndShow(tr("Preferințele locale nu pot fi salvate pentru utilizatorul '%1' !!!"));

    if (!database.commit()) {
        err << tr("Confirmarea tranzacției a eșuat: %1").arg(database.lastError().text());
        database.rollback();
        // Restabilim best-effort setările locale deja scrise.
        AppSettings::saveInfoMessageVisibility(AppSettings::InfoMessage::Video,
                                               m_initialShowVideoMessage);
        AppSettings::saveInfoMessageVisibility(AppSettings::InfoMessage::Reports,
                                               m_initialShowReportsMessage);
        showError(tr("Preferințele utilizatorului '%1' nu pot fi salvate !!!"));
        return false;
    }

    // dupa validare actualizam variabile globale
    if (m_Id == globals().idUserApp) {
        globals().showUserManual = ui->check_showUserManual->isChecked();
        globals().databasesArchiving = ui->check_databasesArchiving->isChecked();
        globals().showDesignerMenuPrint = ui->check_showDesignerMenuPrint->isChecked();
        emit showDesignerMenuPrintChanged(globals().showDesignerMenuPrint);
        globals().minimizeAppToTray = ui->minimizeAppToTray->isChecked();
        globals().showQuestionCloseApp = ui->check_showQuestionClosingApp->isChecked();
        globals().showDocumentsInSeparatWindow = ui->showDocumentsInSeparatWindow->isChecked();
        globals().checkNewVersionApp = ui->check_newVersion->isChecked();
        globals().order_splitFullName = ui->check_splitFullNamePatient->isChecked();
        globals().showAsistantHelper = ui->showAsistantHelper->isChecked();
        globals().updateIntervalListDoc = ui->updateListDoc->value();
        globals().organizationLogoData = m_logoData;
    }
    globals().show_content_info_video = m_showVideoMessage;
    globals().show_info_reports = m_showReportsMessage;
    m_initialShowVideoMessage = m_showVideoMessage;
    m_initialShowReportsMessage = m_showReportsMessage;
    setWindowModified(false);

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

    m_loading = true;
    resetFormForUser();

    //--------------------------------------------------------------
    // setam 'comboUsers' dupa ID

    auto index_user = model_users->match(model_users->index(0, 0), Qt::UserRole, m_Id, 1, Qt::MatchExactly);
    if(!index_user.isEmpty())
        ui->comboUsers->setCurrentIndex(index_user.first().row());

    //--------------------------------------------------------------
    // tabela 'constants' + 'userPreferences'

    QString loadError;
    QVariantMap map = m_db.selectJoinConstantsUserPreferencesByUserId(m_Id, &loadError);

    if (!loadError.isEmpty()) {
        qWarning(logWarning()) << metaObject()->className()
                               << "[slot_IdChanged]:" << loadError;
    }

    if (map.count() > 0) {

        // -- setam comboboxurile - Doctor, As.medicala, Organizatia
        int id_doctor       = map["id_doctors"].toInt();
        int id_nurse        = map["id_nurses"].toInt();
        int id_organization = map["id_organizations"].toInt();

        if (id_doctor > idx_write)
            setIdDoctor(id_doctor);
        if (id_nurse > idx_write)
            setIdNurse(id_nurse);
        if (id_organization > idx_write)
            setIdOrganization(id_organization);

        // -- brand
        QString brandUSG = map["brandUSG"].toString();
        ui->brandUSG->setText(brandUSG);

        // -- preferintele
        ui->minimizeAppToTray->setChecked(map["minimizeAppToTray"].toBool());
        ui->check_showQuestionClosingApp->setChecked(map["showQuestionCloseApp"].toBool());
        ui->check_showDesignerMenuPrint->setChecked(map["showDesignerMenuPrint"].toBool());
        ui->check_showUserManual->setChecked(map["showUserManual"].toBool());
        ui->showDocumentsInSeparatWindow->setChecked(map["showDocumentsInSeparatWindow"].toBool());
        ui->check_splitFullNamePatient->setChecked(map["order_splitFullName"].toBool());
        ui->check_newVersion->setChecked(map["checkNewVersionApp"].toBool());
        ui->check_databasesArchiving->setChecked(map["databasesArchiving"].toBool());
        ui->showAsistantHelper->setChecked(map["showAsistantHelper"].toBool());
        ui->updateListDoc->setValue(map["updateListDoc"].toInt());

        /**** logotip ***********************************************************
         * 1. Extragem din variabila globala (daca este salvat)
         * 2. Extragem din baza de date (daca nu este in variabila globala)
         *************************************************************************/

        QPixmap outPixmap;
        m_logoData = QByteArray::fromBase64(map["logo"].toByteArray());
        if (!m_logoData.isEmpty() && outPixmap.loadFromData(m_logoData)) {
            ui->btnClearLogo->setVisible(true);
            ui->image_logo->setPixmap(outPixmap.scaled(400, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    m_loading = false;
    setWindowModified(false);
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
    if (id_user == m_Id)
        return;

    if (isWindowModified()) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, tr("Modificarea datelor"),
            tr("Preferințele utilizatorului curent au fost modificate. Doriți să le salvați?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if (answer == QMessageBox::Cancel
            || (answer == QMessageBox::Save && !onWritingData())) {
            restoreSelectedUserInCombo();
            return;
        }
        if (answer == QMessageBox::Discard) {
            m_showVideoMessage = m_initialShowVideoMessage;
            m_showReportsMessage = m_initialShowReportsMessage;
            setValueIntoTableMessage();
        }
        setWindowModified(false);
    }
    setId(id_user);
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
        QMessageBox::information(this,
                                 QGuiApplication::applicationDisplayName(),
                                 tr("Nu sunt citite datele imaginei '%1': %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    ui->image_logo->setPixmap(QPixmap::fromImage(newImage).scaled(400,50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->btnClearLogo->setVisible(true);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning(logWarning()) << metaObject()->className() << "[loadFile]:"
                               << tr("Fișierul logo nu poate fi deschis: %1").arg(file.errorString());
        return false;
    }
    m_logoData = file.readAll();
    dataWasModified();
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

void UserPreferences::onLinkActivatedForOpenImage(const QString &link)
{
    if (link != "#LoadImage")
        return;

    if (m_Id == idx_unknow){
        QMessageBox::StandardButton YesNo;
        YesNo = QMessageBox::warning(this,
                                     tr("Verificarea valid\304\203rii"),
                                     tr("Pentru a \303\256nc\304\203rca logotipul este necesar de salvat datele."
                                        "<br>Dori\310\233i s\304\203 salva\310\233i datele ?"),
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
                             tr("Dori\310\233i s\304\203 elimina\310\233i logotipul din baza de date ?"),
                             QMessageBox::Yes | QMessageBox::No, this);

    if (messange_box.exec() == QMessageBox::No)
        return;

    m_logoData.clear();
    ui->image_logo->clear();
    ui->image_logo->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea"
                            "<br>(se recomanda 400x50 px)</a>");
    ui->image_logo->setTextFormat(Qt::RichText);
    ui->image_logo->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->btnClearLogo->setVisible(false);
    dataWasModified();
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
    else if (index.row() == page_message)
        ui->stackedWidget->setCurrentIndex(page_message);     
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

    UserDialog userDialog(m_db, this);
    userDialog.setProperty("isNew", false);
    userDialog.setProperty("id", m_Id);
    userDialog.exec();
}

void UserPreferences::onOpenCatDoctors()
{
    if (m_idDoctor == idx_unknow)
        return;

    CatalogDialog cat(m_db, CatalogType::Type::Doctors, this);
    cat.setProperty("isNew", false);
    cat.setProperty("id", m_idDoctor);
    cat.exec();
}

void UserPreferences::onOpenCatNurses()
{
    if (m_idNurse == idx_unknow)
        return;

    CatalogDialog cat(m_db, CatalogType::Type::Nurses, this);
    cat.setProperty("isNew", false);
    cat.setProperty("id", m_idNurse);
    cat.exec();
}

void UserPreferences::onOpenCatOrganizations()
{
    if (m_IdOrganization == idx_unknow)
        return;

    OrganizationDialog organization(m_db, this);
    organization.setProperty("isNew", false);
    organization.setProperty("id", m_IdOrganization);
    organization.exec();
}

void UserPreferences::changeDataItemTabelMessege(QTableWidgetItem *item)
{
    if (!item || m_loading)
        return;

    const bool checked = item->checkState() == Qt::Checked;
    if (item->row() == row_video)
        m_showVideoMessage = checked;
    else if (item->row() == row_report)
        m_showReportsMessage = checked;
    else
        return;

    dataWasModified();
}

// **********************************************************************************
// --- initierea si setarea modelelor si listWidget

void UserPreferences::setListWidget()
{
    const QStringList LIST_ITEMS = QStringList()
                                   << tr("Setările generale")
                                   << tr("Lansarea/închiderea")
                                   << tr("Setările documentelor")
                                   << tr("Prezentarea mesajelor")
                                   << tr("Setările neredactabile");

    ui->listWidget->setIconSize(QSize(18,18));
    int n = -1;
    foreach( const QString& item, LIST_ITEMS ) {
        n += 1;
        QListWidgetItem* listItem = new QListWidgetItem(item);
#if defined(Q_OS_LINUX)
        if (n == page_general)
            listItem->setIcon(QIcon::fromTheme("preferences-system"));
        else if (n == page_launch)
            listItem->setIcon(QIcon::fromTheme("applications-internet"));
        else if (n == page_document)
            listItem->setIcon(QIcon::fromTheme("edit-paste"));
        else if (n == page_message)
            listItem->setIcon(QIcon(":/img/common/info.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon::fromTheme("emblem-important"));
#elif defined(Q_OS_MACOS)
        if (n == page_general)
            listItem->setIcon(QIcon(":/img/catalogs/settings.png"));
        else if (n == page_launch)
            listItem->setIcon(QIcon(":/img/actions/update_app.png"));
        else if (n == page_document)
            listItem->setIcon(QIcon(":/img/documents/orderEcho.png"));
        else if (n == page_message)
            listItem->setIcon(QIcon(":/img/common/info.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon(":/img/common/not_editable.png"));
#elif defined(Q_OS_WIN)
        if (n == page_general)
            listItem->setIcon(QIcon(":/img/catalogs/settings.png"));
        else if (n == page_launch)
            listItem->setIcon(QIcon(":/img/actions/update_app.png"));
        else if (n == page_document)
            listItem->setIcon(QIcon(":/img/documents/orderEcho.png"));
        else if (n == page_message)
            listItem->setIcon(QIcon(":/img/common/info.png"));
        else if (n == page_notedit)
            listItem->setIcon(QIcon(":/img/common/not_editable.png"));
#endif

        ui->listWidget->addItem(listItem);

    }
    connect(ui->listWidget, &QListWidget::clicked, this, &UserPreferences::onClickedListView);
}

void UserPreferences::setValueIntoTableMessage()
{
    // disconnect
    disconnect(ui->tableWidget, &QTableWidget::itemChanged, this, &UserPreferences::changeDataItemTabelMessege);

    //-- info video
    QTableWidgetItem *itm = ui->tableWidget->item(row_video, column_presentation);
    if (m_showVideoMessage)
        itm->setCheckState(Qt::Checked);
    else
        itm->setCheckState(Qt::Unchecked);

    //-- info reports
    itm = ui->tableWidget->item(row_report, column_presentation);
    if (m_showReportsMessage)
        itm->setCheckState(Qt::Checked);
    else
        itm->setCheckState(Qt::Unchecked);

    // connect
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &UserPreferences::changeDataItemTabelMessege);
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

    QString style_toolButton = m_db.toolButtonStyleForText();
    ui->btnOpenOrganizations->setStyleSheet(style_toolButton);
    ui->btnOpenDoctors->setStyleSheet(style_toolButton);
    ui->btnOpenNurses->setStyleSheet(style_toolButton);
    ui->btnOpenUsers->setStyleSheet(style_toolButton);
    connect(ui->btnOpenOrganizations, &QToolButton::clicked, this, &UserPreferences::onOpenCatOrganizations);
    connect(ui->btnOpenDoctors, &QToolButton::clicked, this, &UserPreferences::onOpenCatDoctors);
    connect(ui->btnOpenNurses, &QToolButton::clicked, this, &UserPreferences::onOpenCatNurses);
    connect(ui->btnOpenUsers, &QToolButton::clicked, this, &UserPreferences::onOpenCatUsers);

    connect(ui->btnClearLogo, &QAbstractButton::clicked, this, &UserPreferences::clearImageLogo);

    connectionsCombo();
    connectionCheckBox();

    connect(ui->tableWidget, QOverload<QTableWidgetItem *>::of(&QTableWidget::itemChanged),
            this, QOverload<QTableWidgetItem *>::of(&UserPreferences::changeDataItemTabelMessege));

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

void UserPreferences::connectionCheckBox()
{
    QList<QCheckBox*> list = this->findChildren<QCheckBox*>();
    for (int n = 0; n < list.count(); n++) {
        connect(list[n], &QCheckBox::clicked, this, &UserPreferences::dataWasModified);
    }

    connect(ui->updateListDoc, QOverload<int>::of(&QSpinBox::valueChanged),
            this, QOverload<int>::of(&UserPreferences::onChangedValueUpdateListDoc));
}

void UserPreferences::resetFormForUser()
{
    m_IdOrganization = idx_unknow;
    m_idDoctor = idx_unknow;
    m_idNurse = idx_unknow;
    m_logoData.clear();

    ui->comboOrganizations->setCurrentIndex(0);
    ui->comboDoctors->setCurrentIndex(0);
    ui->comboNurses->setCurrentIndex(0);
    ui->brandUSG->clear();

    ui->minimizeAppToTray->setChecked(false);
    ui->check_showQuestionClosingApp->setChecked(true);
    ui->check_showDesignerMenuPrint->setChecked(false);
    ui->check_showUserManual->setChecked(false);
    ui->showDocumentsInSeparatWindow->setChecked(false);
    ui->check_splitFullNamePatient->setChecked(false);
    ui->check_newVersion->setChecked(true);
    ui->check_databasesArchiving->setChecked(false);
    ui->showAsistantHelper->setChecked(true);
    ui->updateListDoc->setValue(0);

    ui->image_logo->clear();
    ui->image_logo->setText("<a href=\"#LoadImage\">Apasa pentru a alege imaginea"
                            "<br>(se recomanda 400x50 px)</a>");
    ui->image_logo->setTextFormat(Qt::RichText);
    ui->image_logo->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->btnClearLogo->setVisible(false);
}

void UserPreferences::restoreSelectedUserInCombo()
{
    const QSignalBlocker blocker(ui->comboUsers);
    const auto matches = model_users->match(model_users->index(0, 0), Qt::UserRole,
                                            m_Id, 1, Qt::MatchExactly);
    if (!matches.isEmpty())
        ui->comboUsers->setCurrentIndex(matches.constFirst().row());
}

bool UserPreferences::saveLocalMessagePreferences(QStringList &err)
{
    const bool videoChanged = m_showVideoMessage != m_initialShowVideoMessage;
    const bool reportsChanged = m_showReportsMessage != m_initialShowReportsMessage;

    if (videoChanged
        && !AppSettings::saveInfoMessageVisibility(AppSettings::InfoMessage::Video,
                                                   m_showVideoMessage)) {
        err << tr("Preferința locală pentru mesajul video nu a putut fi salvată.");
        return false;
    }

    if (reportsChanged
        && !AppSettings::saveInfoMessageVisibility(AppSettings::InfoMessage::Reports,
                                                   m_showReportsMessage)) {
        if (videoChanged)
            AppSettings::saveInfoMessageVisibility(AppSettings::InfoMessage::Video,
                                                   m_initialShowVideoMessage);
        err << tr("Preferința locală pentru mesajul rapoartelor nu a putut fi salvată.");
        return false;
    }

    return true;
}

// **********************************************************************************
// --- inserarea si actualizarea datelor

bool UserPreferences::controlRequiredObjects()
{
    if (ui->comboUsers->currentIndex() == 0){
        ui->textBrowser->setHtml(tr("%1 Nu este indicat utilizatorul !!!").arg(m_db.getHTMLImageWarning()));
        ui->textBrowser->setFixedHeight(45);
        ui->dockWidget->setFixedHeight(90);
        ui->dockWidget->show();
        return false;
    }

    return true;
}

bool UserPreferences::recordExists(const QString &tableName, bool &exists, QStringList &err) const
{
    exists = false;
    if (tableName != QStringLiteral("constants")
        && tableName != QStringLiteral("userPreferences")) {
        err << tr("Tabela '%1' nu este acceptată pentru verificarea preferințelor.").arg(tableName);
        return false;
    }

    QSqlQuery qry(m_db.getDatabase());
    if (!qry.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE id_users = ?").arg(tableName))) {
        err << qry.lastError().text();
        return false;
    }
    qry.addBindValue(m_Id);
    if (!qry.exec() || !qry.next()) {
        err << tr("Verificarea tabelei '%1' a eșuat: %2").arg(tableName, qry.lastError().text());
        return false;
    }

    const int count = qry.value(0).toInt();
    if (count > 1) {
        err << tr("Tabela '%1' conține %2 înregistrări pentru utilizatorul cu ID %3. "
                  "Salvarea a fost oprită pentru a evita modificarea ambiguă a datelor.")
                   .arg(tableName).arg(count).arg(m_Id);
        return false;
    }
    exists = count == 1;
    return true;
}

bool UserPreferences::insertDataIntoTableConstants(QStringList &err)
{
    // verificam daca a fost transmis corect ID utilizatorului
    if (m_Id == 0) {
        err << this->metaObject()->className()
            << "[insertDataIntoTableConstants]:"
            << "Valoarea id_users (m_Id) nu este validă.";
        qCritical(logCritical()) << err;
        return false;
    }

    // pregatim datele
    QVariantMap map;
    map["id_users"]         = m_Id;
    map["id_organizations"] = (m_IdOrganization == idx_unknow) ? QVariant() : m_IdOrganization;
    map["id_doctors"]       = (m_idDoctor == idx_unknow) ? QVariant() : m_idDoctor;
    map["id_nurses"]        = (m_idNurse == idx_unknow) ? QVariant() : m_idNurse;
    map["brandUSG"]         = (ui->brandUSG->text().isEmpty()) ? QVariant() : ui->brandUSG->text();

    map["logo"] = m_logoData.isEmpty()
                      ? QVariant(QMetaType(QMetaType::QByteArray))
                      : m_logoData.toBase64();

    // inseram datele prin functia universala
    return m_db.insertIntoTable(this->metaObject()->className(), "constants", map, err);
}

bool UserPreferences::insertDataIntoTableUserPreferences(QStringList &err)
{
    // verificam daca a fost corect trnasmis ID utilizatorului
    if (m_Id == 0) {
        err << this->metaObject()->className()
            << "[insertDataIntoTableUserPreferences]:"
            << "Valoarea id_users (m_Id) nu este validă.";
        qCritical(logCritical()) << err;
        return false;
    }

    // functia helper universal - converteste bool in QVariant
    auto toDbBool = [](bool value) -> QVariant {
        return globals().thisMySQL ? QVariant(value) : QVariant(int(value));
    };

    // pregatim datele
    QVariantMap map;
    map["id_users"]              = m_Id;
    map["versionApp"]            = USG_VERSION_FULL;
    map["showQuestionCloseApp"]  = toDbBool(ui->check_showQuestionClosingApp->isChecked());
    map["showUserManual"]        = toDbBool(ui->check_showUserManual->isChecked());
    map["showHistoryVersion"]    = 0;
    map["order_splitFullName"]   = toDbBool(ui->check_splitFullNamePatient->isChecked());
    map["updateListDoc"]         = ui->updateListDoc->value();
    map["showDesignerMenuPrint"] = toDbBool(ui->check_showDesignerMenuPrint->isChecked());
    map["checkNewVersionApp"]    = toDbBool(ui->check_newVersion->isChecked());
    map["databasesArchiving"]    = toDbBool(ui->check_databasesArchiving->isChecked());
    map["showAsistantHelper"]    = toDbBool(ui->showAsistantHelper->isChecked());
    map["showDocumentsInSeparatWindow"] = toDbBool(ui->showDocumentsInSeparatWindow->isChecked());
    map["minimizeAppToTray"] = toDbBool(ui->minimizeAppToTray->isChecked());

    // inserarea datelor
    return m_db.insertIntoTable(this->metaObject()->className(), "userPreferences", map, err);
}

bool UserPreferences::updateDataIntoTableConstants(QStringList &err)
{
    // verificam daca a fost transmis corect ID utilizatorului
    if (m_Id == 0) {
        err << this->metaObject()->className()
            << "[updateDataIntoTableConstants]:"
            << "Valoarea id_users (m_Id) nu este validă.";
        qCritical(logCritical()) << err;
        return false;
    }

    // pregatim datele
    QVariantMap map;
    map["id_users"]         = m_Id;
    map["id_organizations"] = (m_IdOrganization == idx_unknow) ? QVariant() : m_IdOrganization;
    map["id_doctors"]       = (m_idDoctor == idx_unknow) ? QVariant() : m_idDoctor;
    map["id_nurses"]        = (m_idNurse == idx_unknow) ? QVariant() : m_idNurse;
    map["brandUSG"]         = (ui->brandUSG->text().isEmpty()) ? QVariant() : ui->brandUSG->text();

    map["logo"] = m_logoData.isEmpty()
                      ? QVariant(QMetaType(QMetaType::QByteArray))
                      : m_logoData.toBase64();

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id_users"] = m_Id;

    // actualizam datele prin functia universala
    return m_db.updateTable(this->metaObject()->className(), "constants", map, where, err);
}

bool UserPreferences::updateDataIntoTableUserPreferences(QStringList &err)
{
    // verificam daca a fost transmisa corect ID utilizatorului
    if (m_Id == 0) {
        err << this->metaObject()->className()
            << "[updateDataIntoTableUserPreferences]:"
            << "Valoarea id_users (m_Id) nu este validă.";
        qCritical(logCritical()) << err;
        return false;
    }

    // functia helper universal - converteste bool in QVariant
    auto toDbBool = [](bool value) -> QVariant {
        return globals().thisMySQL ? QVariant(value) : QVariant(int(value));
    };

    // pregatim datele
    QVariantMap map;
    map["versionApp"]            = USG_VERSION_FULL;
    map["showQuestionCloseApp"]  = toDbBool(ui->check_showQuestionClosingApp->isChecked());
    map["showUserManual"]        = toDbBool(ui->check_showUserManual->isChecked());
    map["showHistoryVersion"]    = 0;
    map["order_splitFullName"]   = toDbBool(ui->check_splitFullNamePatient->isChecked());
    map["updateListDoc"]         = ui->updateListDoc->value();
    map["showDesignerMenuPrint"] = toDbBool(ui->check_showDesignerMenuPrint->isChecked());
    map["checkNewVersionApp"]    = toDbBool(ui->check_newVersion->isChecked());
    map["databasesArchiving"]    = toDbBool(ui->check_databasesArchiving->isChecked());
    map["showAsistantHelper"]    = toDbBool(ui->showAsistantHelper->isChecked());
    map["showDocumentsInSeparatWindow"] = toDbBool(ui->showDocumentsInSeparatWindow->isChecked());
    map["minimizeAppToTray"] = toDbBool(ui->minimizeAppToTray->isChecked());

    // pregatim conditia
    QMap<QString, QVariant> where;
    where["id_users"] = m_Id;

    // actualizam datele
    return m_db.updateTable(this->metaObject()->className(), "userPreferences", map, where, err);
}

// **********************************************************************************
// --- evenimentele formei

void UserPreferences::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()){
        QMessageBox messange_box(QMessageBox::Question,
                                 tr("Modificarea datelor"),
                                 tr("Datele au fost modificate.\n"
                                    "Dori\310\233i s\304\203 salva\310\233i aceste modific\304\203ri ?"),
                                 QMessageBox::NoButton, this);
        QPushButton *yesButton    = messange_box.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton *noButton     = messange_box.addButton(tr("Nu"), QMessageBox::NoRole);
        yesButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
        noButton->setStyleSheet(m_db.getStyleForButtonMessageBox());
        messange_box.exec();

        if (messange_box.clickedButton() == yesButton) {
            if (onWritingData())
                event->accept();
            else
                event->ignore();
        } else if (messange_box.clickedButton() == noButton) {
            event->accept();
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
        event->accept();
        return;
    }
    QDialog::keyPressEvent(event);
}
