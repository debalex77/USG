#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QTimer>

class PopUp : public QWidget
{
    Q_OBJECT

    // proprietatea transparentei
    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity NOTIFY popupOpacityChanged)

    void setPopupOpacity(double opacity);
    double getPopupOpacity() const;

public:
    explicit PopUp(QWidget *parent = nullptr);

signals:
    void popupOpacityChanged();

protected:
    void paintEvent(QPaintEvent *event);    // fundalul v-a fi desenat

public slots:
    void setPopupText(const QString& text); // instalarea textului
    void show();                            /* metoda lansarii widget-lui
                                             * sunt necesare optiuni suplimentare
                                             * */
    void showFromGeometry(QPoint p);
    void showFromGeometryTimer(QPoint p);
    void hidePop();

private slots:
    void hideAnimation();                   // Slot p-u lansarea animatiei ascunderii widget-lui
    void hide();                            /* La finisare se efectuiaza control daca widgetul este vizibil,
                                             * sau este necesar de ascuns
                                             * */

private:
    QLabel label;                   // Label p-u mesaje
    QGridLayout layout;             // layot
    QPropertyAnimation animation;   // Proprietatea animatiei p-u evidentierea mesajului
    double popupOpacity;            // proprietatea transparentei widget-lui
    QTimer *timer;                  // timer
};

#endif // POPUP_H
