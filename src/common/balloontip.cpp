#include "balloontip.h"
#include "common/globals.h"

#include <QBitmap>
#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QStyle>

QPointer<BalloonTip> BalloonTip::s_balloonTip = nullptr;

//************************************************************
//******* STATIC FUNCTION

void BalloonTip::showBalloon(QMessageBox::Icon icon,
                             const QString &title,
                             const QString &message,
                             const QPoint &pos,
                             int timeoutMs,
                             bool showArrow,
                             ArrowDirection arrowDir)
{
    hideBalloon();

    if (title.trimmed().isEmpty() && message.trimmed().isEmpty())
        return;

    s_balloonTip = new BalloonTip(icon, title, message);
    s_balloonTip->balloon(pos, timeoutMs, showArrow, arrowDir);
}

void BalloonTip::showBalloonFor(QWidget *w,
                                QMessageBox::Icon icon,
                                const QString &title,
                                const QString &msg,
                                int timeout,
                                bool showArrow,
                                ArrowDirection dir)
{
    if (!w)
        return;

    QPoint pt;

    switch (dir) {
    case BottomCenter:
        pt = w->mapToGlobal(QPoint(w->width() / 2, w->height() / 2));
        break;

    case TopCenter:
        pt = w->mapToGlobal(QPoint(w->width() / 2, w->height() / 2));
        break;

    case LeftCenter:
        pt = w->mapToGlobal(QPoint(0, w->height() / 2));
        break;

    case RightCenter:
        pt = w->mapToGlobal(QPoint(w->width(), w->height() / 2));
        break;

    default:
        // fallback – centru
        pt = w->mapToGlobal(QPoint(w->width() / 2, w->height() / 2));
        break;
    }

    showBalloon(icon, title, msg, pt, timeout, showArrow, dir);
}

void BalloonTip::hideBalloon()
{
    if (!s_balloonTip)
        return;

    s_balloonTip->close();
    delete s_balloonTip;
    s_balloonTip = nullptr;
}

bool BalloonTip::isBalloonVisible()
{
    return !s_balloonTip.isNull();
}

void BalloonTip::updateBalloonPosition(const QPoint &pos)
{
    if (!s_balloonTip)
        return;

    s_balloonTip->buildBalloon(pos);
    s_balloonTip->update();
}

//************************************************************
//******* MAIN FUNCTION

BalloonTip::BalloonTip(QMessageBox::Icon icon,
                       const QString &title,
                       const QString &message,
                       QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setText(title);
    m_titleLabel->setTextFormat(Qt::PlainText);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    m_msgLabel = new QLabel(this);
    m_msgLabel->setText(message);
    m_msgLabel->setTextFormat(Qt::RichText);
    m_msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    const QRect screenRect = availableScreenGeometry(QCursor::pos());
    const int maxTextWidth = qMax(260, screenRect.width() / 3);

    if (m_msgLabel->sizeHint().width() > maxTextWidth) {
        m_msgLabel->setWordWrap(true);
        m_msgLabel->setFixedSize(maxTextWidth,
                                 m_msgLabel->heightForWidth(maxTextWidth));
    }

    QIcon standardIcon;
    switch (icon) {
    case QMessageBox::Warning:
        standardIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    case QMessageBox::Critical:
        standardIcon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case QMessageBox::Information:
        standardIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case QMessageBox::Question:
        standardIcon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
        break;
    case QMessageBox::NoIcon:
    default:
        break;
    }

    constexpr int iconSize = 18;

    m_layout = new QGridLayout(this);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(6);

    if (!standardIcon.isNull()) {
        m_iconLabel = new QLabel(this);
        m_iconLabel->setPixmap(standardIcon.pixmap(iconSize, iconSize));
        m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        m_iconLabel->setMargin(2);

        m_layout->addWidget(m_iconLabel,  0, 0);
        m_layout->addWidget(m_titleLabel, 0, 1);
        m_layout->addWidget(m_msgLabel,   1, 0, 1, 2);
    } else {
        m_layout->addWidget(m_titleLabel, 0, 0);
        m_layout->addWidget(m_msgLabel,   1, 0);
    }

    QString bgColor = globals().isSystemThemeDark ? "#f5f5f5" : "#2b2b2b";
    QString textColor = globals().isSystemThemeDark ? "#000000" : "#EAEAEA";

    m_titleLabel->setStyleSheet(QString("color:%1; font-weight:bold; background:transparent;").arg(textColor));
    m_msgLabel->setStyleSheet(QString("color:%1; background:transparent;").arg(textColor));

    setStyleSheet(QString(
                      "BalloonTip { background-color: %1; }"
                      "QLabel { color: %2; background: transparent; }"
                      ).arg(bgColor, textColor));

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(bgColor));
    pal.setColor(QPalette::WindowText, QColor(textColor));
    setPalette(pal);

    auto *effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);
}

BalloonTip::~BalloonTip()
{
    if (s_balloonTip == this)
        s_balloonTip = nullptr;
}

void BalloonTip::balloon(const QPoint &pos,
                         int timeoutMs,
                         bool showArrow,
                         ArrowDirection arrowDir)
{
    m_showArrow = showArrow;
    m_arrowDir = arrowDir;

    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }

    buildBalloon(pos);

    if (timeoutMs < 0)
        timeoutMs = 10000;

    if (timeoutMs > 0)
        m_timerId = startTimer(timeoutMs);

    auto *effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);

    show();
    raise();

    auto *anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(260);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void BalloonTip::buildBalloon(const QPoint &pos)
{
    constexpr int border = 1;
    constexpr int arrowHeight = 18;
    constexpr int arrowOffset = 18;
    constexpr int arrowWidth = 18;
    constexpr int radius = 7;
    constexpr int baseMargin = 10;

    bool arrowAtTop = false;
    bool arrowAtRight = false;
    bool arrowAtBottom = false;
    bool arrowAtLeft = false;

    if (m_showArrow) {
        switch (m_arrowDir) {
        // balon JOS de punct -> sageata SUS
        case BottomLeft:
        case BottomCenter:
        case BottomRight:
            arrowAtTop = true;
            break;

        // balon STANGA de punct -> sageata DREAPTA
        case LeftTop:
        case LeftCenter:
        case LeftBottom:
            arrowAtRight = true;
            break;

        // balon SUS de punct -> sageata JOS
        case TopLeft:
        case TopCenter:
        case TopRight:
            arrowAtBottom = true;
            break;

        // balon DREAPTA de punct -> sageata STANGA
        case RightTop:
        case RightCenter:
        case RightBottom:
            arrowAtLeft = true;
            break;
        }
    }

    if (m_layout) {
        m_layout->setContentsMargins(baseMargin + border + (arrowAtLeft ? arrowHeight : 0),
                                     baseMargin + border + (arrowAtTop ? arrowHeight : 0),
                                     baseMargin + border + (arrowAtRight ? arrowHeight : 0) + 1,
                                     baseMargin + border + (arrowAtBottom ? arrowHeight : 0) + 1);
        m_layout->activate();
    }

    adjustSize();
    const QSize sh = sizeHint();

    const qreal ml = (arrowAtLeft ? arrowHeight : 0) + 0.5;
    const qreal mt = (arrowAtTop ? arrowHeight : 0) + 0.5;
    const qreal mr = sh.width() - (arrowAtRight ? arrowHeight : 0) - 0.5;
    const qreal mb = sh.height() - (arrowAtBottom ? arrowHeight : 0) - 0.5;

    QPainterPath path;
    path.moveTo(ml + radius, mt);

    QPoint topLeft = pos;

    // =========================================================
    // TOP SIDE  (sageata SUS => balon SUB punct)
    // =========================================================
    if (arrowAtTop) {
        if (m_arrowDir == BottomLeft) {
            path.lineTo(ml + arrowOffset, mt);
            path.lineTo(ml + arrowOffset, mt - arrowHeight);
            path.lineTo(ml + arrowOffset + arrowWidth, mt);

            topLeft = QPoint(pos.x() - arrowOffset, pos.y());
        }
        else if (m_arrowDir == BottomRight) {
            path.lineTo(mr - arrowOffset - arrowWidth, mt);
            path.lineTo(mr - arrowOffset, mt - arrowHeight);
            path.lineTo(mr - arrowOffset, mt);

            topLeft = QPoint(pos.x() - sh.width() + arrowOffset, pos.y());
        }
        else if (m_arrowDir == BottomCenter) {
            const int x = (sh.width() - arrowWidth) / 2;
            path.lineTo(ml + x, mt);
            path.lineTo(ml + x + arrowWidth / 2, mt - arrowHeight);
            path.lineTo(ml + x + arrowWidth, mt);

            topLeft = QPoint(pos.x() - x - radius, pos.y());
        }
    }

    path.lineTo(mr - radius, mt);
    path.arcTo(mr - radius * 2, mt, radius * 2, radius * 2, 90, -90);

    // =========================================================
    // RIGHT SIDE (sageata DREAPTA => balon STANGA de punct)
    // =========================================================
    if (arrowAtRight) {
        if (m_arrowDir == LeftTop) {
            path.lineTo(mr, mt + arrowOffset);
            path.lineTo(mr + arrowHeight, mt + arrowOffset);
            path.lineTo(mr, mt + arrowOffset + arrowWidth);

            topLeft = QPoint(pos.x() - sh.width(), pos.y() - arrowOffset);
        }
        else if (m_arrowDir == LeftBottom) {
            path.lineTo(mr, mb - arrowOffset - arrowWidth);
            path.lineTo(mr + arrowHeight, mb - arrowOffset);
            path.lineTo(mr, mb - arrowOffset);

            topLeft = QPoint(pos.x() - sh.width(), pos.y() - sh.height() + arrowOffset);
        }
        else if (m_arrowDir == LeftCenter) {
            const int y = (sh.height() - arrowWidth) / 2;
            path.lineTo(mr, mt + y);
            path.lineTo(mr + arrowHeight, mt + y + arrowWidth / 2);
            path.lineTo(mr, mt + y + arrowWidth);

            topLeft = QPoint(pos.x() - sh.width(), pos.y() - y - radius);
        }
    }

    path.lineTo(mr, mb - radius);
    path.arcTo(mr - radius * 2, mb - radius * 2, radius * 2, radius * 2, 0, -90);

    // =========================================================
    // BOTTOM SIDE (sageata JOS => balon DEASUPRA punctului)
    // =========================================================
    if (arrowAtBottom) {
        if (m_arrowDir == TopRight) {
            path.lineTo(mr - arrowOffset, mb);
            path.lineTo(mr - arrowOffset, mb + arrowHeight);
            path.lineTo(mr - arrowOffset - arrowWidth, mb);

            topLeft = QPoint(pos.x() - sh.width() + arrowOffset, pos.y() - sh.height());
        }
        else if (m_arrowDir == TopLeft) {
            path.lineTo(ml + arrowOffset + arrowWidth, mb);
            path.lineTo(ml + arrowOffset, mb + arrowHeight);
            path.lineTo(ml + arrowOffset, mb);

            topLeft = QPoint(pos.x() - arrowOffset, pos.y() - sh.height());
        }
        else if (m_arrowDir == TopCenter) {
            const int x = (sh.width() - arrowWidth) / 2;
            path.lineTo(mr - x, mb);
            path.lineTo(mr - x - arrowWidth / 2, mb + arrowHeight);
            path.lineTo(mr - x - arrowWidth, mb);

            topLeft = QPoint(pos.x() - x - radius, pos.y() - sh.height());
        }
    }

    path.lineTo(ml + radius, mb);
    path.arcTo(ml, mb - radius * 2, radius * 2, radius * 2, 270, -90);

    // =========================================================
    // LEFT SIDE (sageata STANGA => balon DREAPTA de punct)
    // =========================================================
    if (arrowAtLeft) {
        if (m_arrowDir == RightBottom) {
            path.lineTo(ml, mb - arrowOffset);
            path.lineTo(ml - arrowHeight, mb - arrowOffset);
            path.lineTo(ml, mb - arrowOffset - arrowWidth);

            topLeft = QPoint(pos.x(), pos.y() - sh.height() + arrowOffset);
        }
        else if (m_arrowDir == RightTop) {
            path.lineTo(ml, mt + arrowOffset + arrowWidth);
            path.lineTo(ml - arrowHeight, mt + arrowOffset);
            path.lineTo(ml, mt + arrowOffset);

            topLeft = QPoint(pos.x(), pos.y() - arrowOffset);
        }
        else if (m_arrowDir == RightCenter) {
            const int y = (sh.height() - arrowWidth) / 2;
            path.lineTo(ml, mb - y);
            path.lineTo(ml - arrowHeight, mb - y - arrowWidth / 2);
            path.lineTo(ml, mb - y - arrowWidth);

            topLeft = QPoint(pos.x(), pos.y() - y - radius);
        }
    }

    path.lineTo(ml, mt + radius);
    path.arcTo(ml, mt, radius * 2, radius * 2, 180, -90);
    path.closeSubpath();

    const QRect screenRect = availableScreenGeometry(pos);
    topLeft = clampToScreen(topLeft, sh, screenRect);
    move(topLeft);

    QBitmap bitmap(sh);
    bitmap.fill(Qt::color0);

    {
        QPainter painter(&bitmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(Qt::color1, border));
        painter.setBrush(QBrush(Qt::color1));
        painter.drawPath(path);
    }

    setMask(bitmap);

    const QColor borderColor = palette().color(QPalette::Window).darker(160);

    m_pixmap = QPixmap(sh);
    m_pixmap.fill(Qt::transparent);

    {
        QPainter painter(&m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(borderColor, border));
        painter.setBrush(palette().color(QPalette::Window));
        painter.drawPath(path);
    }

    resize(sh);
    update();
}

QRect BalloonTip::availableScreenGeometry(const QPoint &pos) const
{
    if (QScreen *screen = QGuiApplication::screenAt(pos))
        return screen->availableGeometry();

    if (QScreen *screen = QGuiApplication::primaryScreen())
        return screen->availableGeometry();

    return QRect(0, 0, 1920, 1080);
}

QPoint BalloonTip::clampToScreen(const QPoint &topLeft,
                                 const QSize &size,
                                 const QRect &screenRect) const
{
    int x = topLeft.x();
    int y = topLeft.y();

    if (x + size.width() > screenRect.left() + screenRect.width())
        x = screenRect.left() + screenRect.width() - size.width();

    if (y + size.height() > screenRect.top() + screenRect.height())
        y = screenRect.top() + screenRect.height() - size.height();

    if (x < screenRect.left())
        x = screenRect.left();

    if (y < screenRect.top())
        y = screenRect.top();

    return QPoint(x, y);
}

//************************************************************
//******* PROTECTED FUNCTION

void BalloonTip::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    close();
}

void BalloonTip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawPixmap(0, 0, m_pixmap);
}

void BalloonTip::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;

        if (underMouse())
            return;

        auto *effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        if (!effect) {
            close();
            return;
        }

        auto *anim = new QPropertyAnimation(effect, "opacity", this);
        anim->setDuration(220);
        anim->setStartValue(effect->opacity());
        anim->setEndValue(0.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        connect(anim, &QPropertyAnimation::finished, this, &QWidget::close);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    QWidget::timerEvent(event);
}
