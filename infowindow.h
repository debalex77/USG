#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QDialog>
#include <QKeyEvent>
#include <data/appsettings.h>
#include <data/globals.h>

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = nullptr);
    ~InfoWindow();

    void setTitle(const QString title);
    void setTex(const QString content);

private:
    Ui::InfoWindow *ui;
    AppSettings *appSettings;

protected:
    void closeEvent(QCloseEvent *event);   // controlam modificarea datelor
};

#endif // INFOWINDOW_H
