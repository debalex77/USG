#ifndef ORGANIZATIONDIALOG_H
#define ORGANIZATIONDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>

#include <catalogs/contractdialog.h>

#include <common/property_macros.h>
#include <common/table_sections.h>
#include <common/balloontip.h>
#include <common/reportsettingsmanager.h>
#include <common/appmetatypes.h>

#include <models/organizationcontractmodel.h>

#include <customs/toolbarcustom.h>

#include <customs/custommessage.h>

#include <data/database.h>
#include <data/popup.h>
#include <common/globals.h>

namespace Ui {
class OrganizationDialog;
}

class OrganizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrganizationDialog(DataBase &db, QWidget *parent = nullptr);
    ~OrganizationDialog();

    // declaram proprietatile si functiile setter/getter + functia de update
    // DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc)
    // vezi common/property_macros.h
    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(int, idContract, IdContract, idContractChanged, slot_IdContractChanged)
    DECLARE_PROPERTY_UPDATE(StatusObject::Column, statusCatalog, StatusCatalog, statusCatalogChanged, slot_StatusCatalogChanged)

    // procedura de marcare pu eliminare din view
    bool setDeleteMarkOrganization(QString &err);

signals:
    void isNewChanged();
    void idChanged();
    void idContractChanged();
    void statusCatalogChanged();

    void organizationCreated(const QVector<QVariant> &dataOrganization);
    void organizationChanged(const QVector<QVariant> &dataOrganization);
    void organizationDeletedMark();

private slots:
    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_IdContractChanged();
    void slot_StatusCatalogChanged();

    void dataWasModified();

    void onAddContract();
    void onEditContract();
    void onDeleteContract();
    void onShowHideColumnTableContract();
    void onSetContractMain();
    void onDeleteContractMain();

    void clearImageStamp();
    bool loadFile(const QString &fileName);
    void onLinkActivatedForOpenImage(const QString &link);

    void controlLengthComment();

    void onDoubleClickedTableContracts(const QModelIndex &index);

private:
    void initModelContract();
    void updateModelContract();

    void loadFilterBySettings(); // pu tableContract

    void initBtnToolBar();
    void initConnections();

    bool confirmSaveIfModified();

    bool controlRequiredObjects();
    bool confirmIfDuplicateExists();
    bool handleInsert();
    bool handleUpdate();
    bool handleDeletionMark(QString &err);
    bool onWritingData();
    void onWritingDataClose();

    void saveSizeSectionsContractView();
    void loadsizeSectionsContractView();

private:
    Ui::OrganizationDialog *ui;
    ReportSettingsManager m_settings;
    CatalogViewFilter m_filterContract;

    const QString className = "OrganizationDialog";

    DataBase &m_db;
    PopUp    *popUp;
    QString styleBtnMessageBox;

    ToolBarCustom *toolBar;

    OrganizationContractModel *modelContract;

    QStringList err;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)

};

#endif // ORGANIZATIONDIALOG_H
