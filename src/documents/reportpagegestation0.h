#ifndef REPORTPAGEGESTATION__H
#define REPORTPAGEGESTATION__H

#include <QWidget>
#include <QKeyEvent>
#include <QPushButton>
#include <QTimer>

#include <documents/reportpagebase.h>
#include <data/database.h>
#include <customs/lineeditcustom.h>

namespace Ui {
class ReportPageGestation0;
}

class ReportPageGestation0 : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageGestation0(DataBase &db,
                                  QSqlDatabase &currentDB,
                                  QWidget *parent = nullptr);
    ~ReportPageGestation0();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

    QDate LMP() const;
    QDate probableDateBirth() const;
    ReportSections::ViewExamination getViewExamination() const;

private slots:
    void onDateLMPChanged();

    void handleSelectTemplate();
    void handleAddTemplate();

    void handleSelectFindingsTemplates();
    void handleAddFindingsTemplates();

private:
    void initRequiredStructure();
    void initInstallEventFilter();
    void setDefaultContext();
    void setPropertyMaxLengthText();
    void initConnections();

    QString calculateGestationalAge(const QDate &lmp);
    QDate calculateDueDate(const QDate &lmp);

    bool existDocument(const int idReport);

    void bindFields(QSqlQuery &q, const int idReport);
    bool insertData(const int idReport);
    bool updateData(const int idReport);

    void insertConclusionTemplate(const QString m_conclusion, const QString m_system);
    void insertFindigsTemplate(const QString m_description, const QString m_typeSystem);


private:
    Ui::ReportPageGestation0 *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;
    QString toolButtonStyleForText;

    //---------------------------------------------------
    /** structura pu conectare cu butone de 'adaugare' si 'selectare'
     ** a sabloanelor de descrierea a formatiunilor
     ** - QAction
     ** - LineEditCustom */
    struct FindingsTemplatesActions {
        QAction        *action_select = nullptr;
        QAction        *action_add    = nullptr;
        LineEditCustom *item_edit     = nullptr;
        QString         name_system;
    };
    std::vector<FindingsTemplatesActions> rows_action_findings;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
};

#endif // REPORTPAGEGESTATION__H
