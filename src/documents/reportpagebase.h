#ifndef REPORTPAGEBASE_H
#define REPORTPAGEBASE_H

#include <QSqlDatabase>
#include <QWidget>
#include <common/table_sections.h>

class ReportPageBase : public QWidget
{
    Q_OBJECT
public:
    explicit ReportPageBase(QWidget *parent = nullptr)
        : QWidget(parent)
    {}

    virtual bool loadData(int idReport) = 0;
    // The caller owns the transaction. Return false on any failed write;
    // implementations must not commit individual systems.
    virtual bool saveData(int idReport) = 0;

    virtual QString concluzionText() const = 0;
    virtual ReportSections::ReportSystem system() const = 0;

signals:
    void dataWasModified();
    void concluzionTextChanged();
    void showPopUp(const QString &text);

    void countImagesBtn(const int &imagesLoaded);
    void countVideoBtn(const int &countVideo);
};

#endif // REPORTPAGEBASE_H