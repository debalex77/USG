#include "uuidmigrationplan.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QHash>
#include <QIODevice>
#include <QMap>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace UuidMigration {
namespace {
using Row = QMap<QString, QVariant>;
struct Spec { QString table; QStringList fields; };
const QList<Spec> specs = {
    {"patients", {"last_name", "first_name", "birthday"}},
    {"organizations", {"name", "address"}},
    {"doctors", {"name", "fName", "mName"}},
    {"nurses", {"name", "fName", "mName"}},
    {"users", {"name"}},
    // Names are translated when the catalog is initialized and discount is
    // mutable. The commercial/CNAM flag is the stable semantic discriminator.
    {"typesPrices", {"noncomercial"}},
    {"investigationsGroup", {"name"}},
    {"investigations", {"cod", "name"}},
    {"conclusionTemplates", {"cod", "name", "system"}},
    {"formationsSystemTemplates", {"name", "typeSystem"}},
    {"contracts", {"name", "dateInit", "id_organizations", "id_typesPrices"}},
    {"pricings", {"numberDoc", "dateDoc", "id_typesPrices", "id_organizations", "id_contracts"}},
    {"orderEcho", {"patient_id", "id_organizations", "numberDoc", "dateDoc"}},
    {"reportEcho", {"patient_id", "id_orderEcho", "numberDoc", "dateDoc"}},
    {"imagesReports", {"patient_id", "id_orderEcho", "id_reportEcho",
                       "image_1", "image_2", "image_3", "image_4", "image_5"}}
};
const QMap<QString, QString> parents = {
    {"patient_id", "patients"}, {"id_organizations", "organizations"},
    {"id_typesPrices", "typesPrices"}, {"id_contracts", "contracts"},
    {"id_orderEcho", "orderEcho"}, {"id_reportEcho", "reportEcho"}
};

QString actualTable(QSqlDatabase db, const QString &table)
{
    for (const QString &name : db.tables(QSql::Tables))
        if (name.compare(table, Qt::CaseInsensitive) == 0
            || (table == "patients" && name.compare("pacients", Qt::CaseInsensitive) == 0))
            return name;
    return {};
}

QString canonical(const QString &table, QString column)
{
    column = column.toLower();

    if (column == "id_pacients" || column == "id_patients")
        return "patient_id";

    if (table == "patients") {
        if (column == "name") return
                "last_name";

        if (column == "fname") return
                "first_name";
    }

    if (table == "organizations" && column == "idno")
        return "idnp";

    return column;
}

bool readRows(QSqlDatabase db, const QString &table, const Spec &spec,
              bool lock, QList<Row> &rows, Plan &plan)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec(QStringLiteral(R"(
        SELECT * FROM `%1` ORDER BY id
    )").arg(table) + (lock ? QStringLiteral(" FOR UPDATE") : QString()))) {
        plan.conflicts << QStringLiteral("%1: read failed (%2)").arg(spec.table, query.lastError().nativeErrorCode());
        return false;
    }
    const QSqlRecord schema = query.record();
    QSet<QString> columns;
    for (int i = 0; i < schema.count(); ++i)
        columns.insert(canonical(spec.table, schema.fieldName(i)));

    QStringList required = spec.fields;
    required << "id";
    if (spec.table == "patients" || spec.table == "organizations")
        required << "idnp";

    for (const QString &field : std::as_const(required)) {
        if (!columns.contains(field.toLower())) {
            plan.conflicts << QStringLiteral("%1: missing identity column %2").arg(spec.table, field);
            return false;
        }
    }
    while (query.next()) {
        Row row;
        // Only retain identity fields, ID and UUID. In particular, do not retain
        // passwords, signatures, free-form clinical content or image payloads.
        for (int i = 0; i < schema.count(); ++i) {
            const QString field = canonical(spec.table, schema.fieldName(i));
            if (field == "uuid" || required.contains(field, Qt::CaseInsensitive)) {
                const QVariant value = query.value(i);
                row[field] = field.startsWith("image_")
                    ? QVariant(QCryptographicHash::hash(value.toByteArray(), QCryptographicHash::Sha256))
                    : value;
            }
        }
        rows.append(row);
    }
    if (query.lastError().isValid()) {
        plan.conflicts << QStringLiteral("%1: incomplete query").arg(spec.table);
        return false;
    }
    return true;
}

QString normalized(const QString &field, const QVariant &value)
{
    if (field == "birthday" || field == "dateInit") {
        QDate date = value.toDate();
        if (!date.isValid())
            date = QDate::fromString(value.toString(), "dd.MM.yyyy");

        return date.isValid() ? date.toString(Qt::ISODate) : QString();
    }

    if (field == "dateDoc") {
        QDateTime date = value.toDateTime();
        if (!date.isValid())
            date = QDateTime::fromString(value.toString(), "yyyy-MM-dd HH:mm:ss");

        if (!date.isValid())
            date = QDateTime::fromString(value.toString(), "dd.MM.yyyy HH:mm:ss");

        return date.isValid() ? date.toString("yyyy-MM-dd HH:mm:ss") : QString();
    }

    if (field == "discount")
        return QString::number(value.toDouble(), 'g', 15);

    if (field == "noncomercial")
        return QString::number(value.toInt());

    if (field.startsWith("image_"))
        return QString::fromLatin1(value.toByteArray().toHex());

    return value.toString().normalized(QString::NormalizationForm_C).simplified().toCaseFolded();
}

QByteArray identity(const Spec &spec, const Row &row, bool local,
                    const QMap<QString, QMap<qint64, qint64>> &maps)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (const QString &field : spec.fields) {
        const QVariant value = row.value(field.toLower());
        if (parents.contains(field)) {
            qint64 id = value.toLongLong();
            if (local && id > 0) {
                id = maps.value(parents.value(field)).value(id, -1);

                if (id < 0)
                    return {}; // Parent has no counterpart; never use its local ID.
            }
            stream << id;
        } else {
            const QString text = normalized(field, value);
            if (text.isEmpty() && (field == "last_name" || field == "first_name"
                || field == "name" || field == "birthday" || field == "numberDoc"
                || field == "dateDoc" || field == "cod"))
                return {};
            stream << text;
        }
    }
    return bytes;
}
}

Plan build(QSqlDatabase local, QSqlDatabase images, QSqlDatabase cloud,
           bool requireLocalUuid, bool lockCloud)
{
    Plan plan;
    QMap<QString, QMap<qint64, qint64>> maps;
    for (const Spec &spec : specs) {

        QSqlDatabase source = spec.table == "imagesReports" ? images : local;

        if (spec.table == "imagesReports" && !source.isOpen())
            continue;

        const QString localTable = actualTable(source, spec.table);
        const QString cloudTable = actualTable(cloud, spec.table);

        if (localTable.isEmpty() || cloudTable.isEmpty()) {
            plan.conflicts << QStringLiteral("%1: missing table").arg(spec.table);
            continue;
        }

        QList<Row> locals, remotes;
        if (!readRows(source, localTable, spec, false, locals, plan)
            || !readRows(cloud, cloudTable, spec, lockCloud, remotes, plan))
            continue;

        QHash<QByteArray, QList<int>> keys;
        QHash<QString, QList<int>> identifiers;
        QHash<QByteArray, qint64> cloudUuids;
        QSet<QByteArray> localUuids;

        for (int i = 0; i < remotes.size(); ++i) {
            const Row &row = remotes.at(i);
            const QByteArray key = identity(spec, row, false, maps);
            if (!key.isEmpty())
                keys[key].append(i);

            const QString idnp = normalized("idnp", row.value("idnp"));
            if (!idnp.isEmpty())
                identifiers[idnp].append(i);

            const QByteArray uuid = row.value("uuid").toByteArray();
            if (!uuid.isEmpty()) {
                if (uuid.size() != 16 || cloudUuids.contains(uuid))
                    plan.conflicts << QStringLiteral("%1: invalid/duplicate cloud UUID").arg(spec.table);
                cloudUuids[uuid] = row.value("id").toLongLong();
            }
        }

        QSet<qint64> claimed;
        int matched = 0, unmatched = 0, skipped = 0;
        for (const Row &row : std::as_const(locals)) {

            const qint64 id = row.value("id").toLongLong();
            const QByteArray uuid = row.value("uuid").toByteArray();
            const QString label = QStringLiteral("%1 local_id=%2").arg(spec.table).arg(id);

            if ((requireLocalUuid || !uuid.isEmpty()) && (uuid.size() != 16 || localUuids.contains(uuid)))
                plan.conflicts << label + ": invalid/duplicate local UUID";

            if (!uuid.isEmpty())
                localUuids.insert(uuid);

            const QByteArray key = identity(spec, row, true, maps);
            const QString idnp   = normalized("idnp", row.value("idnp"));
            const bool person    = spec.table == "patients" || spec.table == "organizations";

            QList<int> candidates = person && !idnp.isEmpty()
                ? identifiers.value(idnp) : keys.value(key);

            // For patients, an equal IDNP is necessary but not sufficient:
            // NPP and birthday must describe the same person as well. Historical
            // data contains reused/mistyped identifiers, so such rows are left
            // independent instead of receiving a potentially wrong UUID.
            if (spec.table == "patients" && !idnp.isEmpty() && !candidates.isEmpty()) {
                QList<int> compatible;
                for (int candidate : std::as_const(candidates)) {
                    if (identity(spec, remotes.at(candidate), false, maps) == key)
                        compatible.append(candidate);
                }
                if (compatible.isEmpty()) {
                    if (plan.warnings.size() < 100)
                        plan.warnings << label + ": skipped (IDNP/NPP/birthday disagreement)";
                    ++skipped;
                    continue;
                }
                candidates = compatible;
            }
            // A demographic match with a different non-empty IDNP is kept as
            // a separate patient. It must not block unrelated safe matches.
            if (person && candidates.isEmpty() && !idnp.isEmpty() && keys.contains(key)) {
                if (plan.warnings.size() < 100)
                    plan.warnings << label + ": skipped (identity/IDNP disagreement)";
                ++skipped;
                continue;
            }

            if (candidates.isEmpty()) {
                if (!uuid.isEmpty() && cloudUuids.contains(uuid))
                    plan.conflicts << label + ": UUID exists for a different identity";
                ++unmatched;
                continue;
            }

            if (candidates.size() != 1) {
                // Equal numeric IDs are not identity evidence. Historical IDs
                // may differ between SQLite and MariaDB, and may also coincide
                // accidentally. Every duplicate identity therefore stays
                // untouched for later manual review.
                if (plan.warnings.size() < 100)
                    plan.warnings << label + ": skipped (ambiguous identity)";
                ++skipped;
                continue;
            }

            const Row &remote = remotes.at(candidates.first());
            const qint64 remoteId = remote.value("id").toLongLong();
            if (claimed.contains(remoteId)) {
                if (plan.warnings.size() < 100)
                    plan.warnings << label + ": skipped (cloud row already claimed)";
                ++skipped;
                continue;
            }

            const QByteArray remoteUuid = remote.value("uuid").toByteArray();
            if ((!remoteUuid.isEmpty() && remoteUuid != uuid)
                || (!uuid.isEmpty() && cloudUuids.contains(uuid) && cloudUuids.value(uuid) != remoteId)) {
                plan.conflicts << label + ": existing UUID conflict";
                continue;
            }
            claimed.insert(remoteId);
            maps[spec.table][id] = remoteId;
            plan.matches.append({localTable, cloudTable, id, remoteId, uuid});
            ++matched;
        }
        plan.summary << QStringLiteral("%1: matched=%2 local_only=%3 skipped=%4 cloud_unmatched=%5")
                            .arg(spec.table).arg(matched).arg(unmatched).arg(skipped)
                            .arg(remotes.size() - claimed.size());
    }
    return plan;
}
}
