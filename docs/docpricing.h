#ifndef DOCPRICING_H
#define DOCPRICING_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QSqlTableModel>
#include <QStyleFactory>
#include <LimeReport>
                            // Printare:
#include <QTextStream>      // flux textului
#include <QTextDocument>    // pu formarea documentului de text
#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTimer>

#include "data/popup.h"
#include <data/globals.h>
#include <data/enums.h>
#include "data/database.h"
#include "models/basesqlquerymodel.h"
#include "models/basesqltablemodel.h"
#include "models/basesortfilterproxymodel.h"
#include "catalogs/catforsqltablemodel.h"
#include <customs/custommessage.h>
#include <catalogs/catorganizations.h>
#include <catalogs/catcontracts.h>

namespace Ui {
class DocPricing;
}

class DocPricing : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool ItNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(int IdOrganization READ getIdOrganization WRITE setIdOrganization NOTIFY IdOrganizationChanged)
    Q_PROPERTY(int IdContract READ getIdContract WRITE setIdContract NOTIFY IdContractChanged)
    Q_PROPERTY(int IdTypePrice READ getIdTypePrice WRITE setIdTypePrice NOTIFY IdTypePriceChanged)
    Q_PROPERTY(int IdUser READ getIdUser WRITE setIdUser NOTIFY IdUserChanged)
    Q_PROPERTY(int Post READ getPost WRITE setPost NOTIFY PostChanged)

public:
    explicit DocPricing(QWidget *parent = nullptr);
    ~DocPricing();

    void setItNew(bool ItNew)
    {m_itNew = ItNew; emit ItNewChanged();}
    bool getItNew() const
    {return m_itNew;}

    void setId(int Id)
    {m_id = Id; emit IdChanged();}
    int getId() const
    {return m_id;}

    void setIdOrganization(int IdOrganization)
    {m_idOrganization = IdOrganization; emit IdOrganizationChanged();}
    int getIdOrganization() const
    {return m_idOrganization;}

    void setIdContract(int IdContract)
    {m_idContract = IdContract; emit IdContractChanged();}
    int getIdContract() const
    {return m_idContract;}

    void setIdTypePrice(int IdTypePrice)
    {m_idTypePrice = IdTypePrice; emit IdTypePriceChanged();}
    int getIdTypePrice() const
    {return m_idTypePrice;}

    void setIdUser(int IdUser)
    {m_idUser = IdUser; emit IdUserChanged();}
    int getIdUser() const
    {return m_idUser;}

    void setPost(int Post)
    {m_post = Post; emit PostChanged();}
    int getPost() const
    {return m_post;}

    void onPrintDocument(Enums::TYPE_PRINT type_print);

signals:
    void ItNewChanged();
    void IdChanged();
    void IdOrganizationChanged();
    void IdContractChanged();
    void IdTypePriceChanged();
    void IdUserChanged();
    void PostChanged();
    void PostDocument();
    void mCloseThisForm(); // pu eliminare memoriei

private slots:
    void dataWasModified();


    void updateTimer();        // actualizarea datei si orei in regim real

    void onDateTimeChanged();
    void slot_ItNewChanged();
    void slot_IdChanged();
    void slot_IdOrganizationChanged();
    void slot_IdContractChanged();
    void slot_IdTypePriceChanged();
    void slot_IdUserChanged();

    void indexChangedComboOrganization(const int arg1);
    void indexChangedComboContract(const int arg1);
    void indexChangedComboTypePrice(const int arg1);

    void openCatOrganization();
    void openCatContract();

    void addRowTable();              // *** slot-urile de adaugare
    void editRowTable();             // editare si eliminare a randurilor
    void deletionRowTable();         // din tabel
    void completeTableDocPricing();  // *** btn completare din BD -> clasa 'CatForSqlTableModel'
    void filterRegExpChanged();

    void getDataSelectable();         // determinarea si setarea datelor selectate din
                                      // 'CatForSqlTableModel' in rand nou

    void onClickedRowTable(const QModelIndex &index); // la click-ul pe rand intram in regimul de redactare

    void getDataObject();
    void onPrint(Enums::TYPE_PRINT type_print);
    bool controlRequiredObjects();
    bool onWritingData();
    void onWritingDataClose();
    void onClose();

private:

    void setTitleDoc();
    void initBtnToolBar();
    void initFooterDoc();
    void updateTableView();
    void updateHeaderTable();

    QString insertDataTablePricings();
    QString updateDataTablePricings();

    void connectionsToIndexChangedCombobox();
    void disconnectionsToIndexChangedCombobox();

private:
    Ui::DocPricing *ui;
    DataBase *db;
    PopUp    *popUp;

    BaseSqlQueryModel *modelOrganizations;
    BaseSqlQueryModel *modelContracts;
    BaseSqlQueryModel *modelTypesPrices;
    BaseSqlTableModel *modelTable;
    BaseSortFilterProxyModel *proxy;

    QMenu *menu;

    QLabel        *labelAuthor = nullptr;
    QTextDocument *documentPrint;

    bool m_itNew         = false;
    int m_id             = Enums::IDX_UNKNOW;
    int m_idOrganization = Enums::IDX_UNKNOW;
    int m_idContract     = Enums::IDX_UNKNOW;
    int m_idTypePrice    = Enums::IDX_UNKNOW;
    int m_idUser         = Enums::IDX_UNKNOW;
    int m_post           = Enums::IDX_UNKNOW;

    QTimer* timer;
    QMap<QString, QString> itemsData;
    CatForSqlTableModel *catInvestigations;

    LimeReport::ReportEngine *m_report;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // DOCPRICING_H
