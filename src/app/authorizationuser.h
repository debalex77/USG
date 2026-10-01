#ifndef AUTHORIZATIONUSER_H
#define AUTHORIZATIONUSER_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QLineEdit>
#include <QShowEvent>
#include <QToolButton>
#include <QThread>
#include <QTimer>

#include <common/table_sections.h>
#include <database/database.h>
#include <infrastructure/database/databaseprovider.h>
#include <infrastructure/database/dataconstantsworker.h>

namespace Ui {
class AuthorizationUser;
}

class AuthorizationUser : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)

public:
    explicit AuthorizationUser(DataBase &db,
                               QWidget *parent = nullptr);
    ~AuthorizationUser();

    void setId(int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

signals:
    void IdChanged();
    void PwdHashChanged();

public slots:
    void reject() override;

private:
    void setDataConstants();

private slots:
    void slot_IdChanged();
    void updateLastConnectionForLogin();
    void textChangedPasswd();

    void onDataReceived(bool success);

    bool onControlAccept();
    void onAccepted();
    void onClose();

private:
    void refreshLastConnection(int userId = 0);

    Ui::AuthorizationUser *ui;
    int m_Id = -1;     /* proprietatea - id obiectului */
    DataBase &m_db;

    QStringList err;

    QLineEdit   *edit_password;
    QToolButton *show_hide_password;
    CryptoManager *crypto_manager;

    bool m_loadingData = false;
    QTimer m_lastConnectionTimer;

protected:
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
};

#endif // AUTHORIZATIONUSER_H
