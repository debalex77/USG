/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "catalogsloader.h"


CatalogsLoader::CatalogsLoader(DataBase &db,
                               CatalogType::Type typeCatalogs)
    : m_db(db)
    , m_typeCatalog(typeCatalogs)
{
    switch (m_typeCatalog) {
    case CatalogType::Type::Doctors:
        strQry = MainDatabaseConnectionContext::instance().isMariaDb()
                     ? m_db.getTextSQL(":/sql/queries/doctors_fullName_select_mariadb.sql")
                     : m_db.getTextSQL(":/sql/queries/doctors_fullName_select.sql");
        break;

    case CatalogType::Type::Nurses:
        strQry = MainDatabaseConnectionContext::instance().isMariaDb()
                     ? m_db.getTextSQL(":/sql/queries/nurses_fullName_select_mariadb.sql")
                     : m_db.getTextSQL(":/sql/queries/nurses_fullName_select.sql");
        break;

    case CatalogType::Type::Users:
        strQry = MainDatabaseConnectionContext::instance().isSqlite()
                     ? m_db.getTextSQL(":/sql/queries/users_listForm_select_sqlite.sql")
                     : m_db.getTextSQL(":/sql/queries/users_listForm_select_mariadb.sql");
        break;

    case CatalogType::Type::Patients:
        strQry = MainDatabaseConnectionContext::instance().isMariaDb()
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

    if (!m_db.getDatabase().isOpen()) {
        result.error = QStringLiteral("database is not open");
        return result;
    }

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(false);
    if (!qry.prepare(sql)) {
        qWarning(logWarning())
            << "CatalogsLoader prepare 'loadNextBatch' error:"
            << qry.lastError().text();
        result.error = qry.lastError().text();
        return result;
    }

    qry.bindValue(":lastName", lastName);
    qry.bindValue(":lastId",   lastId);
    bindSearch(qry);

    return execQuery(qry, limit);
}

CatalogsLoader::BatchResult CatalogsLoader::loadFirstBatch(int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen()) {
        result.error = QStringLiteral("database is not open");
        return result;
    }

    QSqlQuery qry(m_db.getDatabase());
    qry.setForwardOnly(true);

    const QString sql = buildSql(true);
    if (!qry.prepare(sql)) {
        qWarning(logWarning())
            << "CatalogsLoader prepare 'loadFirstBatch' error:"
            << qry.lastError().text();
        result.error = qry.lastError().text();
        return result;
    }
    bindSearch(qry);

    return execQuery(qry, limit);
}

void CatalogsLoader::setSort(int column, Qt::SortOrder order)
{
    m_sortColumn = column;
    m_sortOrder = order;
}

void CatalogsLoader::setSearchText(const QString &text)
{
    // Căutarea este disponibilă doar în catalogul pacienților.
    constexpr int maxSearchWords = 5;

    m_searchWords.clear();
    if (m_typeCatalog != CatalogType::Type::Patients)
        return;

    m_searchWords = text.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (m_searchWords.size() > maxSearchWords)
        m_searchWords.resize(maxSearchWords);
}

QString CatalogsLoader::searchCondition() const
{
    if (m_searchWords.isEmpty())
        return {};

    // Fiecare cuvânt trebuie găsit în nume/prenume, IDNP sau data nașterii
    // (zz.ll.aaaa); astfel „Popescu Ion” și „Ion Popescu” dau același rezultat.
    const QString birthday = MainDatabaseConnectionContext::instance().isMariaDb()
        ? QStringLiteral("DATE_FORMAT(catalog.`birthday`, '%d.%m.%Y')")
        : QStringLiteral("strftime('%d.%m.%Y', catalog.`birthday`)");

    QStringList parts;
    for (int i = 0; i < m_searchWords.size(); ++i) {
        const QString index = QString::number(i);
        parts.append(QStringLiteral("(catalog.`FullName` LIKE :searchName") + index
                     + QStringLiteral(" ESCAPE '!' OR catalog.`idnp` LIKE :searchIdnp") + index
                     + QStringLiteral(" ESCAPE '!' OR ") + birthday
                     + QStringLiteral(" LIKE :searchBirthday") + index
                     + QStringLiteral(" ESCAPE '!')"));
    }
    return QStringLiteral("(") + parts.join(QStringLiteral(" AND ")) + QStringLiteral(")");
}

void CatalogsLoader::bindSearch(QSqlQuery &qry) const
{
    for (int i = 0; i < m_searchWords.size(); ++i) {
        // '%', '_' și '!' din textul introdus se caută literal (ESCAPE '!').
        QString word = m_searchWords.at(i);
        word.replace(QLatin1Char('!'), QStringLiteral("!!"))
            .replace(QLatin1Char('%'), QStringLiteral("!%"))
            .replace(QLatin1Char('_'), QStringLiteral("!_"));
        const QString pattern = QLatin1Char('%') + word + QLatin1Char('%');
        const QString index = QString::number(i);
        qry.bindValue(QStringLiteral(":searchName") + index, pattern);
        qry.bindValue(QStringLiteral(":searchIdnp") + index, pattern);
        qry.bindValue(QStringLiteral(":searchBirthday") + index, pattern);
    }
}

QString CatalogsLoader::buildSql(bool firstBatch) const
{
    QString base = strQry.trimmed();
    base.remove(QRegularExpression(";\\s*$"));
    base.remove(QRegularExpression("\\s+ORDER\\s+BY[\\s\\S]*$", QRegularExpression::CaseInsensitiveOption));

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
    // id: fără funcții, ca SQL-ul să poată folosi cheia primară.
    const QString key = column == 0
        ? field
        : column == 1 || columns.at(column) == "id_contracts"
        ? QString("COALESCE(%1, 0)").arg(field)
        : QString("LOWER(COALESCE(%1, ''))").arg(field);
    QString sql = QString("SELECT catalog.*, %1 AS pagination_key FROM (%2) catalog").arg(key, base);

    QStringList conditions;
    const QString search = searchCondition();
    if (!search.isEmpty())
        conditions.append(search);
    if (!firstBatch) {
        conditions.append(QString("(%1 %2 :lastName OR (%1 = :lastName AND catalog.id < :lastId))")
                              .arg(key, m_sortOrder == Qt::AscendingOrder ? ">" : "<"));
    }
    if (!conditions.isEmpty())
        sql += QStringLiteral(" WHERE ") + conditions.join(QStringLiteral(" AND "));
    sql += QString(" ORDER BY %1 %2, catalog.id DESC LIMIT :limitRows")
               .arg(key, m_sortOrder == Qt::AscendingOrder ? "ASC" : "DESC");
    return sql;
}

CatalogsLoader::BatchResult CatalogsLoader::execQuery(QSqlQuery &qry, int limit)
{
    BatchResult result;

    if (!m_db.getDatabase().isOpen()) {
        qWarning(logWarning()).noquote() << "CatalogsLoader: database is not open";
        result.error = QStringLiteral("database is not open");
        return result;
    }

    qry.bindValue(":limitRows", limit + 1);

    if (!qry.exec()) {
        qWarning(logWarning()).noquote() << "CatalogsLoader exec error:" << qry.lastError().text();
        qWarning(logWarning()).noquote() << "Executed query:" << qry.lastQuery();
        result.error = qry.lastError().text();
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
