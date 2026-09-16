#include "reportjournalloader.h"

ReportJournalLoader::ReportJournalLoader(DataBase &db) : m_db(db) {}

void ReportJournalLoader::setFilter(const JournalFilter &filter) { m_filter = filter; }

QString ReportJournalLoader::buildSql(bool firstBatch) const
{
    QString sql = m_db.getTextSQL(globals().thisMySQL
        ? ":/sql/queries_doc/report_journal_mariadb.sql"
        : ":/sql/queries_doc/report_journal.sql");
    sql.replace("%nr_doc%", m_filter.nrDoc.trimmed().isEmpty() ? "" : " AND r.numberDoc LIKE :nrDoc ");
    sql.replace("%id_org%", m_filter.idOrganization > 0 ? " AND o.id_organizations = :idOrganization " : "");
    sql.replace("%id_cont%", m_filter.idContract > 0 ? " AND o.id_contracts = :idContract " : "");
    sql.replace("%id_us%", m_filter.idUser > 0 ? " AND r.id_users = :idUser " : "");
    sql.replace("%id_pacient%", m_filter.patientId > 0 ? " AND r.patient_id = :patientId " : "");
    sql.replace("%patient_name%", m_filter.patientName.trimmed().isEmpty()
                                     ? ""
                                     : globals().thisMySQL
                                           ? " AND CONCAT_WS(' ', p.last_name, p.first_name, p.middle_name) LIKE :patientName "
                                           : " AND TRIM(p.last_name || ' ' || COALESCE(p.first_name, '') || ' ' || COALESCE(p.middle_name, '')) LIKE :patientName ");
    if (!firstBatch)
        sql += " AND (r.dateDoc < :lastDateDocLess OR (r.dateDoc = :lastDateDocEq AND r.id < :lastId)) ";
    sql += " ORDER BY r.dateDoc DESC, r.id DESC LIMIT :limitRows ";
    return sql;
}

void ReportJournalLoader::bindCommonParams(QSqlQuery &query) const
{
    query.bindValue(":startDate", m_filter.startDate);
    query.bindValue(":endDate", m_filter.endDate);
    if (!m_filter.nrDoc.trimmed().isEmpty()) query.bindValue(":nrDoc", "%" + m_filter.nrDoc.trimmed() + "%");
    if (m_filter.idOrganization > 0) query.bindValue(":idOrganization", m_filter.idOrganization);
    if (m_filter.idContract > 0) query.bindValue(":idContract", m_filter.idContract);
    if (m_filter.idUser > 0) query.bindValue(":idUser", m_filter.idUser);
    if (m_filter.patientId > 0) query.bindValue(":patientId", m_filter.patientId);
    if (!m_filter.patientName.trimmed().isEmpty())
        query.bindValue(":patientName", "%" + m_filter.patientName.trimmed() + "%");
}

ReportJournalLoader::BatchResult ReportJournalLoader::execQuery(QSqlQuery &query, int limit) const
{
    BatchResult result;
    query.bindValue(":limitRows", limit + 1);
    if (!query.exec()) {
        result.error = query.lastError().text();
        qWarning(logWarning()).noquote() << "ReportJournalLoader exec error:" << query.lastError().text();
        qWarning(logWarning()).noquote() << "Executed query:" << query.lastQuery();
        return result;
    }
    result.items.reserve(limit + 1);
    while (query.next()) {
        ReportJournal::Item item;
        item.id = query.value(ReportJournal::Id).toLongLong();
        item.deletionMark = query.value(ReportJournal::DeletionMark).toInt();
        item.attachedImages = query.value(ReportJournal::AttachedImages).toInt();
        item.numberDoc = query.value(ReportJournal::NumberDoc).toString();
        item.dateDoc = query.value(ReportJournal::DateDoc).toDateTime();
        item.dateDocText = item.dateDoc.toString("dd.MM.yyyy hh:mm:ss");
        item.orderId = query.value(ReportJournal::OrderId).toLongLong();
        item.patientId = query.value(ReportJournal::PatientId).toLongLong();
        item.userId = query.value(ReportJournal::UserId).toLongLong();
        item.patientName = query.value(ReportJournal::PatientFullName).toString();
        item.patientIdnp = query.value(ReportJournal::PatientIdnp).toString();
        item.orderDescription = query.value(ReportJournal::OrderDescription).toString();
        item.userName = query.value(ReportJournal::UserName).toString();
        item.conclusion = query.value(ReportJournal::Conclusion).toString();
        item.comment = query.value(ReportJournal::Comment).toString();
        item.patientSearch = query.value(ReportJournal::PatientSearch).toString();
        item.uuid = query.value(ReportJournal::Uuid).toByteArray();
        result.items.push_back(std::move(item));
    }
    result.hasMore = result.items.size() > limit;
    if (result.hasMore) result.items.resize(limit);
    return result;
}

ReportJournalLoader::BatchResult ReportJournalLoader::loadFirstBatch(int limit)
{
    if (!m_db.getDatabase().isOpen())
        return {{}, false, QObject::tr("Conexiunea cu baza de date nu este deschisă.")};
    QSqlQuery query(m_db.getDatabase());
    query.setForwardOnly(true);
    if (!query.prepare(buildSql(true))) {
        qWarning(logWarning()) << "ReportJournalLoader prepare loadFirstBatch error:" << query.lastError().text();
        return {{}, false, query.lastError().text()};
    }
    bindCommonParams(query);
    return execQuery(query, limit);
}

ReportJournalLoader::BatchResult ReportJournalLoader::loadNextBatch(int limit, const QDateTime &date, qint64 id)
{
    if (!m_db.getDatabase().isOpen())
        return {{}, false, QObject::tr("Conexiunea cu baza de date nu este deschisă.")};
    QSqlQuery query(m_db.getDatabase());
    query.setForwardOnly(true);
    if (!query.prepare(buildSql(false))) {
        qWarning(logWarning()) << "ReportJournalLoader prepare loadNextBatch error:" << query.lastError().text();
        return {{}, false, query.lastError().text()};
    }
    bindCommonParams(query);
    query.bindValue(":lastDateDocLess", date);
    query.bindValue(":lastDateDocEq", date);
    query.bindValue(":lastId", id);
    return execQuery(query, limit);
}
