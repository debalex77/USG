#include "database_common.h"

#include <QRegularExpression>

namespace {

QStringList splitSqlScript(const QString &sql)
{
    QStringList statements;
    QString statement;
    bool compoundStatement = false;

    const QStringList lines = sql.split('\n');
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QStringLiteral("--")))
            continue;

        statement += line;
        statement += QLatin1Char('\n');

        if (trimmed.compare(QStringLiteral("BEGIN"), Qt::CaseInsensitive) == 0) {
            compoundStatement = true;
            continue;
        }

        const bool compoundEnd = compoundStatement
                                 && trimmed.compare(QStringLiteral("END;"),
                                                    Qt::CaseInsensitive) == 0;
        const bool simpleEnd = !compoundStatement && trimmed.endsWith(QLatin1Char(';'));
        if (!compoundEnd && !simpleEnd)
            continue;

        statement = statement.trimmed();
        if (!statement.isEmpty())
            statements.append(statement);
        statement.clear();
        compoundStatement = false;
    }

    statement = statement.trimmed();
    if (!statement.isEmpty())
        statements.append(statement);

    return statements;
}

bool executeSqlScript(QSqlDatabase database,
                      const QString &resourcePath,
                      const QString &context)
{
    if (!database.isValid() || !database.isOpen()) {
        qWarning(logWarning()) << context << "database is not open";
        return false;
    }

    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << context << "cannot open" << resourcePath;
        return false;
    }

    const QStringList statements = splitSqlScript(QString::fromUtf8(file.readAll()));
    if (statements.isEmpty()) {
        qWarning(logWarning()) << context << "SQL script is empty" << resourcePath;
        return false;
    }

    QSqlQuery query(database);
    for (qsizetype index = 0; index < statements.size(); ++index) {
        const QString &statement = statements.at(index);
        static const QRegularExpression triggerExpression(
            QStringLiteral(R"(^\s*CREATE\s+TRIGGER\s+(`?[A-Za-z_][A-Za-z0-9_]*`?))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch triggerMatch =
            triggerExpression.match(statement);
        if (triggerMatch.hasMatch()) {
            const QString triggerName = triggerMatch.captured(1);
            if (!query.exec(QStringLiteral("DROP TRIGGER IF EXISTS %1").arg(triggerName))) {
                qWarning(logWarning()) << context << "cannot replace trigger"
                                       << triggerName << query.lastError().text();
                return false;
            }
        }

        if (!query.exec(statement)) {
            qWarning(logWarning())
                << context << "statement" << index + 1 << "of" << statements.size()
                << query.lastError().text();
            return false;
        }
    }
    return true;
}

} // namespace

DataBaseCommon::DataBaseCommon(QObject *parent)
    : QObject{parent}
{}

bool DataBaseCommon::execSingle(const QString &resourcePath, const QString &ctx)
{
    if (!executeSqlScript(QSqlDatabase::database(), resourcePath, ctx))
        return false;
    qInfo(logInfo()) << "Creata tabela -" << ctx;
    return true;
}

bool DataBaseCommon::execFileBatch(const QString &resourcePath, const QString &ctx)
{
    if (!executeSqlScript(QSqlDatabase::database(), resourcePath, ctx))
        return false;
    qInfo(logInfo()) << "Creata tabela -" << ctx;
    return true;
}

bool DataBaseCommon::execFileBatch(QSqlDatabase db,
                                   const QString &resourcePath,
                                   const QString &ctx)
{
    if (!executeSqlScript(db, resourcePath, ctx))
        return false;

    qInfo(logInfo()) << "Obiect SQL creat/verificat -" << ctx;
    return true;
}

QString DataBaseCommon::getTextQryFromResource(const QString &resourcePath)
{
    QFile f(resourcePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning(logWarning()) << "Cannot open" << resourcePath;
        return nullptr;
    }

    QString sql = QString::fromUtf8(f.readAll());
    f.close();

    return sql;
}

bool DataBaseCommon::execPreparedFromFile(QSqlDatabase db,
                                          const QString &sqlPath,
                                          const QVector<QVariant> &binds,
                                          QString *err)
{
    QSqlQuery q(db);
    const QString sql = getTextQryFromResource(sqlPath);

    if (!q.prepare(sql)) {
        if (err) *err = q.lastError().text();
        return false;
    }

    for (const auto &v : binds)
        q.addBindValue(v);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    return true;
}

bool DataBaseCommon::execPreparedFromFileReturnID(QSqlDatabase db,
                                                  const QString &sqlPath,
                                                  const QVector<QVariant> &binds,
                                                  QVariant *lastInsertId,
                                                  QString *err)
{
    QSqlQuery q(db);
    const QString sql = getTextQryFromResource(sqlPath);

    if (!q.prepare(sql)) {
        if (err) *err = q.lastError().text();
        return false;
    }

    for (const auto &v : binds)
        q.addBindValue(v);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }

    if (lastInsertId)
        *lastInsertId = q.lastInsertId();

    return true;
}

bool DataBaseCommon::createAllTablesSqlite()
{
    return createAllTablesSqlite(QSqlDatabase::database());
}

bool DataBaseCommon::createAllTablesSqlite(QSqlDatabase db)
{
    const auto execFileBatch = [&db](const QString &resourcePath, const QString &context) {
        return DataBaseCommon::execFileBatch(db, resourcePath, context);
    };
    const auto execSingle = [&db](const QString &resourcePath, const QString &context) {
        return DataBaseCommon::execFileBatch(db, resourcePath, context);
    };

    if (!execFileBatch(":/sql/sqlite/tables/users.sql", "users"))
        return false;

    if (!execFileBatch(":/sql/sqlite/views/users_combo_view.sql", "users_combo_view"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/doctors_tables.sql", "doctors"))
        return false;

    if (!execFileBatch(":/sql/sqlite/views/doctors_combo_view.sql", "doctors_combo_view"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doctors_triggers_insert.sql", "doctors(triggers insert)"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doctors_triggers_update.sql", "doctors(triggers update)"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/nurses_tables.sql", "nurses"))
        return false;

    if (!execFileBatch(":/sql/sqlite/views/nurses_combo_view.sql", "nurses_combo_view"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/nurses_triggers_insert.sql", "nurses(triggers insert)"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/nurses_triggers_update.sql", "nurses(triggers update)"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/patients_tables.sql", "patients"))
        return false;

    if (!execSingle(":/sql/sqlite/views/patients_completer_view.sql", "patients_completer_view"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/types_prices.sql", "typesPrices"))
        return false;

    if (!execSingle(":/sql/sqlite/views/typePrices_combo_view.sql", "typePrices_combo_view"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/organizations.sql", "organizations"))
        return false;

    if (!execSingle(":/sql/sqlite/views/organizations_combo_view.sql", "organizations_combo_view"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/investigations.sql", "investigations"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/investigations_group.sql", "investigationsGroup"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/constants.sql", "constants"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/contracts.sql", "contracts"))
        return false;

    if (!execSingle(":/sql/sqlite/views/contracts_listForm_view.sql", "contracts_listForm_view"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/pricings.sql", "pricings"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/pricings_triggers_insert.sql", "pricings(triggers insert)"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/pricings_triggers_update.sql", "pricings(triggers update)"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/doc_order_echo.sql", "orderEcho"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doc_order_echo_triggers_insert.sql", "orderEcho(triggers insert)"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doc_order_echo_triggers_update.sql", "orderEcho(triggers update)"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/doc_report_echo.sql", "reportEcho"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doc_report_echo_triggers_insert.sql", "reportEcho(triggers insert)"))
        return false;

    if (!execSingle(":/sql/sqlite/triggers/doc_report_echo_triggers_update.sql", "reportEcho(triggers update)"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/report_video.sql", "report_video"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/organs_internal.sql", "organs_internal"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/urinary_system.sql", "urinary_system"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/prostate.sql", "prostate"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/gynecology.sql", "gynecology"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/breast.sql", "breast"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/thyroid.sql", "thyroid"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/gestation0.sql", "gestation0"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/gestation1.sql", "gestation1"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/gestation2.sql", "gestation2"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/lymph_nodes.sql", "lymph_nodes"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/normograms.sql", "normograms"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/user_preferences.sql", "user_preferences"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/patient_appointments.sql", "patient_appointments"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/conclusion_formations_templates.sql", "conclusion_formations_templates"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/cryptoSplitKey.sql", "cryptoSplitKey"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/online_account.sql", "online_account"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/cloud_server.sql", "cloud_server"))
        return false;

    if (!execFileBatch(":/sql/sqlite/tables/settings_users.sql", "settings_users"))
        return false;

    if (!execSingle(":/sql/sqlite/tables/doc_sequences.sql", "doc_sequences"))
        return false;

    return true;
}

bool DataBaseCommon::createAllTablesMariaDB()
{
    if (!execFileBatch(":/sql/mariadb/tables/users.sql", "users"))
        return false;

    if (!execFileBatch(":/sql/mariadb/views/users_combo_view.sql", "users_combo_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/doctors_tables.sql", "doctors"))
        return false;

    if (!execFileBatch(":/sql/mariadb/views/doctors_combo_view.sql", "doctors_combo_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/triggers/doctors_triggers.sql", "doctors(triggers)"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/nurses_tables.sql", "nurses"))
        return false;

    if (!execFileBatch(":/sql/mariadb/views/nurses_combo_view.sql", "nurses_combo_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/triggers/nurses_triggers.sql", "nurses(triggers)"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/patients_tables.sql", "patients"))
        return false;

    if (!execSingle(":/sql/mariadb/views/patients_completer_view.sql", "patients_completer_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/types_prices.sql", "typesPrices"))
        return false;

    if (!execSingle(":/sql/mariadb/views/typePrices_combo_view.sql", "typePrices_combo_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/organizations.sql", "organizations"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/investigations_group.sql", "investigationsGroup"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/investigations.sql", "investigations"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/constants.sql", "constants"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/contracts.sql", "contracts"))
        return false;

    if (!execSingle(":/sql/mariadb/views/contracts_listForm_view.sql", "contracts_listForm_view"))
        return false;

    // View-ul organizațiilor include contractul implicit, deci poate fi creat
    // numai după existența ambelor tabele.
    if (!execSingle(":/sql/mariadb/views/organizations_combo_view.sql", "organizations_combo_view"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/pricings.sql", "pricings"))
        return false;

    if (!execFileBatch(":/sql/mariadb/triggers/pricings_triggers.sql", "pricings(triggers)"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/doc_order_echo.sql", "orderEcho"))
        return false;

    if (!execFileBatch(":/sql/mariadb/triggers/doc_order_echo_triggers.sql", "orderEcho(triggers)"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/doc_report_echo.sql", "reportEcho"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/image_reports.sql", "imagesReports"))
        return false;

    if (!execFileBatch(":/sql/mariadb/triggers/doc_report_echo_triggers.sql", "reportEcho(triggers)"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/report_video.sql", "report_video"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/organs_internal.sql", "organs_internal"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/urinary_system.sql", "urinary_system"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/prostate.sql", "prostate"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/gynecology.sql", "gynecology"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/breast.sql", "breast"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/thyroid.sql", "thyroid"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/gestation0.sql", "gestation0"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/gestation1.sql", "gestation1"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/gestation2.sql", "gestation2"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/lymph_nodes.sql", "lymph_nodes"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/normograms.sql", "normograms"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/user_preferences.sql", "user_preferences"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/patient_appointments.sql", "patient_appointments"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/conclusion_formations_templates.sql", "conclusion_formations_templates"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/cryptoSplitKey.sql", "cryptoSplitKey"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/online_account.sql", "online_account"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/cloud_server.sql", "cloud_server"))
        return false;

    if (!execFileBatch(":/sql/mariadb/tables/settings_users.sql", "settings_users"))
        return false;

    if (!execSingle(":/sql/mariadb/tables/doc_sequences.sql", "doc_sequences"))
        return false;

    return true;
}

bool DataBaseCommon::createTableDBImageSqlite(QSqlDatabase db, const QString &resourcePath, const QString &ctx)
{
    if (!executeSqlScript(db, resourcePath, ctx))
        return false;
    qInfo(logInfo()) << "Creata tabela -" << ctx;
    return true;
}
