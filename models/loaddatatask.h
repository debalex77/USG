#ifndef LOADDATATASK_H
#define LOADDATATASK_H

#include <QObject>
#include <QRunnable>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <data/globals.h>

class LoadDataTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    LoadDataTask(int limit, int offset, QObject *parent = nullptr);

    void setQuery(const QString strQry);
    void run() override;

signals:
    void dataReady(const QVector<QVector<QVariant>> &rows);
    void columnCountDetermined(int columnCount);

private:
    int limit;
    int offset;
    int columnCountValue = 0;
    QString m_strQry = nullptr;
};

#endif // LOADDATATASK_H
