#ifndef CLOUDSERVERCONFIG_H
#define CLOUDSERVERCONFIG_H

#include <QDialog>
#include <QKeyEvent>

#include "common/cryptomanager.h"
#include <models/basesqlquerymodel.h>
#include <data/database.h>

#include <data/popup.h>

namespace Ui {
class CloudServerConfig;
}

class CloudServerConfig : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int ID_Organization READ get_ID_Organization WRITE set_ID_Organization NOTIFY ID_OrganizationChanged FINAL)
    Q_PROPERTY(int ID_user READ get_ID_user WRITE set_ID_user NOTIFY ID_userChanged FINAL)
public:
    explicit CloudServerConfig(QWidget *parent = nullptr);
    ~CloudServerConfig();

    int get_ID_Organization() const;
    void set_ID_Organization(const int ID_Organization);

    int get_ID_user() const;
    void set_ID_user(const int ID_user);

signals:
    void ID_OrganizationChanged();
    void ID_userChanged();

private:
    void initSetModels();
    void initConnection();
    void connectionLineEdit();
    void disconnectionLineEdit();
    void connectionComboBox();
    void disconnectionComboBox();

    QByteArray getHashUserApp();
    bool existServerConfig();
    bool insertDataIntoTableCloudServer();
    bool updateDataIntoTableCloudServer();

private slots:
    void dataWasModified();

    void slot_ID_OrganizationChanged();
    void slot_ID_userChanged();

    void indexChangedComboOrganization(const int index);
    void indexChangedComboUser(const int index);

    void saveDataClose();
    bool saveData();

private:
    Ui::CloudServerConfig *ui;

    int m_id_user = -1;
    int m_id_organization = -1;

    DataBase *db;
    PopUp *popUp;
    CryptoManager *crypto_manager;
    BaseSqlQueryModel *model_users;
    BaseSqlQueryModel *model_organizations;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
        // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // CLOUDSERVERCONFIG_H
