#ifndef PATIENTHISTORY_H
#define PATIENTHISTORY_H

#include <QCompleter>
#include <QDialog>
#include <QStandardItemModel>

#include <common/table_sections.h>
#include <data/database.h>
#include <catalogs/catalogdialog.h>
#include "models/basesqlquerymodel.h"

namespace Ui {
class PatientHistory;
}

class PatientHistory : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int IdPatient READ getIdPatient WRITE setIdPatient NOTIFY IdPatientChanged)

public:
    explicit PatientHistory(DataBase &db, QWidget *parent = nullptr);
    ~PatientHistory();

    void setIdPatient(int IdPatient) {m_id_patient = IdPatient; emit IdPatientChanged();}
    int getIdPatient() const {return m_id_patient;}

signals:
    void IdPatientChanged();

private slots:
    void slot_IdPatientChanged();

    void slotPatientTextChanged(const QString &text);
    void updateModelPatientsByText();
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

    DataBase &m_db;
    QSqlDatabase m_currentDB;

    QCompleter         *completerPatients  = nullptr;
    QStandardItemModel *model_patients     = nullptr;
    QTimer             *timerPatientSearch = nullptr;

    BaseSqlQueryModel  *model_table;

    int m_id_patient = -1;
};

#endif // PATIENTHISTORY_H
