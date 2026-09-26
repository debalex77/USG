#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QWidget>

class WindowManager
{
public:
    static void resize(QWidget *w, double wPct = 0.8, double hPct = 0.8);
};

#endif // WINDOWMANAGER_H
