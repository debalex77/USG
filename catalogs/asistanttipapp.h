#ifndef ASISTANTTIPAPP_H
#define ASISTANTTIPAPP_H

#include <QDialog>

namespace Ui {
class AsistantTipApp;
}

class AsistantTipApp : public QDialog
{
    Q_OBJECT

public:
    explicit AsistantTipApp(QWidget *parent = nullptr);
    ~AsistantTipApp();

private slots:
    void stepNext();
    void stepPreview();

private:
    void setTipText();

private:
    Ui::AsistantTipApp *ui;

    int current_step = 0;
};

#endif // ASISTANTTIPAPP_H
