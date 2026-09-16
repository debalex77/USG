#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QFile>
#include <QMutex>
#include <QDate>

class QFileInfo;

class LogManager : public QObject
{
    Q_OBJECT
public:
    // Initializeaza fisierul, roteste logul din ziua precedenta si aplica retentia.
    // O valoare negativa pastreaza toate arhivele.
    [[nodiscard]] static bool init(const QString &path, int retainedArchives = -1);
    static void shutdown();

private:
    static QDate firstEntryDate(const QFileInfo &fileInfo);
    static void rotateActiveLog(const QFileInfo &activeLogInfo, QStringList &errors);
    static void removeExpiredArchives(const QFileInfo &activeLogInfo,
                                      int retainedArchives, QStringList &errors);
    static void rotateIfDateChanged(const QDate &currentDate, QStringList &errors);
    static QScopedPointer<QFile> s_logFile;
    static QMutex s_mutex;
    static QtMessageHandler s_previousHandler;
    static bool s_handlerInstalled;
    static QString s_logPath;
    static QDate s_activeDate;
    static int s_retainedArchives;
    static void handler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);

};

#endif // LOGMANAGER_H
