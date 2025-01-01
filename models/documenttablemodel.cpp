#include "documenttablemodel.h"

DocumentTableModel::DocumentTableModel(QObject *parent)
    : QAbstractTableModel{parent}, limit(50), offset(0), totalRecords(0)
{
}


int DocumentTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rows.size(); // numărul curent de rânduri încărcate
}

int DocumentTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rows.isEmpty() ? 0 : rows.first().size(); // Numărul de coloane
}

QVariant DocumentTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const auto &row = rows.at(index.row());
    return row.value(index.column());
}

void DocumentTableModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    if (rows.size() >= totalRecords) // Dacă toate înregistrările sunt încărcate, oprește fetch-ul
        return;

    QSqlQuery query;
    query.prepare("SELECT "
                  " orderEcho.id,"
                  " orderEcho.deletionMark,"
                  " orderEcho.attachedImages,"
                  " orderEcho.cardPayment,"
                  " orderEcho.numberDoc,"
                  " orderEcho.dateDoc,"
                  " organizations.id AS idOrganization,"
                  " organizations.name AS organization,"
                  " contracts.id AS idContract,"
                  " contracts.name AS Contract,"
                  " pacients.id AS idPacient,"
                  " pacients.name ||' '|| pacients.fName ||' '|| pacients.mName ||', '|| substr(pacients.birthday, 9, 2) ||'.'|| substr(pacients.birthday, 6, 2) ||'.'|| substr(pacients.birthday, 1, 4) AS searchPacients,"
                  " pacients.name ||' '|| pacients.fName AS pacient,"
                  " pacients.IDNP,"
                  " fullNameDoctors.nameAbbreviated AS doctor,"
                  " users.id AS idUser,"
                  " users.name AS user,"
                  " orderEcho.sum,"
                  " orderEcho.comment FROM orderEcho "
                  "INNER JOIN organizations ON orderEcho.id_organizations = organizations.id "
                  "INNER JOIN contracts ON orderEcho.id_contracts = contracts.id "
                  "INNER JOIN pacients ON orderEcho.id_pacients = pacients.id "
                  "INNER JOIN fullNameDoctors ON orderEcho.id_doctors = fullNameDoctors.id "
                  "INNER JOIN users ON orderEcho.id_users = users.id "
                  "LIMIT :limit OFFSET :offset;");
    query.bindValue(":limit", limit);
    query.bindValue(":offset", offset);
    query.exec();

    while (query.next()) {
        QVector<QVariant> row;
        for (int i = 0; i < columnCount(); ++i)
            row.append(query.value(i));
        rows.append(row);
    }

    offset += limit; // Actualizează offset-ul
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

bool DocumentTableModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return rows.size() < totalRecords; // Permite fetch-ul dacă mai sunt date de încărcat
}

void DocumentTableModel::setTotalRecords(int total)
{
    if (total == 0)
        totalRecords = getTotalRecords();
    else
        totalRecords = total;
}

int DocumentTableModel::getTotalRecords()
{
    QSqlQuery query("SELECT COUNT(*) FROM orderEcho;");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}
