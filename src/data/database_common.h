#pragma once

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDebug>

#include <data/loggingcategories.h>

class DataBaseCommon : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseCommon(QObject *parent = nullptr);

    static bool execSingle(const QString &resourcePath, const QString &ctx);
    static bool execFileBatch(const QString &resourcePath, const QString &ctx);
    static bool execFileBatch(QSqlDatabase db,
                              const QString &resourcePath,
                              const QString &ctx);
    static QString getTextQryFromResource(const QString &resourcePath);
    static bool execPreparedFromFile(QSqlDatabase db,
                                     const QString &sqlPath,
                                     const QVector<QVariant> &binds,
                                     QString *err = nullptr);

    static bool execPreparedFromFileReturnID(QSqlDatabase db,
                                     const QString &sqlPath,
                                     const QVector<QVariant> &binds,
                                     QVariant *lastInsertId,
                                     QString *err = nullptr);

    static bool createAllTablesSqlite();
    static bool createAllTablesSqlite(QSqlDatabase db);
    static bool createAllTablesMariaDB();
    static bool createTableDBImageSqlite(QSqlDatabase db,
                                         const QString &resourcePath,
                                         const QString &ctx);
};
