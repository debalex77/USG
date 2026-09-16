#include "orderjournalloader.h"

OrderJournalLoader::OrderJournalLoader(DataBase &db)
    : m_db(db)
{

}

void OrderJournalLoader::setFilter(const JournalFilter &filter)
{
    m_filter = filter;
}

QString OrderJournalLoader::buildSql(bool firstBatch) const
{
    const QString queryPath = globals().thisMySQL
                                  ? ":/sql/queries_doc/order_journal_mariadb.sql"
                                  : ":/sql/queries_doc/order_journal.sql";
    QString sql = m_db.getTextSQL(queryPath);

    sql.replace("%nr_doc%",
                m_filter.nrDoc.trimmed().isEmpty()
                    ? ""
                    : " AND doc.numberDoc LIKE :nrDoc ");

    sql.replace("%id_org%",
                m_filter.idOrganization > 0
                    ? " AND o.id = :idOrganization "
                    : "");

    sql.replace("%id_cont%",
                m_filter.idContract > 0
                    ? " AND c.id = :idContract "
                    : "");

    sql.replace("%id_us%",
                m_filter.idUser > 0
                    ? " AND u.id = :idUser "
                    : "");

    sql.replace("%id_pacient%",
                m_filter.patientId > 0
                    ? " AND p.id = :patientId "
                    : "");

    if (!firstBatch) {
        sql += R"(
            AND (
                doc.dateDoc < :lastDateDocLess
                OR (doc.dateDoc = :lastDateDocEq AND doc.id < :lastId)
            )
        )";
    }

    sql += R"(
        ORDER BY doc.dateDoc DESC, doc.id DESC
        LIMIT :limitRows
    )";

    return sql;
}

void OrderJournalLoader::bindCommonParams(QSqlQuery &qry) const
{
    qry.bindValue(":startDate", m_filter.startDate);
    qry.bindValue(":endDate", m_filter.endDate);

    if (!m_filter.nrDoc.trimmed().isEmpty())
        qry.bindValue(":nrDoc", "%" + m_filter.nrDoc.trimmed() + "%");

    if (m_filter.idOrganization > 0)
        qry.bindValue(":idOrganization", m_filter.idOrganization);

    if (m_filter.idContract > 0)
        qry.bindValue(":idContract", m_filter.idContract);

    if (m_filter.idUser > 0)
        qry.bindValue(":idUser", m_filter.idUser);

    if (m_filter.patientId > 0)
        qry.bindValue(":patientId", m_filter.patientId);
}

OrderJournalLoader::BatchResult OrderJournalLoader::execQuery(QSqlQuery &qry, int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen()) {
        qWarning(logWarning()).noquote() << "JournalLoader: database is not open";
        return result;
    }

    qry.bindValue(":limitRows", limit + 1);

    if (!qry.exec()) {
        qWarning(logWarning()).noquote() << "JournalLoader exec error:" << qry.lastError().text();
        qWarning(logWarning()).noquote() << "Executed query:" << qry.lastQuery();
        return result;
    }

    result.items.reserve(limit + 1); // evitam realocare repetate ale vectorului

    while (qry.next()) {
        OrderJournal::Item item;

        item.id               = qry.value(OrderJournal::Id).toLongLong();
        item.deletionMark     = qry.value(OrderJournal::DeletionMark).toInt();
        item.attachedImages   = qry.value(OrderJournal::AttachedImages).toInt();
        item.cardPayment      = qry.value(OrderJournal::CardPayment).toInt();
        item.numberDoc        = qry.value(OrderJournal::NumberDoc).toString();
        item.dateDoc          = qry.value(OrderJournal::DateDoc).toDateTime();
        item.dateDocText      = item.dateDoc.toString("dd.MM.yyyy hh:mm:ss");

        item.idOrganizations  = qry.value(OrderJournal::Id_Organization).toLongLong();
        item.idContracts      = qry.value(OrderJournal::Id_Contract).toLongLong();
        item.patientId       = qry.value(OrderJournal::PatientId).toLongLong();
        item.idDoctors        = qry.value(OrderJournal::Id_Doctor).toLongLong();
        item.idUsers          = qry.value(OrderJournal::Id_User).toLongLong();

        item.organizationName = qry.value(OrderJournal::OrganizationName).toString();
        item.contractName     = qry.value(OrderJournal::ContractName).toString();
        item.patientName      = qry.value(OrderJournal::PatientFullName).toString();
        item.patientIdnp      = qry.value(OrderJournal::PatientIdnp).toString();
        item.doctorName       = qry.value(OrderJournal::DoctorName).toString();
        item.userName         = qry.value(OrderJournal::UserName).toString();
        item.patientSearch   = qry.value(OrderJournal::PatientSearch).toString();

        item.sum              = qry.value(OrderJournal::Sum).toDouble();
        item.sumText          = QString::number(item.sum, 'f', 2);
        item.comment          = qry.value(OrderJournal::Comment).toString();
        item.uuid             = qry.value(OrderJournal::Uuid).toByteArray();

        result.items.push_back(std::move(item));
    }

    if (result.items.size() > limit) {
        result.hasMore = true;
        result.items.resize(limit);
    } else {
        result.hasMore = false;
    }

    return result;
}

OrderJournalLoader::BatchResult OrderJournalLoader::loadNextBatch(int limit, const QDateTime &lastDateDoc, qint64 lastId)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen())
        return result;

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(false);
    if (!qry.prepare(sql)) {
        qWarning() << "JournalLoader prepare 'loadNextBatch' error:" << qry.lastError().text();
        return result;
    }

    bindCommonParams(qry);

    qry.bindValue(":lastDateDocLess", lastDateDoc);
    qry.bindValue(":lastDateDocEq", lastDateDoc);
    qry.bindValue(":lastId", lastId);

    return execQuery(qry, limit);
}

OrderJournalLoader::BatchResult OrderJournalLoader::loadFirstBatch(int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen())
        return result;

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(true);
    if (!qry.prepare(sql)) {
        qWarning(logWarning()) << "JournalLoader prepare 'loadFirstBatch' error:" << qry.lastError().text();
        return result;
    }

    bindCommonParams(qry);

    return execQuery(qry, limit);
}
