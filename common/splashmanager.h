#ifndef SPLASHMANAGER_H
#define SPLASHMANAGER_H

#include <QObject>
#include <common/version.h>

class QWidget;

class SplashManager : public QObject
{
    Q_OBJECT
public:
    static void show(QWidget &mainWindow, int durationMs = 5000);

signals:
};

#endif // SPLASHMANAGER_H
