#ifndef REGISTRATIONTABLEMODEL_H
#define REGISTRATIONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QLocale>
#include <QTime>

class RegistrationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RegistrationTableModel(int rows, int cols, QObject *parent = nullptr);

signals:
    void m_data_changed();

private:
    int row_count = 0;
    int col_count = 0;

protected:
    QHash<QModelIndex, QVariant> cell_data;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
};

// **********************************************************************************

class RegistrationPatientsModel : public RegistrationTableModel
{
    Q_OBJECT

public:
    explicit RegistrationPatientsModel(int rows, int cols, QObject *parent = nullptr);
    void setDataPatient(int time_num, int column_num, QString _data);


    // QAbstractItemModel interface
public:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    enum sections_tab
    {
        section_id            = 0,
        section_execute       = 1,
        section_data_patient  = 2,
        section_investigation = 3,
        section_organization  = 4,
        section_doctor        = 5,
        section_comment       = 6
    };

    QTime init_time = QTime(8,00,00);

};

#endif // REGISTRATIONTABLEMODEL_H
