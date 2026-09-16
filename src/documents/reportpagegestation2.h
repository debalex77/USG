#ifndef REPORTPAGEGESTATION2_H
#define REPORTPAGEGESTATION2_H

#include <QWidget>
#include <QKeyEvent>
#include <QPushButton>
#include <QTimer>

#include <documents/reportpagebase.h>
#include <data/database.h>
#include <data/fetalreferenceranges.h>
#include <customs/lineeditcustom.h>

namespace Ui {
class ReportPageGestation2;
}

class ReportPageGestation2 : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageGestation2(DataBase &db,
                                  QSqlDatabase &currentDB,
                                  QWidget *parent = nullptr);
    ~ReportPageGestation2();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

    QDate LMP() const;
    QDate probableDateBirth() const;
    bool isGestation2() const;

private slots:
    void onDateLMPChanged();
    void setProbabilDateBirth();

    void updateDescriptionFetusWeight();
    void updateTextDescriptionDoppler();

    void handleSelectTemplate();
    void handleAddTemplate();

    void handleSelectFindingsTemplates();
    void handleAddFindingsTemplates();

    void updateGestationTrimestru(const QDate &dateMenstruation);

private:
    void initRequiredStructure();
    void initInstallEventFilter();
    void setDefaultContext();
    void setPropertyMaxLengthText();
    void initConnections();

    void setGestation2ItemsEnabled(bool trim2);

    QString calculateGestationalAge(const QDate &lmp);
    QDate calculateProbabilDateBirth(const int week, const int day);

    void extractWeeksAndDays(const QString &str_vg, int &weeks, int &days);
    QDate calculateDueDateFromFetalAge(int fetalAgeWeeks, int fetalAgeDays);
    void setDueDateGestation2();

    QString getPercentageByDopplerUmbelicalArtery();
    QString getPercentageByDopplerUterineArteryLeft();
    QString getPercentageByDopplerUterineArteryRight();
    QString getPercentageByDopplerCMA();


    void bindTabGes2Fields(QSqlQuery &q, const int idReport);
    void bindBiometryFields(QSqlQuery &q, const int idReport);
    void bindCraniumFields(QSqlQuery &q, const int idReport);
    void bindSNCFields(QSqlQuery &q, const int idReport);
    void bindHeartFields(QSqlQuery &q, const int idReport);
    void bindThoraxFields(QSqlQuery &q, const int idReport);
    void bindAbdomenFields(QSqlQuery &q, const int idReport);
    void bindUrinarySystemFields(QSqlQuery &q, const int idReport);
    void bindOtherFields(QSqlQuery &q, const int idReport);
    void bindDopplerFields(QSqlQuery &q, const int idReport);


    void insertConclusionTemplate(const QString m_conclusion, const QString m_system);
    void insertFindigsTemplate(const QString m_description, const QString m_typeSystem);

private:
    Ui::ReportPageGestation2 *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;

    FetalReferenceRanges *refRanges;
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

#endif // REPORTPAGEGESTATION2_H
