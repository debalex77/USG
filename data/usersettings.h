#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QDialog>
#include <QKeyEvent>
#include <QBuffer>
#include <QCompleter>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QTreeWidget>

#include "data/popup.h"
#include "data/database.h"
#include "models/basesqlquerymodel.h"

namespace Ui {
class UserSettings;
}

class UserSettings : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged) // user
    Q_PROPERTY(int Id_Organization READ getIdOrganization WRITE setIdOrganization NOTIFY IdChangedOrganization)
    Q_PROPERTY(int IdDoctor READ getIdDoctor WRITE setIdDoctor NOTIFY IdDoctorChanged)
    Q_PROPERTY(int IdNurse READ getIdNurse WRITE setIdNurse NOTIFY IdNurseChanged)

public:
    explicit UserSettings(QWidget *parent = nullptr);
    ~UserSettings();

    void setId(int Id)
    {m_Id = Id; emit IdChanged();};  // proprietatea - id obiectului
    int getId() const
    {return m_Id;};

    void setIdOrganization(int IdOrganization)
    {m_IdOrganization = IdOrganization; emit IdChangedOrganization();};
    int getIdOrganization() const
    {return m_IdOrganization;};

    void setIdDoctor(int IdDoctor)
    {m_idDoctor = IdDoctor; emit IdDoctorChanged();}
    int getIdDoctor() const
    {return m_idDoctor;}

    void setIdNurse(int IdNurse)
    {m_idNurse = IdNurse; emit IdNurseChanged();}
    int getIdNurse() const
    {return m_idNurse;}

signals:
    void IdChanged();    // p-u conectarea la slot slot_IdChanged()
    void IdChangedOrganization();
    void IdDoctorChanged();
    void IdNurseChanged();

private:

    enum idx
    {
        idx_unknow   = -1,
        idx_write    = 0,
        idx_deletion = 1
    };

    void initSetCompleter();
    void initSetTreeWidget();
    bool controlRequiredObjects();
    QMap<QString, QString> getDataObject();
    bool insertIntoTableConstants();
    bool updateDataTableConstants();
    QString existSettingsForUser();
    void setTableSettingsUsers();

private slots:
    void dataWasModified();
    void slot_IdChanged();
    void slot_IdDoctorChanged();
    void slot_IdNurseChanged();
    void slot_IdChangedOrganization();

    void onTreeWidgetItemDoubleClicked(QTreeWidgetItem * item, int column);

    void activatedItemCompleter(const QModelIndex &index);
    void activatedComboUser(const int index);
    void activatedComboDoctor(const int index);
    void activatedComboNurse(const int index);

    bool loadFile(const QString &fileName);
    void onLinkActivatedForOpenImage(const QString &link);
    void clearImageLogo();

    bool onWritingData();
    void onWritingDataClose();
    void onClose();

private:
    Ui::UserSettings *ui;
    DataBase* db;
    PopUp* popUp;

    BaseSqlQueryModel* modelUser;
    BaseSqlQueryModel* modelDoctor;
    BaseSqlQueryModel* modelNurse;

    QCompleter* completer;
    QStandardItemModel* modelOrganization;

    int m_Id             = idx_unknow;
    int m_IdOrganization = idx_unknow;
    int m_idDoctor       = idx_unknow;
    int m_idNurse        = idx_unknow;

    QStringList list_owners;
    QStringList list_items;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
};

#endif // USERSETTINGS_H
