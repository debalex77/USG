#ifndef CUSTOMDIALOGINVESTIG_H
#define CUSTOMDIALOGINVESTIG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDebug>

class CustomDialogInvestig : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool t_organs_internal READ get_t_organs_internal WRITE set_t_organs_internal NOTIFY t_organs_internalChanged)
    Q_PROPERTY(bool t_urinary_system READ get_t_urinary_system WRITE set_t_urinary_system NOTIFY t_urinary_systemChanged)
    Q_PROPERTY(bool t_prostate READ get_t_prostate WRITE set_t_prostate NOTIFY t_prostateChanged)
    Q_PROPERTY(bool t_gynecology READ get_t_gynecology WRITE set_t_gynecology NOTIFY t_gynecologyChanged)
    Q_PROPERTY(bool t_breast READ get_t_breast WRITE set_t_breast NOTIFY t_breastChanged)
    Q_PROPERTY(bool t_thyroide READ get_t_thyroide WRITE set_t_thyroide NOTIFY t_thyroideChanged)
    Q_PROPERTY(bool t_gestation0 READ get_t_gestation0 WRITE set_t_gestation0 NOTIFY t_gestation0Changed)
    Q_PROPERTY(bool t_gestation1 READ get_t_gestation1 WRITE set_t_gestation1 NOTIFY t_gestation1Changed)
    Q_PROPERTY(bool t_gestation2 READ get_t_gestation2 WRITE set_t_gestation2 NOTIFY t_gestation2Changed)

public:
    CustomDialogInvestig(QWidget *parent = nullptr);

    void set_t_organs_internal(bool t_organs_internal) {m_organs_internal = t_organs_internal; emit t_organs_internalChanged();}
    bool get_t_organs_internal() const {return m_organs_internal;}

    void set_t_urinary_system(bool t_urinary_system) {m_urinary_system = t_urinary_system; emit t_urinary_systemChanged();}
    bool get_t_urinary_system() const {return m_urinary_system;}

    void set_t_prostate(bool t_prostate) {m_prostate = t_prostate; emit t_prostateChanged();}
    bool get_t_prostate() const {return m_prostate;}

    void set_t_gynecology(bool t_gynecology) {m_gynecology = t_gynecology; emit t_gynecologyChanged();}
    bool get_t_gynecology() const {return m_gynecology;}

    void set_t_breast(bool t_breast) {m_breast = t_breast; emit t_breastChanged();}
    bool get_t_breast() const {return m_breast;}

    void set_t_thyroide(bool t_thyroide) {m_thyroide = t_thyroide; emit t_thyroideChanged();}
    bool get_t_thyroide() const {return m_thyroide;}

    void set_t_gestation0(bool t_gestation0) {m_gestation0 = t_gestation0; emit t_gestation0Changed();}
    bool get_t_gestation0() const {return m_gestation0;}

    void set_t_gestation1(bool t_gestation1) {m_gestation1 = t_gestation1; emit t_gestation1Changed();}
    bool get_t_gestation1() const {return m_gestation1;}

    void set_t_gestation2(bool t_gestation2) {m_gestation2 = t_gestation2; emit t_gestation2Changed();}
    bool get_t_gestation2() const {return m_gestation2;}

signals:
    void t_organs_internalChanged();
    void t_urinary_systemChanged();
    void t_prostateChanged();
    void t_gynecologyChanged();
    void t_breastChanged();
    void t_thyroideChanged();
    void t_gestation0Changed();
    void t_gestation1Changed();
    void t_gestation2Changed();

public slots:
    void highlightChecked(QListWidgetItem* item);
    void save();
    void setListWidget();

private:
    enum tag_investig
    {
        t_organsInternal = 0,
        t_urinarySystem  = 1,
        t_prostate       = 2,
        t_gynecology     = 3,
        t_breast         = 4,
        t_thyroide       = 5,
        t_gestation0     = 6,
        t_gestation1     = 7,
        t_gestation2     = 8
    };
    void createListWidget();
    void createOtherWidgets();
    void createLayout();
    void createConnections();

private:

    bool m_organs_internal = false;
    bool m_urinary_system  = false;
    bool m_prostate   = false;
    bool m_thyroide   = false;
    bool m_gynecology = false;
    bool m_breast     = false;
    bool m_gestation0 = false;
    bool m_gestation1 = false;
    bool m_gestation2 = false;

    QListWidget* widget;
    QGroupBox*   viewBox;
    QPushButton* btnOK;
    QPushButton* btnClose;
    QDialogButtonBox* buttonBox;
};

#endif // CUSTOMDIALOGINVESTIG_H
