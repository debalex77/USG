#ifndef CATGENERAL_H
#define CATGENERAL_H

#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include <QMap>
#include <QMapIterator>
#include <QCryptographicHash>

#include <data/database.h>
#include <data/popup.h>
#include <data/globals.h>

namespace Ui {
class CatGeneral;
}

class CatGeneral : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool itNew READ getItNew WRITE setItNew NOTIFY ItNewChanged)
    Q_PROPERTY(int Id READ getId WRITE setId NOTIFY IdChanged)
    Q_PROPERTY(QString FullName READ getFullName WRITE setFullName NOTIFY FullNameChanged)
    Q_PROPERTY(TypeCatalog typeCatalog READ getTypeCatalog WRITE setTypeCatalog NOTIFY typeCatalogChanged)

public:
    explicit CatGeneral(QWidget *parent = nullptr);
    ~CatGeneral();

    enum TypeCatalog{Doctors, Nurses, Pacients};
    Q_ENUM(TypeCatalog)

    void setItNew(bool itNew) {m_itNew = itNew; emit ItNewChanged();}
    bool getItNew() {return m_itNew;}

    void setId(int Id) {m_Id = Id; emit IdChanged();}
    int getId() const {return m_Id;}

    void setFullName(QString FullName) {m_full_name = FullName; emit FullNameChanged();}
    QString getFullName() const {return m_full_name;}

    void setTypeCatalog(TypeCatalog typeCatalog) {m_typeCatalog = typeCatalog; emit typeCatalogChanged();}
    TypeCatalog getTypeCatalog() {return m_typeCatalog;}

private:
    bool controlRequiredObjects();
    bool insertDataIntoTableByNameTable(const QString name_table);
    bool updateDataIntoTableByNameTable(const QString name_table);
    bool objectExistsInTableByName(const QString name_table);
    void loadImageOpeningCatalog();

signals:
    void IdChanged();          // p-u conectarea la slot slot_IdChanged()
    void ItNewChanged();
    void FullNameChanged();
    void typeCatalogChanged();
    void createCatGeneral();
    void changedCatGeneral();

private slots:
    void controlLengthComment();
    void dataWasModified();

    void clearImageSignature();
    void clearImageStamp();
    bool loadFile(const QString &fileName, const QString &link);
    void onLinkActivatedForOpenImage(const QString &link);

    void slot_typeCatalogChanged();
    void slot_IdChanged();
    void slot_ItNewChanged();

    void fullNameChanged();
    void fullNameSplit();

    bool onWritingData();
    void onWritingDataClose();

private:
    Ui::CatGeneral *ui;

    bool m_itNew;              /* proprietatea - obiectul nou creat */
    int m_Id = -1;             /* proprietatea - id obiectului (-1 = nou creat) */
    TypeCatalog m_typeCatalog; /* proprietatea - tipul catalogului */
    QString m_full_name = nullptr;

    DataBase* db;
    PopUp* popUp;

    int firstSpace = 0;
    QString strName;
    QString strPrenume;
    QString strPatrimonic;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    void changeEvent(QEvent *event);       // contolam traducerea aplicatiei
    void keyPressEvent(QKeyEvent *event);  // procedura de bypass a elementelor
                                           // (Qt::Key_Return | Qt::Key_Enter)
};

#endif // CATGENERAL_H
