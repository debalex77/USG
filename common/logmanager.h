#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QFile>

class LogManager : public QObject
{
    Q_OBJECT
public:
    // Inițializează fișierul de log și instalează handlerul Qt
    static void init(const QString &path);

private:
    static QScopedPointer<QFile> s_logFile;
    static void handler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);

};

#endif // LOGMANAGER_H
