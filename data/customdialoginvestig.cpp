#include "customdialoginvestig.h"
#include <QGuiApplication>
#include <QToolButton>
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QMessageBox>
#include <QScreen>
#endif

CustomDialogInvestig::CustomDialogInvestig(QWidget *parent):
    QDialog(parent)
{
    setWindowTitle(tr("Alege investigațiile"));

    createListWidget();
    createOtherWidgets();
    createLayout();
    createConnections();

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    int x = (screenWidth / 2) - (318 / 2);
    int y = (screenHeight / 2) - (302 / 2);
    move(x, y);
}

void CustomDialogInvestig::highlightChecked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked)
        item->setBackground(QColor(255,255,178));
    else
        item->setBackground(QColor(255,255,255));
}

void CustomDialogInvestig::save()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        bool isChecked = (item->checkState() == Qt::Checked) ? true : false;

        switch (i) {
        case t_organsInternal:
            m_organs_internal = isChecked;
            break;
        case t_urinarySystem:
            m_urinary_system = isChecked;
            break;
        case t_prostate:
            m_prostate = isChecked;
            break;
        case t_thyroide:
            m_thyroide = isChecked;
            break;
        case t_breast:
            m_breast = isChecked;
            break;
        case t_gynecology:
            m_gynecology = isChecked;
            break;
        case t_gestation0:
            m_gestation0 = isChecked;
            break;
        case t_gestation1:
            m_gestation1 = isChecked;
            break;
        case t_gestation2:
            m_gestation2 = isChecked;
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
    if (m_organs_internal == false &&
        m_urinary_system == false &&
        m_prostate == false &&
        m_thyroide == false &&
        m_breast == false &&
        m_gynecology == false &&
        m_gestation0 == false &&
        m_gestation1 == false &&
        m_gestation2 == false){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Selectarea sistemului"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Nu este selectat nici un sistem !!!<br> Bifati sistemul necesar."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    QDialog::accept();
}

void CustomDialogInvestig::createListWidget()
{
    widget = new QListWidget;
    QStringList strList;
    strList << tr("organe interne")
            << tr("sistemul urinar")
            << tr("prostata")
            << tr("ginecologia")
            << tr("gl.mamare")
            << tr("tiroida")
            << tr("sarcina până la 11 săptămâni")
            << tr("sarcina 11-14 săptămâni")
            << tr("sarcina 15-40 săptămâni");

    widget->addItems(strList);

    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void CustomDialogInvestig::createOtherWidgets()
{
    viewBox   = new QGroupBox(tr("Lista investigațiilor"));
    buttonBox = new QDialogButtonBox;
    btnOK    = buttonBox->addButton(QDialogButtonBox::Ok);
    btnClose = buttonBox->addButton(QDialogButtonBox::Close);
}

void CustomDialogInvestig::createLayout()
{
    QToolButton *btnCheck          = new QToolButton(this);
    QToolButton *btnUncheck        = new QToolButton(this);
    btnCheck->setIcon(QIcon(":/img/checked_checkbox.png"));
    btnCheck->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnUncheck->setIcon(QIcon(":/img/unchecked_checkbox.png"));
    btnUncheck->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QSpacerItem *itemSpacer = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* horizontalLayout_btn = new QHBoxLayout;
    horizontalLayout_btn->addItem(itemSpacer);
    horizontalLayout_btn->addWidget(btnCheck);
    horizontalLayout_btn->addWidget(btnUncheck);

    QVBoxLayout* viewLayout = new QVBoxLayout;
    viewLayout->addLayout(horizontalLayout_btn);
    viewLayout->addWidget(widget);
    viewBox->setLayout(viewLayout);

    QHBoxLayout* horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(buttonBox);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(viewBox);
    mainLayout->addLayout(horizontalLayout);

    setLayout(mainLayout);

    connect(btnCheck, &QAbstractButton::clicked, this, [this]()
    {
        QListWidgetItem* item = 0;
        for(int i = 0; i < widget->count(); ++i){
            item = widget->item(i);
            switch (i) {
            case t_organsInternal:
                item->setCheckState(Qt::Checked);
                break;
            case t_urinarySystem:
                item->setCheckState(Qt::Checked);
                break;
            case t_prostate:
                item->setCheckState(Qt::Checked);
                break;
            case t_thyroide:
                item->setCheckState(Qt::Checked);
                break;
            case t_breast:
                item->setCheckState(Qt::Checked);
                break;
            case t_gynecology:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation0:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation1:
                item->setCheckState(Qt::Checked);
                break;
            case t_gestation2:
                item->setCheckState(Qt::Checked);
                break;
            default:
                qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
            }
        }
    });

    connect(btnUncheck, &QAbstractButton::clicked, this, [this]()
    {
        QListWidgetItem* item = 0;
        for(int i = 0; i < widget->count(); ++i){
            item = widget->item(i);
            switch (i) {
            case t_organsInternal:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_urinarySystem:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_prostate:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_thyroide:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_breast:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gynecology:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation0:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation1:
                item->setCheckState(Qt::Unchecked);
                break;
            case t_gestation2:
                item->setCheckState(Qt::Unchecked);
                break;
            default:
                qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
            }
        }
    });
}

void CustomDialogInvestig::createConnections()
{
    connect(this, &CustomDialogInvestig::t_organs_internalChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_urinary_systemChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_prostateChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gynecologyChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_breastChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_thyroideChanged, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation0Changed, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation1Changed, this, &CustomDialogInvestig::setListWidget);
    connect(this, &CustomDialogInvestig::t_gestation2Changed, this, &CustomDialogInvestig::setListWidget);

    connect(btnOK, &QPushButton::clicked, this, &CustomDialogInvestig::save);
    connect(btnClose, &QPushButton::clicked, this, &CustomDialogInvestig::close);
}

void CustomDialogInvestig::setListWidget()
{
    QListWidgetItem* item = 0;
    for(int i = 0; i < widget->count(); ++i){
        item = widget->item(i);
        switch (i) {
        case t_organsInternal:
            item->setCheckState((m_organs_internal) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_urinarySystem:
            item->setCheckState((m_urinary_system) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_prostate:
            item->setCheckState((m_prostate) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_thyroide:
            item->setCheckState((m_thyroide) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_breast:
            item->setCheckState((m_breast) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gynecology:
            item->setCheckState((m_gynecology) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation0:
            item->setCheckState((m_gestation0) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation1:
            item->setCheckState((m_gestation1) ? Qt::Checked : Qt::Unchecked);
            break;
        case t_gestation2:
            item->setCheckState((m_gestation2) ? Qt::Checked : Qt::Unchecked);
            break;
        default:
            qDebug() << tr("Nu a fost determinat indexul din clasa '%1' !!!").arg(metaObject()->className());
        }
    }
}
