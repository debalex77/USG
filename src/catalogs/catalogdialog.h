#ifndef CATALOGDIALOG_H
#define CATALOGDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QUuid>
#include <QMap>
#include <QMapIterator>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>

#include <common/table_sections.h>
#include "common/property_macros.h"

#include <customs/custommessage.h>

#include <data/database.h>
#include <data/popup.h>
#include <common/globals.h>

namespace Ui {
class CatalogDialog;
}

/** Clasa pentru urmatoare cataloage:
      - doctors
      - nurses
      - patients
**********************************************/
class CatalogDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CatalogDialog(DataBase &db,
                           CatalogType::Type catalogType,
                           QWidget *parent = nullptr);
    ~CatalogDialog();

    // declaram proprietatile si functiile setter/getter + functia de update
    // DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc)
    // vezi common/property_macros.h
    DECLARE_PROPERTY_UPDATE(bool, isNew, IsNew, isNewChanged, slot_IsNewChanged)
    DECLARE_PROPERTY_UPDATE(int, id, Id, idChanged, slot_IdChanged)
    DECLARE_PROPERTY_UPDATE(QString, fullName, FullName, fullNameChanged, slot_FullNameChanged)
    DECLARE_PROPERTY_UPDATE(StatusObject::Column, statusCatalog, StatusCatalog, statusCatalogChanged, slot_StatusCatalogChanged)

    bool setDeleteMarkCatalog(QString &err);

signals:
    void isNewChanged();
    void idChanged();
    void fullNameChanged();
    void statusCatalogChanged();

    void catalogDialogCreatedReturnID(const int id);
    void catalogDialogCreated();
    void catalogDialogChanged();
    void catalogDialogDeletedMark();

private slots:
    void slot_IsNewChanged();
    void slot_IdChanged();
    void slot_FullNameChanged();
    void slot_StatusCatalogChanged();

    void catalogTypeChanged();
    void controlLengthComment();
    void dataWasModified();

    void clearImageSignature();
    void clearImageStamp();
    bool loadFile(const QString &fileName, const QString &link);
    void onLinkActivatedForOpenImage(const QString &link);

    void fullNameSplit();

    bool onWritingData();
    void onWritingDataClose();

private:
    void connectionModified();

    bool controlRequiredObjects();
    bool insertDataIntoTableByNameTable(const QString name_table);
    bool updateDataIntoTableByNameTable(const QString name_table);
    bool objectExistsInTableByName(const QString name_table);

    bool confirmIfDuplicateExist(const QString &name_table, const QString &type_label, const QString &extra_info);
    bool handleInsert(const QString &name_table, const QString &type_label, const QString &extra_info);
    bool handleUpdate(const QString &name_table, const QString &type_label);
    bool handleDeletionMark(QString &err);

private:
    Ui::CatalogDialog *ui;

    CatalogType::Type m_typeCatalog;
    QString m_full_name = nullptr;

    DataBase &m_db;
    PopUp    *popUp;
    QString styleBtnMessageBox ;

    int firstSpace        = 0;
    QString strName       = nullptr;
    QString strPrenume    = nullptr;
    QString strPatrimonic = nullptr;

    QStringList err; // pu mesaje de eroare

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
    // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // CATALOGDIALOG_H
