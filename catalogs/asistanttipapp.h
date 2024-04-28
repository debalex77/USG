#ifndef ASISTANTTIPAPP_H
#define ASISTANTTIPAPP_H

#include <QDialog>
#include <QStyleFactory>
#include <data/database.h>

namespace Ui {
class AsistantTipApp;
}

class AsistantTipApp : public QDialog
{
    Q_OBJECT

public:
    explicit AsistantTipApp(QWidget *parent = nullptr);
    ~AsistantTipApp();

    void setStep(int m_step = 0);

private slots:
    void stepNext();
    void stepPreview();
    void onClose();

private:
    void setTipText();

private:
    Ui::AsistantTipApp *ui;
    DataBase* db;

    int current_step = 0;
    int max_step = 5;
    QStyle* style_fusion = QStyleFactory::create("Fusion");
};

#endif // ASISTANTTIPAPP_H
