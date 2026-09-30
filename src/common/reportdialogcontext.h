#ifndef REPORTDIALOGCONTEXT_H
#define REPORTDIALOGCONTEXT_H

#include <QSqlDatabase>
#include <QString>

struct ReportDocumentContextData
{
    int orderId           = 0;
    // Date relaționale ale comenzii. organizationId este organizația
    // trimițătoare și nu trebuie folosită drept identitate de tipar.
    int organizationId    = 0;
    int orderUserId       = 0;
    int executingDoctorId = 0;
    int nurseId           = 0;

    [[nodiscard]] bool isValid() const
    {
        return orderId > 0 &&
               organizationId > 0;
    }
};

class ReportDialogContext final
{
public:
    [[nodiscard]]
    const ReportDocumentContextData &data() const;

    [[nodiscard]]
    bool loadFromOrder(const QSqlDatabase &database,
                       int orderId,
                       QString *error = nullptr);
    void clear();

private:
    ReportDocumentContextData m_data;
};

#endif // REPORTDIALOGCONTEXT_H
