#ifndef BALLOONTIP_H
#define BALLOONTIP_H

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPointer>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

class BalloonTip final : public QWidget
{
    Q_OBJECT

public:
    enum ArrowDirection {
        TopLeft      = 1,
        RightTop     = 2,
        RightCenter  = 3,
        RightBottom  = 4,
        BottomRight  = 5,
        BottomCenter = 6,
        BottomLeft   = 7,
        LeftBottom   = 8,
        LeftCenter   = 9,
        LeftTop      = 10,
        TopRight     = 11,
        TopCenter    = 12
    };

    static void showBalloon(QMessageBox::Icon icon,
                            const QString &title,
                            const QString &message,
                            const QPoint &pos,
                            int timeoutMs = 10000,
                            bool showArrow = true,
                            ArrowDirection arrowDir = BottomCenter);

    static void showBalloonFor(QWidget *w,
                               QMessageBox::Icon icon,
                               const QString &title,
                               const QString &msg,
                               int timeout,
                               bool showArrow,
                               ArrowDirection dir);

    static void hideBalloon();
    static bool isBalloonVisible();
    static void updateBalloonPosition(const QPoint &pos);

public:
    explicit BalloonTip(QMessageBox::Icon icon,
                        const QString &title,
                        const QString &message,
                        QWidget *parent = nullptr);
    ~BalloonTip() override;

    void balloon(const QPoint &pos,
                 int timeoutMs,
                 bool showArrow,
                 ArrowDirection arrowDir);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void buildBalloon(const QPoint &pos);
    QRect availableScreenGeometry(const QPoint &pos) const;
    QPoint clampToScreen(const QPoint &topLeft,
                         const QSize &size,
                         const QRect &screenRect) const;

private:
    static QPointer<BalloonTip> s_balloonTip;

    QPixmap m_pixmap;
    int m_timerId = 0;
    bool m_showArrow = true;
    ArrowDirection m_arrowDir = BottomCenter;

    QGridLayout *m_layout = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_msgLabel = nullptr;
    QLabel *m_iconLabel = nullptr;
};

#endif // BALLOONTIP_H
