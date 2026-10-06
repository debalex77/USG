// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Modified in 2026 for the USG QSQLCIPHER driver.

#ifndef QSQL_SQLCIPHER_H
#define QSQL_SQLCIPHER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtSql/qsqldriver.h>

struct sqlite3;

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_SQLCIPHER
#else
#define Q_EXPORT_SQLDRIVER_SQLCIPHER Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE

class QSqlResult;
class QSQLCipherDriverPrivate;

class Q_EXPORT_SQLDRIVER_SQLCIPHER QSQLCipherDriver : public QSqlDriver
{
    Q_DECLARE_PRIVATE(QSQLCipherDriver)
    Q_OBJECT
    friend class QSQLCipherResultPrivate;
public:
    explicit QSQLCipherDriver(QObject *parent = nullptr);
    explicit QSQLCipherDriver(sqlite3 *connection, QObject *parent = nullptr);
    ~QSQLCipherDriver();
    bool hasFeature(DriverFeature f) const override;
    bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts) override;
    void close() override;
    QSqlResult *createResult() const override;
    bool beginTransaction() override;
    bool commitTransaction() override;
    bool rollbackTransaction() override;
    QStringList tables(QSql::TableType) const override;

    QSqlRecord record(const QString& tablename) const override;
    QSqlIndex primaryIndex(const QString &tablename) const override;
    QVariant handle() const override;

    QString escapeIdentifier(const QString &identifier, IdentifierType type) const override;
    bool isIdentifierEscaped(const QString &identifier, IdentifierType type) const override;
    QString stripDelimiters(const QString &identifier, IdentifierType type) const override;

    bool subscribeToNotification(const QString &name) override;
    bool unsubscribeFromNotification(const QString &name) override;
    QStringList subscribedToNotifications() const override;
private Q_SLOTS:
    void handleNotification(const QString &tableName, qint64 rowid);
};

QT_END_NAMESPACE

#endif // QSQL_SQLCIPHER_H
