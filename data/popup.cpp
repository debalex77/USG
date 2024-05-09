#include "popup.h"

#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QApplication>
#include <QDesktopWidget>
#else
#include <QGuiApplication>
#include <QScreen>
#endif

PopUp::PopUp(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint |        // Отключаем оформление окна
                   Qt::Tool |                       // Отменяем показ в качестве отдельного окна
                   Qt::WindowStaysOnTopHint);       // Устанавливаем поверх всех окон
    setAttribute(Qt::WA_TranslucentBackground);     // Указываем, что фон будет прозрачным
    setAttribute(Qt::WA_ShowWithoutActivating);     // При показе, виджет не получается фокуса автоматически

    animation.setTargetObject(this);                // Устанавливаем целевой объект анимации
    animation.setPropertyName("popupOpacity");      // Устанавливаем анимируемое свойство
    connect(&animation, &QAbstractAnimation::finished, this, &PopUp::hide); /* Подключаем сигнал окончания
                                                                             * анимации к слоты скрытия
                                                                             * */

    // Настройка текста уведомления
    label.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter); // Устанавливаем по центру

    // И настраиваем стили
#if defined(Q_OS_LINUX)

    label.setStyleSheet("QLabel {"
                        "color: white;"
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

#elif defined(Q_OS_MACOS)

    label.setStyleSheet("QLabel {"
                        "color: white;"
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

#elif defined(Q_OS_WINDOWS)

    label.setStyleSheet("QLabel {"
                        "font-size: 12px;"
                        "color: white;"
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");
#endif

    // Производим установку текста в размещение, ...
    layout.addWidget(&label, 0, 0);
    setLayout(&layout); // которое помещаем в виджет

    // По сигналу таймера будет произведено скрытие уведомления, если оно видимо
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &PopUp::hideAnimation);
}

void PopUp::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    /* А теперь настраиваем фон уведомления,
     * который является прямоугольником с чёрным фоном
     * */
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Включаем сглаживание

    // Подготавливаем фон. rect() возвращает внутреннюю геометрию виджета уведомления, по содержимому
    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    // Кисть настраиваем на чёрный цвет в режиме полупрозрачности 180 из 255
    painter.setBrush(QBrush(QColor(0,0,0,180)));
    painter.setPen(Qt::NoPen); // Край уведомления не будет выделен

    // Отрисовываем фон с закруглением краёв в 10px
    painter.drawRoundedRect(roundedRect, 10, 10);
}

void PopUp::setPopupText(const QString &text)
{
    label.setText(text);    // Устанавливаем текст в Label
    adjustSize();           // С пересчётом размеров уведомления
}

void PopUp::show()
{
    setWindowOpacity(0.0);  // Устанавливаем прозрачность в ноль

    animation.setDuration(150);     // Настраиваем длительность анимации
    animation.setStartValue(0.0);   // Стартовое значение будет 0 (полностью прозрачный виджет)
    animation.setEndValue(1.0);     // Конечное - полностью непрозрачный виджет

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    setGeometry(QApplication::desktop()->geometry().width() - 36 - width() + QApplication::desktop()->geometry().x(),
                QApplication::desktop()->geometry().height() - 56 - height() + QApplication::desktop()->geometry().y(),
                width(),
                height());
#else
    setGeometry(QGuiApplication::primaryScreen()->geometry().width() - 36 - width() + QGuiApplication::primaryScreen()->geometry().x(),
                QGuiApplication::primaryScreen()->geometry().height() - 56 - height() + QGuiApplication::primaryScreen()->geometry().y(),
                width(),
                height());
#endif


    QWidget::show();                // Отображаем виджет, который полностью прозрачен

    animation.start();              // И запускаем анимацию
    timer->start(5000);             // А также стартуем таймер, который запустит скрытие уведомления через 5 секунды
}

void PopUp::showFromGeometry(QPoint p)
{
    setWindowOpacity(0.0);  // Устанавливаем прозрачность в ноль

    animation.setDuration(150);     // Настраиваем длительность анимации
    animation.setStartValue(0.0);   // Стартовое значение будет 0 (полностью прозрачный виджет)
    animation.setEndValue(1.0);     // Конечное - полностью непрозрачный виджет

    QSize s(width(), height());
    setGeometry(QRect(p, s));

    QWidget::show();                // Отображаем виджет, который полностью прозрачен

    animation.start();              // И запускаем анимацию
}

void PopUp::showFromGeometryTimer(QPoint p)
{
    setWindowOpacity(0.0);  // Устанавливаем прозрачность в ноль

    animation.setDuration(150);     // Настраиваем длительность анимации
    animation.setStartValue(0.0);   // Стартовое значение будет 0 (полностью прозрачный виджет)
    animation.setEndValue(1.0);     // Конечное - полностью непрозрачный виджет

    QSize s(width(), height());
    setGeometry(QRect(p, s));

    QWidget::show();                // Отображаем виджет, который полностью прозрачен

    animation.start();              // И запускаем анимацию
    timer->start(3000);             // А также стартуем таймер, который запустит скрытие уведомления через 3 секунды
}

void PopUp::hideAnimation()
{
    timer->stop();                  // Останавливаем таймер
    animation.setDuration(1000);    // Настраиваем длительность анимации
    animation.setStartValue(1.0);   // Стартовое значение будет 1 (полностью непрозрачный виджет)
    animation.setEndValue(0.0);     // Конечное - полностью прозрачный виджет
    animation.start();              // И запускаем анимацию
}

void PopUp::hide()
{
    // Если виджет прозрачный, то скрываем его
    if(getPopupOpacity() == 0.0){
        QWidget::hide();
    }
}

void PopUp::hidePop()
{
    QWidget::hide();
}

void PopUp::setPopupOpacity(double opacity)
{
    popupOpacity = opacity;
    emit popupOpacityChanged();
    setWindowOpacity(opacity);
}

double PopUp::getPopupOpacity() const
{
    return popupOpacity;
}
