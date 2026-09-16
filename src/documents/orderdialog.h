#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCompleter>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <LimeReport>
#include <QStyleFactory>
#include <QDomDocument>

#include <catalogs/catalogdialog.h>
#include <catalogs/patienthistory.h>

#include <common/appmetatypes.h>
#include <common/globals.h>
#include <common/table_sections.h>
#include <common/property_macros.h>
#include <common/handlerfunctionthread.h>
#include <common/balloontip.h>
#include <common/reportsettingsmanager.h>

#include <documents/reportdialog.h>

#include <data/popup.h>
#include <data/database.h>
#include <customs/customdialoginvestig.h>

#include <models/queryrolesmodel.h>
#include <models/tabledocmodel.h>
#include <models/sortmodel.h>
#include <models/orderinvestigationmodel.h>

#include <threads/databaseprovider.h>
#include <threads/patientsaverworker.h>
#include <threads/syncpatientworker.h>
#include <threads/syncorderworker.h>

namespace Ui {
class OrderDialog;
}

class OrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDialog(DataBase &db, QWidget *parent = nullptr);
    ~OrderDialog();

    struct PrefillData {
        int organizationId = 0;
        int referringDoctorId = 0;
        int patientId = 0;
        QString patientText;
        QList<int> investigationIds;
    };

    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(int, idOrganization, IdOrganization, idOrganizationChanged, slot_IdOrganizationChanged)
    DECLARE_PROPERTY_UPDATE(int, idContract, IdContract, idContractChanged, slot_IdContractChanged)
    DECLARE_PROPERTY_UPDATE(int, idTypePrice, IdTypePrice, idTypePriceChanged, slot_IdTypePriceChanged)
    DECLARE_PROPERTY_UPDATE(int, idPatient, IdPatient, idPatientChanged, slot_IdPatientChanged)
    DECLARE_PROPERTY_UPDATE(int, idNurse, IdNurse, idNurseChanged, slot_IdNurseChanged)
    DECLARE_PROPERTY_UPDATE(int, idPerformingDoctor, IdPerformingDoctor, idPerformingDoctorChanged, slot_IdPerformingDoctorChanged)
    DECLARE_PROPERTY_UPDATE(int, idRefferingDoctor, IdRefferingDoctor, idRefferingDoctorChanged, slot_IdRefferingDoctorChanged)
    DECLARE_PROPERTY_UPDATE(int, idUser, IdUser, idUserChanged, slot_IdUserChanged)
    DECLARE_PROPERTY_UPDATE(int, post, Post, postChanged, slot_PostChanged)

    // functiile exportate pu solicitarea din alte clase
    void onPrintDocument(PrintType::Column type_print,
                         const QString &filePDF = QString());
    bool extPostDocument();
    void setSuggestedPatientName(const QString &fullName);
    bool applyPrefillData(const PrefillData &data, QString *errorText = nullptr);

signals:
    void isNewChanged();  // signals proprietatilor
    void idChanged();
    void idOrganizationChanged();
    void idContractChanged();
    void idTypePriceChanged();
    void idPatientChanged();
    void idNurseChanged();
    void idPerformingDoctorChanged(); // a executat
    void idRefferingDoctorChanged();  // a trimis
    void idUserChanged();
    void postChanged();

    void PostDocument();       // conectarea -> 'OrderView'
    void SaveDocument();

    void createNewPacient();   // conectarea -> 'CatalogTableEditor'
    void printToPdfFinished(); // conectarea -> 'OrderView' -> onSendEmail()

private slots:
    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();
    void slot_IdTypePriceChanged();
    void slot_IdPatientChanged();
    void slot_IdNurseChanged();
    void slot_IdPerformingDoctorChanged();
    void slot_IdRefferingDoctorChanged();
    void slot_IdUserChanged();
    void slot_PostChanged();

    void dataWasModified();
    void updateTimerDateDoc();
    void onDateTimeChanged();

    void onCreateNewDoctor();
    void onOpenCatalogDoctor();

    void indexChangedCombo(int index);

    void newPatientStateChanged(const int value);
    bool splitFullNamePatient(QString &_name, QString &_fName);

    void onValidateDataPatient();
    void onEditDataPatient();
    void onClearDataPatient();
    void onOpenPatientHistory();

    void slotPatientTextChanged(const QString &text);
    void updateModelPatientsByText();
    void activatedItemCompleter(const QModelIndex &index);

    void filterRegExpChanged();

    void onDoubleClickedTableSource(const QModelIndex &index);
    void onDoubleClickedTableOrder(const QModelIndex &index);

    void setImageForDocPrint();
    void onPrint(PrintType::Column type_print, const QString &filePDF); // printare

    int valuePaymentOrder() const;
    bool insertDataOrder(QString &details_error);
    bool updateDataOrder(QString &details_error);

    void editRowTableOrder();
    void removeRowTableOrder();
    void slotContextMenuRequested(const QPoint &pos);

    bool deleteRowsOrderTable(QString &details_error);
    bool reinsertAllOrderRows(QString &details_error);

    bool controlRequiredObjects();
    void onOpenReport();
    bool onSave();  // 'btnWrite'
    bool onPost();  // 'btnOk'

private:
    void setupDateDocFormat();

    void updateModelOrganizations();
    void updateModelContracts();
    void updateModelTypesPrices();
    void updateModelPerformingDoctors();
    void updateModelNurses();
    void updateModelRefferingDoctors();

    void setStyleMaxVisibleItemsComboBox();
    void initConnections();

    void setupMaxLengthForDataPatient();
    void initSetCompleter();
    void ensurePatientInCompleterModel(int idPatient, const QString &fullName);
    void loadPatientDetails();
    void initSyncPatientData(PatientDataStructure patientData);
    void initSyncOrderData();

    void handleCompleterAddress(const QString &text);
    QStringList loadDataFromXml(const QString &filePath, const QString &tagName);
    void initSetCompleterAddress();

    void initTableSource();
    void updateTableSource();

    void initTableOrder();
    void updateTableOrder();

    void changeIconForItemToolBox(const int index);

    void setPatientDataEnabled(bool enabled);

    double documentSum() const;
    void updateDocumentSumText();

    void initFooterDoc();

    void saveLayoutSizes();
    void loadLayoutSizes();

    DatabaseProvider *dbProvider();

private:
    Ui::OrderDialog *ui;
    ReportSettingsManager m_settings;

    // structura
    struct DialogLayoutSettings {
        QSize window;
        QMap<int, int> sourceTable;
        QMap<int, int> orderTable;
    };
    DialogLayoutSettings m_layoutSizes;

    // implicite pu documente
    DataBase &m_db;  // conectarea la bd
    QSqlDatabase m_currentDB;

    PopUp    *popUp; // mesaje
    QTimer   *timer; // data si ora
    DatabaseProvider m_provider;

    // modele combobox-lor
    QueryRolesModel *modelOrganizations     = nullptr;
    QueryRolesModel *modelContracts         = nullptr;
    QueryRolesModel *modelTypePrices        = nullptr;
    QueryRolesModel *modelPerformingDoctors = nullptr; // a executat
    QueryRolesModel *modelNurses            = nullptr;
    QueryRolesModel *modelRefferingDoctors  = nullptr; // a trimis

    // pacient
    QStandardItemModel *modelPatients      = nullptr;
    QCompleter         *completerPatients  = nullptr;
    QTimer             *timerPatientSearch = nullptr;

    // completarea adresei
    QCompleter *completerCity = nullptr;
    QStringList cityList;

    // tabele
    OrderInvestigationModel *modelTableSource = nullptr;
    OrderInvestigationModel *modelTableOrder  = nullptr;
    SortModel         *proxy            = nullptr;
    int m_tempOrderRowId = -1; // id temporare negative pu 'modelTableOrder'

    // atasarea imaginilor
    int m_attachedImages = StatusObject::Unknow;

    // printare
    int exist_logo         = StatusObject::ZeroWrite; // variabile pu forma de tipar
    int exist_stamp        = StatusObject::ZeroWrite;
    int exist_stamp_doctor = StatusObject::ZeroWrite;
    int exist_signature    = StatusObject::ZeroWrite;
    QStandardItemModel *model_img = nullptr;

    // other
    bool m_postInProgress = false;
    QString toolButtonStyleForText;
    QString styleForButtonMessageBox;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // ORDERDIALOG_H
