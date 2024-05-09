#ifndef USERPREFERENCES_H
#define USERPREFERENCES_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTableWidgetItem>

#include <catalogs/catgeneral.h>
#include <catalogs/catusers.h>
#include <catalogs/catorganizations.h>
#include "data/database.h"
#include "models/basesqlquerymodel.h"
#include "popup.h"

namespace Ui {
class UserPreferences;
}

class UserPreferences : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged) // user
    Q_PROPERTY(int Id_Organization READ getIdOrganization WRITE setIdOrganization NOTIFY IdChangedOrganization)
    Q_PROPERTY(int IdDoctor READ getIdDoctor WRITE setIdDoctor NOTIFY IdDoctorChanged)
    Q_PROPERTY(int IdNurse READ getIdNurse WRITE setIdNurse NOTIFY IdNurseChanged)

public:
    explicit UserPreferences(QWidget *parent = nullptr);
    ~UserPreferences();

    void setId(int Id)
    {m_Id = Id; emit IdChanged();};  // proprietatea - id obiectului
    int getId() const
    {return m_Id;};

    void setIdOrganization(int IdOrganization)
    {m_IdOrganization = IdOrganization; emit IdChangedOrganization();};
    int getIdOrganization() const
    {return m_IdOrganization;};

    void setIdDoctor(int IdDoctor)
    {m_idDoctor = IdDoctor; emit IdDoctorChanged();}
    int getIdDoctor() const
    {return m_idDoctor;}

    void setIdNurse(int IdNurse)
    {m_idNurse = IdNurse; emit IdNurseChanged();}
    int getIdNurse() const
    {return m_idNurse;}

signals:
    void IdChanged();    // p-u conectarea la slot slot_IdChanged()
    void IdChangedOrganization();
    void IdDoctorChanged();
    void IdNurseChanged();

private slots:
    void dataWasModified();

    bool onWritingData();
    void onWritingDataClose();

    void slot_IdChanged();
    void slot_IdDoctorChanged();
    void slot_IdNurseChanged();
    void slot_IdChangedOrganization();

    void activatedComboUsers(const int index);
    void activatedComboDoctors(const int index);
    void activatedComboNurses(const int index);
    void activatedComboOrganizations(const int index);

    bool loadFile(const QString &fileName);
    void onLinkActivatedForOpenImage(const QString &link);
    void clearImageLogo();

    void onClickedListView(const QModelIndex index);
    void onChangedValueUpdateListDoc(const int value);

    void onOpenCatUsers();
    void onOpenCatDoctors();
    void onOpenCatNurses();
    void onOpenCatOrganizations();

    void changeDataItemTabelMessege(QTableWidgetItem *item);

private:
    void setListWidget();
    void setValueIntoTableMessage();
    void initSetModels();
    void initConnections();
    void connectionsCombo();
    void disconnectCombo();
    void connectionCheckBox();
    void disconnectionCheckBox();

    bool controlRequiredObjects();
    bool existRecordInTable();
    bool insertDataIntoTable();
    bool updateDataIntoTable();

private:
    Ui::UserPreferences *ui;

    enum idx
    {
        idx_unknow   = -1,
        idx_write    = 0,
        idx_deletion = 1
    };

    enum idx_page
    {
        page_general  = 0,
        page_launch   = 1,
        page_document = 2,
        page_message  = 3,
        page_notedit  = 4
    };

    enum row_table_message {
        row_video  = 0,
        row_report = 1
    };

    enum column_table_message {
        column_msg          = 0,
        column_presentation = 1
    };

    int m_Id             = idx_unknow;
    int m_IdOrganization = idx_unknow;
    int m_idDoctor       = idx_unknow;
    int m_idNurse        = idx_unknow;

    DataBase *db;
    PopUp    *popUp;

    BaseSqlQueryModel *model_users;
    BaseSqlQueryModel *model_doctors;
    BaseSqlQueryModel *model_nurses;
    BaseSqlQueryModel *model_organizations;

    AppSettings *app_settings;

    QStyle *style_fusion = QStyleFactory::create("Fusion");

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
};

#endif // USERPREFERENCES_H
