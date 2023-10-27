#ifndef PATIENTHISTORY_H
#define PATIENTHISTORY_H

#include <QCompleter>
#include <QDialog>
#include <QStandardItemModel>
#include <data/database.h>
#include "catalogs/catgeneral.h"
#include "models/basesortfilterproxymodel.h"
#include "models/basesqlquerymodel.h"

namespace Ui {
class PatientHistory;
}

class PatientHistory : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int IdPatient READ getIdPatient WRITE setIdPatient NOTIFY IdPatientChanged)

public:
    explicit PatientHistory(QWidget *parent = nullptr);
    ~PatientHistory();

    void setIdPatient(int IdPatient) {m_id_patient = IdPatient; emit IdPatientChanged();}
    int getIdPatient() const {return m_id_patient;}

signals:
    void IdPatientChanged();

private slots:
    void slot_IdPatientChanged();
    void filterRegExpChangedPacient();
    void activatedItemCompleter(const QModelIndex &index);
    void onClickedTable(const QModelIndex &index);
    void onDoubleClickedTable(const QModelIndex &index);

    void clearDataComboPatient();
    void openCatPatient();

private:
    void initSetCompleter();
    void updateModelPatients();
    void updateTableDoc();
    void loadImagesPatients();

private:
    Ui::PatientHistory *ui;

    DataBase* db;
    QCompleter* completer;
    QStandardItemModel* model_patients;
    BaseSortFilterProxyModel* proxy;
    BaseSqlQueryModel* model_table;
    CatGeneral* cat_patients;

    int m_id_patient = -1;
};

#endif // PATIENTHISTORY_H
