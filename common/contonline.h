#ifndef CONTONLINE_H
#define CONTONLINE_H

#include <QDialog>
#include <QMessageBox>

#include "common/cryptomanager.h"
#include "common/emailcore.h"
#include <models/basesqlquerymodel.h>
#include <data/database.h>
#include <data/enums.h>
#include <data/globals.h>

#include <data/popup.h>

namespace Ui {
class ContOnline;
}

class ContOnline : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int Id_Organization READ getIdOrganization WRITE setIdOrganization NOTIFY IdChangedOrganization)
public:
    explicit ContOnline(QWidget *parent = nullptr);
    ~ContOnline();

    void setIdOrganization(int IdOrganization)
    {
        m_idOrganization = IdOrganization;
        emit IdChangedOrganization();
    };

    int getIdOrganization() const
    {
        return m_idOrganization;
    };

signals:
    void IdChangedOrganization();

private:
    bool existEmail();
    void setHashUserApp();

private slots:
    void slot_IdChangedOrganization();
    void currentIndexChangedOrganization(int index);

    bool insertDataIntoTableContsOnline();
    bool updateDataIntoTableContsOnline();

    void saveDataClose();
    bool saveData();
    void checkingConnection();

private:
    Ui::ContOnline *ui;

    int m_idOrganization = Enums::IDX::IDX_UNKNOW;

    PopUp *popUp;
    DataBase *db;
    BaseSqlQueryModel *model_organizations;

    EmailCore *email_core;
    CryptoManager *crypto_manager;

    QByteArray m_hashUserApp;
};

#endif // CONTONLINE_H
