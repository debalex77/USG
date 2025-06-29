#ifndef AUTHORIZATIONUSER_H
#define AUTHORIZATIONUSER_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QLineEdit>
#include <QToolButton>
#include <QThread>

#include <data/database.h>
#include <threads/databaseprovider.h>
#include <threads/dataconstantsworker.h>

namespace Ui {
class AuthorizationUser;
}

class AuthorizationUser : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)

public:
    explicit AuthorizationUser(QWidget *parent = nullptr);
    ~AuthorizationUser();

    void setId(int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

signals:
    void IdChanged();
    void PwdHashChanged();

private:
    DatabaseProvider *dbProvider();
    void setDataConstants();

private slots:
    void slot_IdChanged();
    void textChangedPasswd();

    void onDataReceived();

    bool onControlAccept();
    void onAccepted();
    void onClose();

private:
    Ui::AuthorizationUser *ui;
    int m_Id = -1;     /* proprietatea - id obiectului */
    DataBase *db;

    QStringList err;

    QLineEdit   *edit_password;
    QToolButton *show_hide_password;
    CryptoManager *crypto_manager;

    DatabaseProvider m_dbProvider;

protected:
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);
};

#endif // AUTHORIZATIONUSER_H
