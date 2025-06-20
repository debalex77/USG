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
    Q_PROPERTY(TypeInfo typeInfo READ getTypeInfo WRITE setTypeInfo NOTIFY typeInfoChanged)

public:
    explicit InfoWindow(QWidget *parent = nullptr);
    ~InfoWindow();

    enum TypeInfo { INFO_REALEASE, INFO_VIDEO, INFO_REPORT };

    TypeInfo getTypeInfo();
    void setTypeInfo(TypeInfo typeInfo);

    void setTitle(const QString title);
    void setTex(const QString content);

signals:
    void typeInfoChanged();

private slots:
    void slot_typeInfoChanged();

private:
    Ui::InfoWindow *ui;
    TypeInfo m_typeInfo;
    AppSettings *appSettings;

protected:
    void closeEvent(QCloseEvent *event); // controlam modificarea datelor
};

#endif // INFOWINDOW_H
