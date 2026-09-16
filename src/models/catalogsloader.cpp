#include "catalogsloader.h"

CatalogsLoader::CatalogsLoader(DataBase &db,
                               CatalogType::Type typeCatalogs)
    : m_db(db)
    , m_typeCatalog(typeCatalogs)
{
    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        strQry = m_db.getTextSQL(":/sql/queries/doctors_fullName_select.sql");
        break;

    case CatalogType::Type::Nurses:
        strQry = m_db.getTextSQL(":/sql/queries/nurses_fullName_select.sql");
        break;

    case CatalogType::Type::Users:
        strQry = globals().thisSqlite
                     ? m_db.getTextSQL(":/sql/queries/users_listForm_select_sqlite.sql")
                     : m_db.getTextSQL(":/sql/queries/users_listForm_select_mariadb.sql");
        break;

    case CatalogType::Type::Patients:
        strQry = globals().thisMySQL
                     ? m_db.getTextSQL(":/sql/queries/patients_fullName_select_mariadb.sql")
                     : m_db.getTextSQL(":/sql/queries/patients_fullName_select_sqlite.sql");
        break;

    case CatalogType::Type::Organizations:
        strQry = m_db.getTextSQL(":/sql/queries/organizations_select.sql");
        break;

    default:
        break;
    }
}

CatalogsLoader::BatchResult CatalogsLoader::loadNextBatch(int limit, const QVariant &lastName, qint64 lastId)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen())
        return result;

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(false);
    if (!qry.prepare(sql)) {
        qWarning(logWarning())
            << "CatalogsLoader prepare 'loadNextBatch' error:"
            << qry.lastError().text();
        return result;
    }

    qry.bindValue(":lastName", lastName);
    qry.bindValue(":lastId",   lastId);

    return execQuery(qry, limit);
}

CatalogsLoader::BatchResult CatalogsLoader::loadFirstBatch(int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen())
        return result;

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(true);
    if (!qry.prepare(sql)) {
        qWarning(logWarning())
            << "CatalogsLoader prepare 'loadFirstBatch' error:"
            << qry.lastError().text();
        return result;
    }

    return execQuery(qry, limit);
}

void CatalogsLoader::setSort(int column, Qt::SortOrder order)
{
    m_sortColumn = column;
    m_sortOrder = order;
}

QString CatalogsLoader::buildSql(bool firstBatch) const
{
    QString base = strQry.trimmed();
    base.remove(QRegularExpression(";\\s*$"));
    base.remove(QRegularExpression("\\s+ORDER\\s+BY[\\s\\S]*$", QRegularExpression::CaseInsensitiveOption));
    if (globals().thisMySQL)
        base.replace("name ||' '|| fName", "CONCAT(name, ' ', fName)");

    QStringList columns;
    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
    case CatalogType::Type::Nurses:
        columns = {"id", "deletionMark", "FullName", "telephone", "email", "comment", "uuid"};
        break;
    case CatalogType::Type::Users:
        columns = {"id", "deletionMark", "name", "password", "hash", "lastConnection", "uuid"};
        break;
    case CatalogType::Type::Patients:
        columns = {"id", "deletion_mark", "FullName", "birthday", "idnp", "medical_policy", "address", "telephone", "email", "comment", "uuid"};
        break;
    case CatalogType::Type::Organizations:
        columns = {"id", "deletionMark", "name", "IDNP", "address", "telephone", "email", "comment", "id_contracts", "stamp", "uuid"};
        break;
    default:
        return {};
    }
    const int column = m_sortColumn >= 0 && m_sortColumn < columns.size() ? m_sortColumn : 2;
    const QString field = QString("catalog.`%1`").arg(columns.at(column));
    const QString key = column <= 1 || columns.at(column) == "id_contracts"
        ? QString("COALESCE(%1, 0)").arg(field)
        : QString("LOWER(COALESCE(%1, ''))").arg(field);
    QString sql = QString("SELECT catalog.*, %1 AS pagination_key FROM (%2) catalog").arg(key, base);
    if (!firstBatch) {
        sql += QString(" WHERE (%1 %2 :lastName OR (%1 = :lastName AND catalog.id < :lastId))")
                   .arg(key, m_sortOrder == Qt::AscendingOrder ? ">" : "<");
    }
    sql += QString(" ORDER BY %1 %2, catalog.id DESC LIMIT :limitRows")
               .arg(key, m_sortOrder == Qt::AscendingOrder ? "ASC" : "DESC");
    return sql;
}

CatalogsLoader::BatchResult CatalogsLoader::execQuery(QSqlQuery &qry, int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen()) {
        qWarning(logWarning()).noquote() << "CatalogsLoader: database is not open";
        return result;
    }

    qry.bindValue(":limitRows", limit + 1);

    if (!qry.exec()) {
        qWarning(logWarning()).noquote() << "CatalogsLoader exec error:" << qry.lastError().text();
        qWarning(logWarning()).noquote() << "Executed query:" << qry.lastQuery();
        return result;
    }

    result.items.reserve(limit + 1); // evitam realocare repetate ale vectorului

    while (qry.next()) {
        CatalogsCommon item;

        // comune
        item.id           = qry.value(DoctorsSections::Id).toLongLong();
        item.deletionMark = qry.value(DoctorsSections::DeletionMark).toInt();

        if (m_typeCatalog == CatalogType::Type::Doctors ||
            m_typeCatalog == CatalogType::Type::Nurses) {
            item.fullName = qry.value(DoctorsSections::FullName).toString();
            item.phone    = qry.value(DoctorsSections::Telephone).toString();
            item.email    = qry.value(DoctorsSections::Email).toString();
            item.comment  = qry.value(DoctorsSections::Comment).toString();
            item.uuid     = qry.value(DoctorsSections::Uuid).toByteArray();

        } else if (m_typeCatalog == CatalogType::Type::Users) {
            item.fullName          = qry.value(UsersSections::Name).toString();
            item.lastConnection    = qry.value(UsersSections::LastConnection).toDateTime();
            item.txtLastConnection = item.lastConnection.toString("dd.MM.yyyy hh:mm:ss");

        } else if (m_typeCatalog == CatalogType::Type::Patients) {
            item.fullName      = qry.value(PatientsColumns::FullName).toString();
            item.birthday      = qry.value(PatientsColumns::Birthday).toDate();
            item.txtBirthday   = item.birthday.toString("dd.MM.yyyy");
            item.idnp          = qry.value(PatientsColumns::IDNP).toString();
            item.medicalPolicy = qry.value(PatientsColumns::MedicalPolicy).toString();
            item.adress        = qry.value(PatientsColumns::Address).toString();
            item.phone         = qry.value(PatientsColumns::Telephone).toString();
            item.email         = qry.value(PatientsColumns::Email).toString();
            item.comment       = qry.value(PatientsColumns::Comment).toString();
            item.uuid          = qry.value(PatientsColumns::Uuid).toByteArray();

        } else if (m_typeCatalog == CatalogType::Type::Organizations) {
            item.fullName   = qry.value(OrganizationsSections::Name).toString();
            item.idnp       = qry.value(OrganizationsSections::IDNP).toString();
            item.adress     = qry.value(OrganizationsSections::Address).toString();
            item.phone      = qry.value(OrganizationsSections::Telephone).toString();
            item.email      = qry.value(OrganizationsSections::Email).toString();
            item.comment    = qry.value(OrganizationsSections::Comment).toString();
            item.idContract = qry.value(OrganizationsSections::Id_contracts).toInt();
            item.uuid       = qry.value(OrganizationsSections::Uuid).toByteArray();

        }


        if (result.items.size() < limit)
            result.cursor = qry.value("pagination_key");
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
