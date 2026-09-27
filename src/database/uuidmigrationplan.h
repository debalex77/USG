#pragma once

#include <QSqlDatabase>
#include <QList>
#include <QStringList>

// Read-only matching, independent of the UI and of UUID generation.
namespace UuidMigration {

struct Match {
    QString localTable;
    QString cloudTable;
    qint64 localId = 0;
    qint64 cloudId = 0;
    QByteArray uuid;
};

struct Plan {
    QList<Match> matches;
    QStringList summary;
    QStringList warnings;
    QStringList conflicts;
    bool valid() const { return conflicts.isEmpty(); }
};

// lockCloud requires an active MariaDB transaction and uses FOR UPDATE.
// No DDL or data writes are performed here. Missing local UUIDs are allowed
// during the preliminary audit, before the migration generates them.
Plan build(QSqlDatabase local, QSqlDatabase images, QSqlDatabase cloud,
           bool requireLocalUuid = false, bool lockCloud = false);

}
