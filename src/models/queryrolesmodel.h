#ifndef QUERYROLESMODEL_H
#define QUERYROLESMODEL_H

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

class QueryRolesModel : public QSqlQueryModel
{
    Q_OBJECT
public:

    // Role-urile pentru coloane încep de aici
    enum
    {
        FirstColumnRole = Qt::UserRole + 1
    };

    explicit QueryRolesModel(const QString &strQuery, QObject *parent = nullptr);

    void setQuery(QSqlQuery query);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Numele rolurilor
    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Role după index de coloană
    static int roleForColumn(int columnIndex);
    int roleForColumn(const QString& columnName) const;

    // Index coloană după nume
    int columnIndex(const QString& columnName) const;

    void setEmptyRowEnabled(bool on, const QString& text = QStringLiteral("<<- Selectează ->>"));

    int rowById(const QString& columnName, const QVariant& value) const;

private:
    void rebuildRoles();

private:
    mutable QHash<int, QByteArray> m_roles;
    QHash<QString, int> m_colByName;

    bool m_hasEmptyRow = false;
    QString m_emptyText = nullptr;
};

#endif // QUERYROLESMODEL_H
