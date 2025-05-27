#ifndef CATORGANIZATIONS_H
#define CATORGANIZATIONS_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMap>
#include <QMapIterator>
#include <QStandardPaths>

#include "data/database.h"
#include <data/appsettings.h>
#include <data/popup.h>
#include <data/globals.h>
#include "catalogs/catcontracts.h"
#include "models/basesqlquerymodel.h"

namespace Ui {
class CatOrganizations;
}

class CatOrganizations : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool itNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(QString NameOrganization READ getNameOrganization WRITE setNameOrganization NOTIFY NameOrganizationChanged)
    Q_PROPERTY(int IdMainContract READ getIdMainContract WRITE setIdMainContract NOTIFY IdMainContractChanged)

public:
    explicit CatOrganizations(QWidget *parent = nullptr);
    ~CatOrganizations();

    void setItNew(bool itNew) {m_itNew = itNew; emit ItNewChanged();}
    bool getItNew() {return m_itNew;}

    void setId(const int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

    void setNameOrganization(const QString NameOrganization) {m_name_organization = NameOrganization; emit NameOrganizationChanged();}
    QString getNameOrganization() {return m_name_organization;}

    void setIdMainContract(const int IdMainContract) {m_IdMainContract = IdMainContract; emit IdMainContractChanged();}
    int getIdMainContract() const {return m_IdMainContract;}

signals:
    void IdChanged();      // p-u conectarea la slot changedIdObject()
    void ItNewChanged();
    void NameOrganizationChanged();
    void IdMainContractChanged();
    void mWriteNewOrganization();
    void mChangedDateOrganization();

private slots:
    void controlLengthComment();
    void dataWasModified();

    void slot_ItNewChanged();
    void slot_IdChanged();
    void textChangedNameOrganization();

    void clearImageStamp();
    bool loadFile(const QString &fileName);
    void onLinkActivatedForOpenImage(const QString &link);

    void createNewContract();
    void editContract();
    void markDeletionContract();
    void setDefaultContract();
    void slot_updateTableContracts();
    void onDoubleClickedTableContracts(const QModelIndex &index);

    bool onWritingData();
    void onWritingDataClose();

private:
    void initConnections();
    void connectionsModified();
    void disconnectionsModified();
    void initBtnToolBar();
    void updateTableContracts();
    void updateHeaderTableContracts();

    bool controlRequiredObjects();
    bool confirmIfDuplcateExist();
    bool handleInsert();
    bool handleUpdate();
    bool insertIntoTableOrganizations();
    bool updateDataTableOrganizations();

    enum sectionsContract
    {
        sectionsContract_id               = 0,
        sectionsContract_deletionMark     = 1,
        sectionsContract_id_organizations = 2,
        sectionsContract_id_typesPrices   = 3,
        sectionsContract_name             = 4,
        sectionsContract_dateInit         = 5,
        sectionsContract_valid            = 6,
        sectionsContract_comment          = 7
    };

    enum sectionsQry
    {
        sectionsQry_id           = 0,
        sectionsQry_deletionMark = 1,
        sectionsQry_dateInit     = 2,
        sectionsQry_name         = 3
    };

private:
    Ui::CatOrganizations *ui;

    bool m_itNew = false;       // obiectul nou creat
    int m_Id     = -1;          // id obiectului
    int m_IdMainContract = -1;  // id contractului de baza
    QString m_name_organization = nullptr;

    DataBase *db;
    PopUp    *popUp;

    CatContracts      *catContracts;
    BaseSqlQueryModel *modelCantract;

    QStringList err;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // CATORGANIZATIONS_H
