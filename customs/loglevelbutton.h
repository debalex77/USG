#ifndef LOGLEVELBUTTON_H
#define LOGLEVELBUTTON_H

#include <QToolButton>
#include <QButtonGroup>
#include <QWidget>
#include <QMenu>
#include <QRadioButton>
#include <QWidgetAction>
#include <data/globals.h>

class LogLevelButton : public QToolButton
{
    Q_OBJECT
public:
    LogLevelButton(QWidget *parent = nullptr);

signals:
    void selectedLevel(const QString level);

private:
    QButtonGroup *group;
};

#endif // LOGLEVELBUTTON_H
