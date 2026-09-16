#ifndef REPORTPAGETHYROID_H
#define REPORTPAGETHYROID_H

#include <QWidget>
#include <QKeyEvent>
#include <QPushButton>
#include <QPlainTextEdit>

#include <documents/reportpagebase.h>
#include <data/database.h>

namespace Ui {
class ReportPageThyroid;
}

class ReportPageThyroid : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageThyroid(DataBase &db,
                               QSqlDatabase &currentDB,
                               QWidget *parent = nullptr);
    ~ReportPageThyroid();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

private slots:
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

    bool existDocument(const int idReport);
    void bindFields(QSqlQuery &q, const int idReport);
    bool insertData(const int idReport);
    bool updateData(const int idReport);

    void insertConclusionTemplate(const QString m_conclusion, const QString m_system);
    void insertFindigsTemplate(const QString m_description, const QString m_typeSystem);

private:
    Ui::ReportPageThyroid *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;

    QString toolButtonStyleForText;

    //---------------------------------------------------
    /** structura pu conectare cu butone de 'adaugare' si 'selectare'
     ** a sabloanelor de descrierea a formatiunilor
     ** - QToolButton
     ** - LineEditCustom */
    struct FindingsTemplatesBtn {
        QAbstractButton *btn_select = nullptr;
        QAbstractButton *btn_add    = nullptr;
        QPlainTextEdit  *item_edit  = nullptr;
        QString          name_system;
    };
    std::vector<FindingsTemplatesBtn> rows_btn_findings;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
};

#endif // REPORTPAGETHYROID_H
