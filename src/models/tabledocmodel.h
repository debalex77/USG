#ifndef TABLEDOCMODEL_H
#define TABLEDOCMODEL_H

#include <QSqlTableModel>
#include <QBrush>

#include <common/globals.h>
#include <common/table_sections.h>

class TableDocModel : public QSqlTableModel
{
    Q_OBJECT
public:

    enum Purpose {
        Table_source,
        Table_destination,
        Pricing
    };

    explicit TableDocModel(QObject *parent = nullptr);

    void setPurpose(Purpose purpose);
    void setFlags(Qt::ItemFlags flag);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    Qt::ItemFlags m_flags;
    Purpose m_purpose;
};

#endif // TABLEDOCMODEL_H
