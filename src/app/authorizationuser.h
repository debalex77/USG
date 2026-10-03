#ifndef AUTHORIZATIONUSER_H
#define AUTHORIZATIONUSER_H

#include <QDeadlineTimer>
#include <QDialog>
#include <QHash>
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

    // Limitarea încercărilor eșuate (în memorie, pentru fiecare login).
    QString attemptsKey() const;
    int remainingLockoutSeconds() const;
    void rejectCredentials();
    void updateLockoutState();

    // Hash SHA-256 vechi -> PBKDF2, doar pe o bază migrată la 4.2.7.
    void upgradeLegacyPasswordHash(QSqlDatabase &database, int userId,
                                   const QString &legacyHash);

    Ui::AuthorizationUser *ui;
    int m_Id = -1;     /* proprietatea - id obiectului */
    DataBase &m_db;

    QStringList err;

    QLineEdit   *edit_password;
    QToolButton *show_hide_password;
    CryptoManager *crypto_manager;

    bool m_loadingData = false;
    QTimer m_lastConnectionTimer;

    QHash<QString, int> m_failedAttempts;
    QHash<QString, QDeadlineTimer> m_lockedUntil;
    QTimer m_lockoutTimer;
    QString m_okText;

protected:
    void changeEvent(QEvent *event) override;       // contolam traducerea aplicatiei
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // AUTHORIZATIONUSER_H
