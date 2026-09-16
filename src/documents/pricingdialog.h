#ifndef PRICINGDIALOG_H
#define PRICINGDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QStyleFactory>
#include <QTimer>
#include <LimeReport>

#include <common/table_sections.h>
#include <common/property_macros.h>

#include <data/popup.h>
#include <common/globals.h>
#include <data/database.h>

#include <models/queryrolesmodel.h>
#include <models/tabledocmodel.h>
#include <models/sortmodel.h>

#include <views/catalogtableeditor.h>
#include <catalogs/organizationdialog.h>
#include <catalogs/contractdialog.h>

#include <customs/custommessage.h>

namespace Ui {
class PricingDialog;
}

class PricingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PricingDialog(DataBase &db, QWidget *parent = nullptr);
    ~PricingDialog();

    // declaram proprietatile si functiile setter/getter + functia de update
    // DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc)
    // vezi common/property_macros.h
    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(int, idOrganization, IdOrganization, idOrganizationChanged, slot_IdOrganizationChanged)
    DECLARE_PROPERTY_UPDATE(int, idContract, IdContract, idContractChanged, slot_IdContractChanged)
    DECLARE_PROPERTY_UPDATE(int, idTypePrice, IdTypePrice, idTypePriceChanged, slot_IdTypePriceChanged)
    DECLARE_PROPERTY_UPDATE(int, idUser, IdUser, idUserChanged, slot_IdUserChanged)
    DECLARE_PROPERTY_UPDATE(int, post, Post, postChanged, slot_PostChanged)

    // finctiile export din alte clase
    void onPrintDocument(PrintType::Column type_print);

signals:
    void isNewChanged();
    void idChanged();
    void idOrganizationChanged();
    void idContractChanged();
    void idTypePriceChanged();
    void idUserChanged();
    void postChanged();
    void PostDocument();
    void mCloseThisForm(); // pu eliminare memoriei

private slots:
    void dataWasModified();
    void updateTimer();        // actualizarea datei si orei in regim real
    void onDateTimeChanged();

    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();
    void slot_IdTypePriceChanged();
    void slot_IdUserChanged();
    void slot_PostChanged();

    void indexChangedCombo(int index); // modificarea indexurilor combobox-urilor

    void openCatOrganization();
    void openCatContract();

    void addRowTable();          // *** slot-urile de adaugare
    void editRowTable();         // editare
    void deletionRowTable();     // eliminare din tabel
    void populateTable();        // *** btn completare din BD -> clasa 'CatalogTableEditor'
    void filterRegExpChanged();

    void getDataSelectable(const QVariantMap data); // determinarea si setarea datelor selectate din
    // 'CatalogTableEditor' in rand nou

    void onClickedRowTable(const QModelIndex &index); // la click-ul pe rand intram in regimul de redactare

    void onPrint(PrintType::Column type_print);
    bool controlRequiredObjects();
    bool onWritingData();
    void onWritingDataClose();
    void onClose();

    void slotGetCallbackData(LimeReport::CallbackInfo info, QVariant &data);
    void slotGetCallbackDataItems(LimeReport::CallbackInfo info, QVariant &data);
    void prepareData(QSqlQuery *qry, LimeReport::CallbackInfo info, QVariant &data);
    void slotChangePos(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);
    void slotChangePosItems(const LimeReport::CallbackInfo::ChangePosType &type, bool &result);

private:
    void setTitleDoc();
    void setupDateDocFormat();
    void initBtnToolBar();
    void initFooterDoc();

    void updateModelOrganizations();
    void updateModelContracts();
    void updateModelTypesPrices();

    void initTable();
    void updateTableView();
    void updateHeaderTable();

    bool insertDataTablePricings(QString &err);
    bool updateDataTablePricings(QString &err);

    void initConnections();

private:
    Ui::PricingDialog *ui;
    DataBase &m_db;
    PopUp    *popUp;

    QueryRolesModel *modelOrganizations = nullptr;
    QueryRolesModel *modelContracts     = nullptr;
    QueryRolesModel *modelTypesPrices   = nullptr;

    TableDocModel *modelTable = nullptr;
    SortModel     *proxy = nullptr;

    QMenu *menu;
    QLabel *labelAuthor = nullptr;
    QTimer* timer;

    LimeReport::ReportEngine *m_report = nullptr;
    QSqlQuery *m_owner = nullptr;
    QSqlQuery *m_investigations = nullptr;

    QString styleForButtonMessageBox;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // PRICINGDIALOG_H
