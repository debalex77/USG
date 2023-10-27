#ifndef MDIAREACONTAINER_H
#define MDIAREACONTAINER_H

#include <QObject>
#include <QMdiArea>

class MdiAreaContainer : public QObject
{
    Q_OBJECT
public:
    explicit MdiAreaContainer(QMdiArea *widget, QObject *parent = nullptr);

    virtual int count() const;
    virtual int currentIndex() const;
    virtual QWidget *widget(int index) const;
    virtual void setCurrentIndex(int index);
    virtual void addWidget(QWidget *widget);
    virtual void remove(int index);

private:
    QMdiArea *m_mdiArea;
};

#endif // MDIAREACONTAINER_H
