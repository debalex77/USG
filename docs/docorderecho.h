#ifndef DOCORDERECHO_H
#define DOCORDERECHO_H

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

#include "data/popup.h"
#include <data/globals.h>
#include <data/enums.h>
#include "data/database.h"
#include "catalogs/catgeneral.h"
#include "models/basesqlquerymodel.h"
#include "models/basesqltablemodel.h"
#include "models/basesortfilterproxymodel.h"
#include <data/customdialoginvestig.h>
#include <catalogs/patienthistory.h>

// https://qtexcel.github.io/QXlsx/Example.html
//#include <QXlsx/header/xlsxdocument.h>
//#include <QXlsx/header/xlsxformat.h>
//#include <QXlsx/header/xlsxcellrange.h>
//#include <QXlsx/header/xlsxworksheet.h>

namespace Ui {
class DocOrderEcho;
}

class DocOrderEcho : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool ItNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(int IdOrganization READ getIdOrganization WRITE setIdOrganization NOTIFY IdOrganizationChanged)
    Q_PROPERTY(int IdContract READ getIdContract WRITE setIdContract NOTIFY IdContractChanged)
    Q_PROPERTY(int IdTypePrice READ getIdTypePrice WRITE setIdTypePrice NOTIFY IdTypePriceChanged)
    Q_PROPERTY(int IdPacient READ getIdPacient WRITE setIdPacient NOTIFY IdPacientChanged)
    Q_PROPERTY(int IdNurse READ getIdNurse WRITE setIdNurse NOTIFY IdNurseChanged)
    Q_PROPERTY(int IdDoctor READ getIdDoctor WRITE setIdDoctor NOTIFY IdDoctorChanged)
    Q_PROPERTY(int IdDoctorExecute READ getIdDoctorExecute WRITE setIdDoctorExecute NOTIFY IdDoctorChangedExecute)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(int Post READ getPost WRITE setPost NOTIFY PostChanged)
    Q_PROPERTY(QString NamePatient READ getNamePatient WRITE setNamePatient NOTIFY NamePatientChanged)

public:
    explicit DocOrderEcho(QWidget *parent = nullptr);
    ~DocOrderEcho();

    void setItNew(bool ItNew) {m_itNew = ItNew; emit ItNewChanged();}
    bool getItNew() const {return m_itNew;}

    void setId(int Id) {m_id = Id; emit IdChanged();}
    int getId() const {return m_id;}

    void setIdOrganization(int IdOrganization) {m_idOrganization = IdOrganization; emit IdOrganizationChanged();}
    int getIdOrganization() const {return m_idOrganization;}

    void setIdContract(int IdContract) {m_idContract = IdContract; emit IdContractChanged();}
    int getIdContract() const {return m_idContract;}

    void setIdTypePrice(int IdTypePrice) {m_idTypePrice = IdTypePrice; emit IdTypePriceChanged();}
    int getIdTypePrice() const {return m_idTypePrice;}

    void setIdPacient(int IdPacient) {m_idPacient = IdPacient; emit IdPacientChanged();}
    int getIdPacient() const {return m_idPacient;}

    void setIdNurse(int IdNurse) {m_idNurse = IdNurse; emit IdNurseChanged();}
    int getIdNurse() const {return m_idNurse;}

    void setIdDoctor(int IdDoctor) {m_idDoctor = IdDoctor; emit IdDoctorChanged();}
    int getIdDoctor() const {return m_idDoctor;}

    void setIdDoctorExecute(int IdDoctorExecute) {m_idDoctor_execute = IdDoctorExecute; emit IdDoctorChangedExecute();}
    int getIdDoctorExecute() const {return m_idDoctor_execute;}

    void setIdUser(int IdUser) {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const {return m_idUser;}

    void setPost(int Post) {m_post = Post; emit PostChanged();}
    int getPost() const {return m_post;}

    void setNamePatient(QString NamePatient) {m_name_patient = NamePatient; emit NamePatientChanged();}
    QString getNamePatient() const {return m_name_patient;}

    void onPrintDocument(Enums::TYPE_PRINT type_print); // functiile exportate pu solicitarea din alte clase
    void m_OnOpenReport();  // ex.: 'ListDocWebOrder'
    void m_onWritingData();

signals:
    void mCloseThisForm();   // pu eliberarea memoriei la inchiderea paginei vezi 'MainWindow'
    void ItNewChanged();
    void IdChanged();
    void IdOrganizationChanged();
    void IdContractChanged();
    void IdTypePriceChanged();
    void IdPacientChanged();
    void IdNurseChanged();
    void IdDoctorChanged();
    void IdDoctorChangedExecute();
    void IdUserChanged();
    void PostChanged();
    void NamePatientChanged();
    void PostDocument();      // pu conectarea si actualizarea 'listFormOrderReport'
    void mCreateNewPacient(); // pu conectarea si actualizarea 'listForm'

private slots:
    void controlLengthComment();
    void dataWasModified();    // modificarea datelor formei
    void updateTimer();        // actualizarea datei si orei in regim real
    void onDateTimeChanged();
    void changeIconForItemToolBox(const int _index);

    void createCatDoctor();
    void openCatDoctor();

    void slot_ItNewChanged();  // slot-urile la signal-uri
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();
    void slot_IdTypePriceChanged();
    void slot_IdPacientChanged();
    void slot_IdNurseChanged();
    void slot_IdDoctorChanged();
    void slot_IdDoctorExecuteChanged();
    void slot_IdUserChanged();
    void slot_NamePatientChanged();

    void activatedItemCompleter(const QModelIndex &index); // activarea 'completer'

    void stateChangedCheckBox(const int value);            // modificarea  statutului 'checkBox(nou)'
    void indexChangedComboOrganization(const int index);   // modificarea indexurilor combobox-urilor
    void indexChangedComboContract(const int index);
    void indexChangedComboTypePrice(const int index);
    void indexChangedComboDoctor(const int index);
    void indexChangedComboDoctorExecute(const int index);
    void indexChangedComboNurse(const int index);

    void onClickNewPacient();   // slot-urile pu crearea, editarea
    void onClikEditPacient();   // pacientului
    void onClickClearPacient(); // golirea datelor pacientului + 'comboPacient'
    void onClickOpenHistoryPatient();

    void updateTextSumOrder();                                  // actualizarea sumei documentului
    void onDoubleClickedTableSource(const QModelIndex &index);  // dublu click 'tableSource' - alegerea investigatiilor
    void onClickedRowTableOrder(const QModelIndex &index);      // editarea sumei din tabela 'tableOrder'
    void filterRegExpChanged();                                 // filtrarea 'proxy' pu cautarea rapida a investigatiilor din tabela 'tableSource'
    void filterRegExpChangedPacient();
    void slotContextMenuRequested(QPoint pos);
    void removeRowTableOrder();
    void editRowTableOrder();

    bool controlRequiredObjects();      // controlul completarii obiectelor obligatorii
    void onOpenReport();                // 'btnReport'
    void onPrint(Enums::TYPE_PRINT type_print); // printare
    void openDesignerPrintDoc();
    void openPreviewPrintDoc();
    bool onWritingData();               // 'btnWrite'
    void onWritingDataClose();          // 'btnOk'
    void onClose();                     // 'btnClose'

    void handleCompleterAddressPatients(const QString &text);

private:

    enum Columns {column_Id = 0, column_DeletionMark = 1, column_IdPricings = 2, column_Cod = 3, column_Name = 4, column_Price = 5};
    enum IndexToolBox {box_organization = 0,box_patient = 1,box_commnet = 2};

    void setTitleDoc();                           // setarea titlului documentului
    void initConnections();                       // initierea conectarilor
    void connectionsToIndexChangedCombobox();     // conectarea la modificarea indexului comboboxurlor
    void disconnectionsToIndexChangedCombobox();  // conectarea la modificarea indexului comboboxurlor -> modificarea formei
    void updateModelPacients();                   // actualizarea 'modelPacient' in caz de crearea pacientului nou -> actualizam 'completer'
    void initSetCompleter();                      // initierea 'completer'
    void enableDisableDataPacient(bool mEnabled = true); // apasarea 'btnEdit' si 'checkBox - nou'
    void initFooterDoc();                                // imaginea + textul autorului + btnPrint + btnOk + btnWrite + btnClose

    bool existPatientByNameFName(const QString &_name, const QString &_fName, QString &_birthday);
    bool existPatientByIDNP(const QString &_name, const QString &_fName, QString &_birthday);
    bool splitFullNamePacient(QString &_name, QString &_fName); // divizarea 'comboPacient'
                                                                            // in nme, fName, mName
    int getValuePaymentOrder();
    // solicitarile pu inserarea datelor in tabele 'pacients' si 'orderEcho'
    bool insertDataTablePacients(int &last_id, const QString &_name, const QString &_fName, QString &details_error);
    bool updateDataTablePacients(const QString _name, const QString _fName, QString &details_error);
    bool insertDataTableOrderEcho(QString details_error);
    bool updateDataTableOrderEcho(QString details_error);
    QString existPacientByIDNP() const;

    void updateModelDoctors(); // actualizarea 'modelDoctors' in caz de crearea doctorului nou
    void updateTableSources();
    void updateTableOrder();
    void updateHeaderTableSource();
    void updateHeaderTableOrder();

    QMap<QString, QString> getItemsByTableOrganization();
    QMap<QString, QString> getItemsByTablePacient();

    void setImageForDocPrint();

    QStringList loadDataFromXml(const QString &filePath, const QString &tagName);
    void initCompleterAddressPatients();

private:
    Ui::DocOrderEcho *ui;

    bool m_itNew           = false;
    int m_id               = Enums::IDX_UNKNOW;
    int m_idOrganization   = Enums::IDX_UNKNOW;
    int m_idContract       = Enums::IDX_UNKNOW;
    int m_idTypePrice      = Enums::IDX_UNKNOW;
    int m_idPacient        = Enums::IDX_UNKNOW;
    int m_idNurse          = Enums::IDX_UNKNOW;
    int m_idDoctor         = Enums::IDX_UNKNOW;
    int m_idDoctor_execute = Enums::IDX_UNKNOW;
    int m_idUser           = Enums::IDX_UNKNOW;
    int m_post             = Enums::IDX_UNKNOW;
    int m_attachedImages   = Enums::IDX_UNKNOW;

    int exist_logo      = 0; // variabile pu forma de tipar
    int exist_stamp     = 0;
    int exist_signature = 0;

    QString m_name_patient;

    int lastIdPricings     = Enums::IDX_UNKNOW; // pu determinarea ultimului 'id' docum. 'pricing'
    int sumOrder           = 0;          // suma totala a comenzii
    int noncomercial_price = Enums::IDX_UNKNOW;

    QLabel* labelAuthor;

    PopUp      *popUp;
    QMenu      *menu;
    QMenu      *setUpMenu;
    DataBase   *db;
    QTimer     *timer;  // pu data si ora actuala
    QCompleter *completer;

    //---------------------------------

    QCompleter *city_completer;
    QStringList cityList;

    //---------------------------------

    BaseSqlQueryModel  *modelOrganizations;
    BaseSqlQueryModel  *modelContracts;
    BaseSqlQueryModel  *modelTypesPrices;
    BaseSqlQueryModel  *modelDoctors;
    BaseSqlQueryModel  *modelDoctorsExecute;
    BaseSqlQueryModel  *modelNurses;
    QStandardItemModel *modelPacients;

    BaseSqlTableModel *modelTableSource;
    BaseSqlTableModel *modelTableOrder;
    BaseSortFilterProxyModel *proxy;        // proxy - model pu tabelul source = sortarea tabelei
    BaseSortFilterProxyModel *proxyPacient;

    QSqlQueryModel *print_model_organization;
    QSqlQueryModel *print_model_patient;
    QSqlQueryModel *print_model_table;

    CatGeneral *catDoctors;
    LimeReport::ReportEngine *m_report;

    QStandardItemModel *model_img;
    PatientHistory     *patient_history;

    QStyle *style_fusion = QStyleFactory::create("Fusion");

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // DOCORDERECHO_H
