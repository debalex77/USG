#ifndef CATUSERS_H
#define CATUSERS_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include <QMap>
#include <QMapIterator>
#include <QCryptographicHash>
#include <QLineEdit>
#include <QToolButton>

#include <customs/custommessage.h>
#include <data/appsettings.h>
#include "data/database.h"
#include "data/popup.h"

namespace Ui {
class CatUsers;
}

class CatUsers : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool itNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(QString NameUser READ getNameUser WRITE setNameUser NOTIFY NameUserChanged)

public:
    explicit CatUsers(QWidget *parent = nullptr);
    ~CatUsers();

    void setItNew(bool itNew) {m_itNew = itNew; emit ItNewChanged();}
    bool getItNew() {return m_itNew;}

    void setId(const int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

    void setNameUser(const QString NameUser) {m_name_user = NameUser; emit NameUserChanged();}
    QString getNameUser() const {return m_name_user;}

signals:
    void IdChanged();      // p-u conectarea la slot changedIdObject()
    void ItNewChanged();
    void NameUserChanged();
    void mCreateNewUser();
    void mChangedDataUser();

private slots:
    void dataWasModified();

    void slot_ItNewChanged();
    void textChangedPasswd();
    void changedIdObject();

    bool onWritingData();
    void onWritingDataClose();

private:
    QVariantMap getDataObject();
    bool handleInsert();
    bool handleUpdate();

private:
    Ui::CatUsers *ui;

    bool m_itNew;      /* proprietatea - obiectul nou creat */
    int m_Id = -1;     /* proprietatea - id obiectului (-1 = nou creat) */
    QString m_name_user = nullptr;

    DataBase *db;
    PopUp    *popUp;

    QStringList err;

    QLineEdit   *edit_password;
    QToolButton *show_hide_password;

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // CATUSERS_H
