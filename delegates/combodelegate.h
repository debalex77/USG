#ifndef COMBODELEGATE_H
#define COMBODELEGATE_H

#include <QItemDelegate>
#include <QComboBox>
#include <QSqlQuery>

class ComboDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(QString str_qry, QObject *parent = nullptr);

public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void set_data_choice(const int index, const QString text);

private slots:
    void setData(const int val);

private:
//    QMap<int, QString> m_data;
    QString m_str_qry;
    mutable QComboBox *combo;
};

#endif // COMBODELEGATE_H
