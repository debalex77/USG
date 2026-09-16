#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include <QDialog>

#include <data/database.h>

class CheckBoxDelegate;
class ComboDelegate;
class PatientAppointmentDelegate;
class MultiInvestigationDelegate;
class PopUp;
class RegistrationPatientsModel;

namespace Ui { class AppointmentDialog; }

class AppointmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppointmentDialog(DataBase &db, QWidget *parent = nullptr);
    ~AppointmentDialog() override;

private slots:
    void dataWasModified();
    void showPreviousDay();
    void showNextDay();
    void reload();
    void removeAppointments();
    void removeCurrentAppointment();
    void saveAppointments();
    void printAppointments();
    void createOrder();

private:
    enum Section { Id, Executed, Patient, Investigation, Organization,
                   Doctor, Comment, SectionCount };
    static constexpr int RowCount = 33;

    void setupDelegates();
    void setupConnections();
    void setupTable();
    void setupStyle();
    bool saveChanges();
    int currentRow() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::AppointmentDialog *ui;
    DataBase &m_db;
    PopUp *m_popup = nullptr;
    RegistrationPatientsModel *m_model = nullptr;
    ComboDelegate *m_organizationDelegate = nullptr;
    ComboDelegate *m_doctorDelegate = nullptr;
    MultiInvestigationDelegate *m_investigationDelegate = nullptr;
    PatientAppointmentDelegate *m_patientDelegate = nullptr;
    CheckBoxDelegate *m_executedDelegate = nullptr;
    bool m_loading = false;
};

#endif // APPOINTMENTDIALOG_H
