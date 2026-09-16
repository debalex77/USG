#include "queryrolesmodel.h"

QueryRolesModel::QueryRolesModel(const QString &strQuery, QObject *parent)
    : QSqlQueryModel{parent}
{
    if (strQuery.isEmpty())
        return;

    QSqlQuery query(QSqlDatabase::database());
    if (!query.prepare(strQuery) || !query.exec()) {
        const QString parentContext = parent
                                          ? QStringLiteral("%1/%2")
                                                .arg(parent->metaObject()->className(),
                                                     parent->objectName())
                                          : QStringLiteral("fără părinte");
        qWarning().noquote()
            << tr("Nu s-a reușit executarea interogării pentru %1. Eroarea: %2; SQL: %3")
                   .arg(parentContext, query.lastError().text(), query.lastQuery());
    }
    QSqlQueryModel::setQuery(std::move(query)); // setam solicitarea in model
    rebuildRoles();
}

void QueryRolesModel::setQuery(QSqlQuery query)
{
    QSqlQueryModel::setQuery(std::move(query));
    rebuildRoles();
}

int QueryRolesModel::rowCount(const QModelIndex &parent) const
{
    const int base = QSqlQueryModel::rowCount(parent);
    return base + (m_hasEmptyRow ? 1 : 0);
}

int QueryRolesModel::columnCount(const QModelIndex &parent) const
{
    return QSqlQueryModel::columnCount(parent);
}

QHash<int, QByteArray> QueryRolesModel::roleNames() const
{
    if (!m_roles.isEmpty())
        return m_roles;

    // fallback (dacă nu avem încă record)
    QHash<int, QByteArray> r;
    r[Qt::DisplayRole] = "display";
    r[Qt::EditRole]    = "edit";
    return r;
}

QVariant QueryRolesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    // rândul 0 = "Selectează..."
    if (m_hasEmptyRow && index.row() == 0) {
        if (role == Qt::DisplayRole) {
            // afișăm text doar în coloana care e setată ca display (de obicei "name")
            // dar Qt cere DisplayRole pe modelColumn, deci e suficient:
            return m_emptyText;
        }
        // pentru role-urile de coloane (UserRole+...), întoarcem null/empty
        return {};
    }

    // Pentru restul rândurilor, mapăm la rândul din modelul de bază
    const int baseRow = m_hasEmptyRow ? (index.row() - 1) : index.row();
    if (baseRow < 0 || baseRow >= QSqlQueryModel::rowCount())
        return {};

    // Role standard -> baza
    if (role < FirstColumnRole)
        return QSqlQueryModel::data(this->index(baseRow, index.column()), role);

    // Role pentru coloane -> col = role - FirstColumnRole
    const int col = role - FirstColumnRole;
    if (col < 0 || col >= columnCount())
        return {};

    return QSqlQueryModel::data(this->index(baseRow, col), Qt::DisplayRole);
}

int QueryRolesModel::roleForColumn(int columnIndex)
{
    return FirstColumnRole + columnIndex;
}

int QueryRolesModel::roleForColumn(const QString &columnName) const
{
    auto it = m_colByName.constFind(columnName.toLower());
    if (it == m_colByName.constEnd())
        return -1;
    return roleForColumn(it.value());
}

int QueryRolesModel::columnIndex(const QString &columnName) const
{
    auto it = m_colByName.constFind(columnName.toLower());
    return (it == m_colByName.constEnd()) ? -1 : it.value();
}

void QueryRolesModel::setEmptyRowEnabled(bool on, const QString &text)
{
    m_hasEmptyRow = on;
    if (!text.isEmpty())
        m_emptyText = text;
}

int QueryRolesModel::rowById(const QString &columnName, const QVariant &value) const
{
    int col = columnIndex(columnName);

    for (int r = 0; r < rowCount(); ++r)
    {
        if (data(index(r, col)).toString() == value.toString())
            return r;
    }
    return -1;
}

void QueryRolesModel::rebuildRoles()
{
    m_roles.clear();
    m_colByName.clear();

    m_roles[Qt::DisplayRole] = "display";
    m_roles[Qt::EditRole]    = "edit";

    const QSqlRecord rec = this->record();
    for (int i = 0; i < rec.count(); ++i) {
        const QString name = rec.fieldName(i);
        const int role = roleForColumn(i);

        m_roles[role] = name.toUtf8();
        m_colByName.insert(name.toLower(), i);
    }
}
