#include "loaddatatask.h"

LoadDataTask::LoadDataTask(int limit, int offset, QObject *parent)
    : QObject(parent), limit(limit), offset(offset)
{

}

void LoadDataTask::setQuery(const QString strQry)
{
    m_strQry = strQry;
}

void LoadDataTask::run()
{
    if (m_strQry == nullptr || m_strQry.isEmpty()) {
        qDebug() << "Nu este setat textul solicitarii !!!";
        return;
    }

    // Deschide baza de date pentru acest fir
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "ThreadConnection");
    db.setDatabaseName(globals::sqlitePathBase);

    if (! db.open()) {
        qDebug() << "Eroare la deschiderea bazei de date:" << db.lastError().text();
        return;
    }

    QSqlQuery qry(db);
    qry.prepare(m_strQry);
    qry.bindValue(":limit", limit);
    qry.bindValue(":offset", offset);

    if (! qry.exec()) {
        qDebug() << "Eroare la execuția interogării:" << qry.lastError().text();
        QSqlDatabase::removeDatabase("ThreadConnection");
        return;
    }

    QVector<QVector<QVariant>> rows;
    while (qry.next()) {
        QVector<QVariant> row;
        for (int i = 0; i < qry.record().count(); ++i) {
            row.append(qry.value(i));
        }
        rows.append(row);

        // Setează numărul de coloane (o singură dată, la prima interogare)
        if (columnCountValue == 0) {
            columnCountValue = qry.record().count();
        }
    }

    // Transmite datele înapoi către clasa părinte
    emit dataReady(rows);
    emit columnCountDetermined(columnCountValue);

    // Curăță conexiunea
    //QSqlDatabase::removeDatabase("ThreadConnection");
}
