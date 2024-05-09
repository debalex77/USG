#ifndef INITLAUNCH_H
#define INITLAUNCH_H

#include <QDialog>
#include <QTranslator>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStyleFactory>
#include <data/globals.h>

namespace Ui {
class InitLaunch;
}

class InitLaunch : public QDialog
{
    Q_OBJECT

public:
    explicit InitLaunch(QWidget *parent = nullptr);
    ~InitLaunch();

private:
    void dataWasModified();

private slots:
    void changeIndexComboLangApp(const int _index);
    void onClickedRadBtnFirstLaunch();
    void onClickedRadBtnAppMove();

    void onWritingData();
    void onClose();

private:
    Ui::InitLaunch *ui;
    QTranslator    *translator;
    QString         nameLocale = nullptr;
    QStyle         *style_fusion = QStyleFactory::create("Fusion");

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
};

#endif // INITLAUNCH_H
