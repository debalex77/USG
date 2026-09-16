#ifndef REPORTPAGELYMPHNODES_H
#define REPORTPAGELYMPHNODES_H

#include <QWidget>
#include <documents/reportpagebase.h>
#include <data/database.h>
#include <views/catalogtableeditor.h>

namespace Ui {
class ReportPageLymphNodes;
}

class ReportPageLymphNodes : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageLymphNodes(DataBase &db,
                                  QSqlDatabase &currentDB,
                                  QWidget *parent = nullptr);
    ~ReportPageLymphNodes();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

    QString typeInvestigation() const;

private slots:
    void handleSelectConclusionTemplate();
    void handleAddConclusionTemplate();
    void handleSelectRecommendationTemplate();
    void handleAddRecommendationTemplate();

private:
    void initInstallEventFilter();
    void setDefaultContext();
    void updateInvestigationTabs();
    void setPropertyMaxLengthText();

    bool existDocument(const int idReport);
    void bindFields(QSqlQuery &q, const int idReport);
    bool insertData(const int idReport);
    bool updateData(const int idReport);

    void insertConclusionTemplate(const QString &conclusion, const QString &system);
    void insertRecommendationTemplate(const QString &recommendation, const QString &system);
    bool ensureRecommendationTemplateType();

private:
    Ui::ReportPageLymphNodes *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;

    QString toolButtonStyleForText;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
};

#endif // REPORTPAGELYMPHNODES_H
