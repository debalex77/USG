#ifndef VARIANTMAPTABLEMODEL_H
#define VARIANTMAPTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>

//-----------------------------------------------------------------

class AbstractColumn
{
public:
    AbstractColumn(QString name);
    QString name() {return _name;}
    virtual QVariant colData(const QVariantMap &rowData, int role = Qt::DisplayRole) = 0;
private:
    QString _name;
};

//-----------------------------------------------------------------

class SimpleColumn : public AbstractColumn
{
public:
    SimpleColumn(QString name);
    virtual QVariant colData(const QVariantMap &rowData, int role) override;
};

//-----------------------------------------------------------------

class VariantMapTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit VariantMapTableModel(QObject *parent = nullptr);

    bool itIndexAmniotic = false;
    void registerColumn(AbstractColumn* column);
    void addRow(QVariantMap rowData);

    int idByRow(int row) const;
    int colByName(QString name) const;
    QString NameByCol(int col) const;

public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

private:

    enum sections_tab {
        section_id         = 0,
        section_crl        = 1,
        section_5_centile  = 2,
        section_50_centile = 3,
        section_95_centile = 4
    };

    QList<int> _rowIndex;
    QHash<int, QVariantMap> _dataHash;
    QList<AbstractColumn*> _columns;
};

#endif // VARIANTMAPTABLEMODEL_H
