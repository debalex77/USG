#ifndef LISTDOCREPORTORDER_H
#define LISTDOCREPORTORDER_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QLineEdit>
#include <QStatusBar>
#include <QScrollBar>

#include "data/popup.h"
#include "data/database.h"
#include <data/enums.h>
#include <data/appsettings.h>
#include <catalogs/catorganizations.h>
#include <catalogs/catcontracts.h>
#include <catalogs/catgeneral.h>
#include "catalogs/customperiod.h"
#include "docs/choicecolumns.h"
#include "docs/docorderecho.h"
#include <docs/docreportecho.h>
#include "models/basesqlquerymodel.h"
#include <models/basesqltablemodel.h>
#include "models/basesortfilterproxymodel.h"
#include <models/paginatedsqlmodel.h>

#include <common/agentsendemail.h>
#include <common/processingaction.h>
#include <common/handlerfunctionthread.h>
#include <common/reportsettingsmanager.h>

#include <threads/databaseprovider.h>

namespace Ui {
class ListDocReportOrder;
}

class ListDocReportOrder : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(TypeDoc typeDoc READ getTypeDoc WRITE setTypeDoc NOTIFY TypeDocChanged)
    Q_PROPERTY(int IdOrganization READ getIdOrganization WRITE setIdOrganization NOTIFY IdOrganizationChanged)
    Q_PROPERTY(int IdContract READ getIdContract WRITE setIdContract NOTIFY IdContractChanged)
    Q_PROPERTY(int IdPacient READ getIdPacient WRITE setIdPacient NOTIFY IdPacientChanged)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(QString NumberDoc READ getNumberDoc WRITE setNumberDoc NOTIFY NumberDocChanged)

public:
    explicit ListDocReportOrder(QWidget *parent = nullptr);
    ~ListDocReportOrder();

    enum TypeDoc{unknowDoc,orderEcho,reportEcho};
    Q_ENUM(TypeDoc)

    void setTypeDoc(TypeDoc typeDoc)
    {m_typeDoc = typeDoc; emit TypeDocChanged();}
    TypeDoc getTypeDoc() const
    {return m_typeDoc;}

    void setIdOrganization(int IdOrganization)
    {m_idOrganization = IdOrganization; emit IdOrganizationChanged();}
    int getIdOrganization() const
    {return m_idOrganization;}

    void setIdContract(int IdContract)
    {m_idContract = IdContract; emit IdContractChanged();}
    int getIdContract() const
    {return m_idContract;}

    void setIdPacient(int IdPacient)
    {m_idPacient = IdPacient; emit IdPacientChanged();}
    int getIdPacient() const
    {return m_idPacient;}

    void setIdUser(int IdUser)
    {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const
    {return m_idUser;}

    void setNumberDoc(QString NumberDoc)
    {m_numberDoc = NumberDoc; emit NumberDocChanged();}
    QString getNumberDoc() const
    {return m_numberDoc;}

signals:
    void TypeDocChanged();
    void IdOrganizationChanged();
    void IdContractChanged();
    void IdPacientChanged();
    void IdUserChanged();
    void NumberDocChanged();
    void updateProgress(const int num_records, const int value);
    void finishedProgress(const QString txt);

private slots:
    void onScroll(int value);

    void updateMainTableByTimer();
    void enableLineEditSearch();
    void onClosePeriodFilter();

    void onStartDateTimeChanged();
    void onEndDateTimeChanged();

    void indexChangedComboOrganization(const int index);   // modificarea indexurilor combobox-urilor
    void indexChangedComboContract(const int index);
    void slot_TypeDocChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();

    void onClickBtnFilterApply();
    void onClickBtnFilterClear();
    void onClickBtnFilterClose();

    void onClickBtnAdd();
    void onClickBtnEdit();
    void onClickBtnDeletion();
    void onClickBtnAddFilter();
    void onClickBtnFilterByIdOrganization();
    void onClickBtnFilterRemove();
    void onClickBtnUpdateTable();
    void onClickBtnHideShowColumn();
    void onClickBtnPrint();
    void onClickBtnSendEmail();
    void onClickBtnReport();
    void onClickBtnShowHideViewTab();
    void openHistoryPatients();
    void filterRegExpChanged();

    void onClickedTable(const QModelIndex &index);
    void onDoubleClickedTable(const QModelIndex &index);
    void slotContextMenuRequested(QPoint pos);
    void onActionEditOrganization();
    void onActionEditContract();
    void onActionEditPacient();

    void showHideColums();

private:

    enum sections
    {
        section_id             = 0,
        section_deletionMark   = 1,
        section_attachedImages = 2,
        section_cardPayment    = 3,
        section_numberDoc      = 4,
        section_dateDoc        = 5,
        section_idOrganization = 6,
        section_Organization   = 7,
        section_idContract     = 8,
        section_Contract       = 9,
        section_idPacient      = 10,
        section_searchPacient  = 11,
        section_pacient        = 12,
        section_IDNP           = 13,
        section_doctor         = 14,
        section_idUser         = 15,
        section_user           = 16,
        section_sum            = 17,
        section_comment        = 18
    };

    enum report_sections
    {
        report_id             = 0,
        report_deletionMark   = 1,
        report_attachedImages = 2,
        report_cardPayment    = 3,
        report_numberDoc      = 4,
        report_dateDoc        = 5,
        report_id_orderEcho   = 6,
        report_id_patient     = 7,
        report_search_patient = 8,
        report_name_patient   = 9,
        report_idnp_patient   = 10,
        report_name_DocOrder  = 11,
        report_id_user        = 12,
        report_name_user      = 13,
        report_concluzion     = 14,
        report_comment        = 15
    };

    QString enumToString(TypeDoc typeDoc);
    void initBtnToolBar();
    void initBtnFilter();
    void updateTextPeriod();

    void openDocOrderEchoByClickBtnTable();
    void openPrintDesignerPreviewOrder(bool preview = true);
    void openPrintDesignerPreviewReport(bool preview = true);
    void formationPrinMenuForOrder();
    void formationPrinMenuForReport();

    void updateTableView();
    void updateTableViewOrderEcho();
    void updateTableViewOrderEchoFull();
    void updateTableViewReportEcho();
    void updateTableViewReportEchoFull();
    void updateHeaderTableOrderEcho();
    void updateHeaderTableReportEcho();

    QString getNameTableForSettings();
    void loadFilterData();
    void loadSizeSectionPeriodTable();
    void saveSizeSectionTable();
    int sizeSectionDefault(const int numberSection);

    DatabaseProvider *dbProvider();

private:
    Ui::ListDocReportOrder *ui;
    ReportSettingsManager settings;

    TypeDoc m_typeDoc    = unknowDoc;
    int m_idOrganization = Enums::IDX::IDX_UNKNOW;
    int m_idContract     = Enums::IDX::IDX_UNKNOW;
    int m_idPacient      = Enums::IDX::IDX_UNKNOW;
    int m_idUser         = Enums::IDX::IDX_UNKNOW;
    int m_currentRow     = Enums::IDX::IDX_UNKNOW;
    QString m_numberDoc;

    static const int sz_id              = 5;    // size section default
    static const int sz_deletionMark    = 5;
    static const int sz_attachedImages  = 5;
    static const int sz_cardPayment     = 5;
    static const int sz_numberDoc       = 60;
    static const int sz_dateDoc         = 150;
    static const int sz_id_organization = 5;
    static const int sz_organization    = 200;
    static const int sz_id_contract     = 5;
    static const int sz_contract        = 100;
    static const int sz_id_pacient      = 5;
    static const int sz_search_pacient  = 400;
    static const int sz_pacient         = 230;
    static const int sz_idnp            = 130;
    static const int sz_doctor          = 80;
    static const int sz_id_user         = 5;
    static const int sz_user            = 60;
    static const int sz_sum             = 50;

    static const int section_zero = 0;  // pu salvarea perioadei

    const QSet<QString> code_organs_internal =
        {
            "1050.10.", "1050.11.", "1050.14.", "1050.15.", "1050.17.", "1050.18.", "1050.60.", "1050.61.",
            "1050.62.", "1050.63."
        };

    const QSet<QString> code_urynari_system =
        {
            "1050.19.", "1050.20.", "1050.21.", "1050.29.", "1050.30.", "1050.56.", "1050.61.", "1050.62.",
            "1050.63."
        };

    const QSet<QString> code_prostate =
        {
            "1038.", "1039.", "1050.37.", "1050.38.", "1050.39.", "1050.55."
        };

    const QSet<QString> code_gynecologie =
        {
            "1050.22.", "1050.23.", "1050.24.", "1050.25.", "1050.26.", "1050.27.", "1049.2.", "1050.68.",
            "1050.69.", "1050.63.", "1050.62."
        };

    const QSet<QString> code_breast =
        {
            "1050.34.", "1050.35.", "1050.36."
        };

    const QSet<QString> code_thyroide =
        {
            "1050.31.", "1050.32.", "1050.33."
        };

    const QSet<QString> code_gestation0 =
        {
            "1050.66.", "1050.67."
        };

    const QSet<QString> code_gestation1 =
        {
            "1027.4.1.", "1027.5.1.", "1028.4.1.", "1028.5.1."
        };

    const QSet<QString> code_gestation2 =
        {
            "1029.1.1.", "1029.1.2.", "1029.2.", "1029.3.", "1029.4.", "1029.31.",
        };

    DataBase *db;
    DatabaseProvider m_dbProvider;

    PopUp    *popUp;
    QMenu    *menu;
    QMenu    *setUpMenu_order;
    QMenu    *setUpMenu_report;

    BaseSqlQueryModel *modelOrganizations;
    BaseSqlQueryModel *modelContracts;
    PaginatedSqlModel *modelTable;//BaseSqlQueryModel *modelTable;
    BaseSqlQueryModel *model_view_table_order;
    BaseSqlQueryModel *model_view_table_report;
    BaseSortFilterProxyModel *proxyTable;

    int pressed_btn_viewTab = Enums::IDX::IDX_UNKNOW; // pu determinarea apasarii btn

    DocOrderEcho   *docOrderEcho;
    CustomPeriod   *customPeriod;
    PatientHistory *patient_history;
    AgentSendEmail *agent_sendEmail;
    ProcessingAction *loader;

    QTimer *timer;

    bool loadDocumentsFull = false; // pu incarcarea datelor documetelor de la initierea programei se seteaza la enableLineEditSearch()

    ChoiceColumns *columns;

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void keyReleaseEvent(QKeyEvent *event); // pu identificare butoanelor:
                                            // 'Key_Up', 'Key_Down', 'Key_Home', 'Key_End', 'Key_Prt'
                                            // se foloseste functia - keyReleaseEvent(), analogica keyPressEvent()
};

#endif // LISTDOCREPORTORDER_H
