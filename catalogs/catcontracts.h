#ifndef CATCONTRACTS_H
#define CATCONTRACTS_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolBar>

#include "data/database.h"
#include "models/basesqlquerymodel.h"

namespace Ui {
class CatContracts;
}

class CatContracts : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool itNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(int IdOrganization READ getIdOrganization WRITE setIdOrganization NOTIFY IdChangedOrganization)
    Q_PROPERTY(int IdTypesPrices READ getIdTypesPrices WRITE setIdTypesPrices NOTIFY IdTypesPricesChanged)

public:
    explicit CatContracts(QWidget *parent = nullptr);
    ~CatContracts();

    void setItNew(bool itNew) {m_itNew = itNew;}
    bool getItNew() {return m_itNew;}

    void setId(const int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

    void setIdOrganization(const int IdOrganization) {m_IdOrganization = IdOrganization; emit IdChangedOrganization();}
    int getIdOrganization() const {return m_IdOrganization;}

    void setIdTypesPrices(const int IdTypesPrices) {m_id_types_prices = IdTypesPrices; emit IdTypesPricesChanged();}
    int getIdTypesPrices() const {return m_id_types_prices;}

    QString getNameParentContract();

signals:
    void IdChanged();          // p-u conectarea la slot slot_IdChanged()
    void ItNewChanged();
    void IdChangedOrganization();
    void IdTypesPricesChanged();
    void createNewContract();
    void changedCatContract();

private:
    void initConnections();
    bool controlRequiredObjects();
    QMap<QString, QString> getDataObject();
    bool insertIntoTableContracts();
    bool updateDataTableContracts();

private slots:
    void dataWasModified();

    void slot_IdChanged();
    void slot_ItNewChanged();
    void slot_IdChangedOrganization();

    void changedIndexComboOrganization(const int arg1);
    void currentIndexTypesPricesChanged(const int index);

    bool onWritingData();
    void onWritingDataClose();
    void onClose();

private:
    Ui::CatContracts *ui;
    DataBase          *db;
    BaseSqlQueryModel *model;
    BaseSqlQueryModel *modelTypesPrices;

    bool m_itNew          = false;   /* proprietatea - obiectul nou creat */
    int m_Id              = -1;      /* proprietatea - id obiectului (-1 = nou creat) */
    int m_IdOrganization  = -1;
    int m_id_types_prices = -1;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
};

#endif // CATCONTRACTS_H
