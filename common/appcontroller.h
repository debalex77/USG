#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>

class QApplication;
class MainWindow;
class AppSettings; // definit în <data/appsettings.h>

class AppController : public QObject
{
    Q_OBJECT
public:
    int run(int &argc, char **argv);
private:
    void applyGlobalFont();
    void applyStyleSheet();
    int  handleLaunchFlow(char **);

    AppSettings *m_appSettings = nullptr;
    MainWindow  *m_mainWin     = nullptr;
};

#endif // APPCONTROLLER_H
