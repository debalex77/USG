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

#include <features/catalogs/catalogdialog.h>
#include <features/patients/patienthistory.h>

#include <common/appmetatypes.h>
#include <common/globals.h>
#include <common/table_sections.h>
#include <ui/widgets/balloontip.h>
#include <common/orderdialogcontext.h>
#include <settings/reportsettingsmanager.h>

#include <features/reports/reportdialog.h>

#include <app/popup.h>
#include <database/database.h>
#include <ui/dialogs/customdialoginvestig.h>

#include <models/queryrolesmodel.h>
#include <models/tabledocmodel.h>
#include <models/sortmodel.h>
#include <features/orders/orderinvestigationmodel.h>

#include <infrastructure/database/databaseprovider.h>
#include <infrastructure/persistence/patientsaverworker.h>
#include <infrastructure/sync/syncpatientworker.h>
#include <infrastructure/sync/syncorderworker.h>

namespace Ui {
class OrderDialog;
}

class OrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDialog(DataBase &db, QWidget *parent = nullptr);
    OrderDialog(DataBase &db, int orderId, QWidget *parent = nullptr);
    ~OrderDialog();

    struct PrefillData {
        int organizationId = 0;
        int referringDoctorId = 0;
        int patientId = 0;
        QString patientText;
        QList<int> investigationIds;
    };

    // functiile exportate pu solicitarea din alte clase
    void onPrintDocument(PrintType::Column type_print,
                         const QString &filePDF = QString());
    bool extPostDocument();
    void setSuggestedPatientName(const QString &fullName);
    bool applyPrefillData(const PrefillData &data, QString *errorText = nullptr);

signals:
    void PostDocument();       // conectarea -> 'OrderView'
    void SaveDocument();

    void createNewPacient();   // conectarea -> 'CatalogTableEditor'
    void printToPdfFinished(); // conectarea -> 'OrderView' -> onSendEmail()

private slots:
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
    void initializeNewOrder();
    void loadOrder();

    [[nodiscard]] bool isNewDocument() const;
    [[nodiscard]] int orderId() const;
    [[nodiscard]] int organizationId() const;
    [[nodiscard]] int contractId() const;
    [[nodiscard]] int priceTypeId() const;
    [[nodiscard]] int patientId() const;
    [[nodiscard]] int nurseId() const;
    [[nodiscard]] int performingDoctorId() const;
    [[nodiscard]] int referringDoctorId() const;
    [[nodiscard]] int authorUserId() const;
    [[nodiscard]] int documentStatus() const;

    void setOrderId(int value);
    void setIdOrganization(int value);
    void setIdContract(int value);
    void setIdTypePrice(int value);
    void setIdPatient(int value);
    void setIdNurse(int value);
    void setIdPerformingDoctor(int value);
    void setIdRefferingDoctor(int value);
    void setIdUser(int value);
    void setPost(int value);

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

    QStringList selectedInvestigationCodes() const;
    ReportSections::ConsentTypes selectedConsentTypes() const;
    QString informedConsentText() const;

    void initFooterDoc();
    void setPrintModelOrganization(QSqlQueryModel *model);

    void saveLayoutSizes();
    void loadLayoutSizes();

    DatabaseProvider *dbProvider();

private:
    Ui::OrderDialog *ui;
    ReportSettingsManager m_settings;
    OrderDialogContext m_documentContext;

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
