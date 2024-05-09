#ifndef DOCAPPOINTMENTSPATIENTS_H
#define DOCAPPOINTMENTSPATIENTS_H

#include <QDialog>
#include <QDebug>
#include <QKeyEvent>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlRecord>

#include "data/database.h"
#include "data/popup.h"
#include "models/registrationtablemodel.h"
#include <delegates/combodelegate.h>
#include "delegates/checkboxdelegate.h"
#include <docs/docorderecho.h>

namespace Ui {
class DocAppointmentsPatients;
}

class DocAppointmentsPatients : public QDialog
{
    Q_OBJECT

public:
    explicit DocAppointmentsPatients(QWidget *parent = nullptr);
    ~DocAppointmentsPatients();

    static const int m_rows = 33;   // de la 08:00 - 16:00 (interval 15 min.)
    static const int m_column = 7;  /* id, data_patient, investigation,
                                       organization, doctor, comment, executat */
signals:
    void mCloseThisForm();

private slots:
    void dataWasModified();
    void onClickBackDay();
    void onClickNextDay();
    void update_model();
    bool removeData_Doc();
    void writeRegistration();
    void onClickPrint();
    void onClickOrderEcho();
    void onClose();
    void GetChangedValueComboDelegate(QWidget* combo);

    void getChangedDataDelegateComplet(const int id, const QString str_data);

private:
    enum sections
    {
        section_id            = 1,
        section_execute       = 2,
        section_data_patient  = 3,
        section_investigation = 4,
        section_organization  = 5,
        section_doctor        = 6,
        section_comment       = 7
    };

    void setStyleSheetButtoms();
    int getIdOrganizationByName(const QString _name) const;
    int getIdDoctorByFullName(const QString _name) const;

private:
    Ui::DocAppointmentsPatients *ui;
    DataBase *db;
    PopUp    *popUp;
    QTime init_time = QTime(8,00,00);

    QMap<int, QString> items_organizations;
    QMap<int, QString> items_doctors;

    RegistrationPatientsModel *model_main;
    ComboDelegate             *delegate_organizations;
    ComboDelegate             *delegate_doctors;
    CheckBoxDelegate          *delegate_execute;

    QStyle *style_fusion = QStyleFactory::create("Fusion");

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // DOCAPPOINTMENTSPATIENTS_H
