#include "basesqlquerymodel.h"
#include <QIcon>
#include <QFont>
#include <QDate>
#include <QBrush>
#include <QStyle>
#include <QDebug>
#include <data/globals.h>

namespace
{
    enum Columns // Depends with 'query.prepare( QString( "SELECT ... '
    {
        Id,
        Data,
    };
}

QVariant BaseSqlQueryModel::dataFromParent(QModelIndex index, int column) const
{
    if (strcmp(parent()->metaObject()->className(), "QComboBox") == 0){
        return QSqlQueryModel::data(QSqlQueryModel::index(index.row() - 1, column)); // din cauza primului rand (selecteaza)
    } else {
        return QSqlQueryModel::data(QSqlQueryModel::index(index.row(), column));
    }
}

BaseSqlQueryModel::BaseSqlQueryModel(QString &strQuery, QObject *parent): QSqlQueryModel(parent)
{
    if (strQuery.isEmpty()){
        return;
    }
    QSqlQuery query(QSqlDatabase::database("MainConnection"));
    query.prepare(strQuery);
    if (!query.exec()){
        qDebug() << tr("Nu s-a reusit de executat solicitarea. Eroarea: ") + query.lastError().text();
    }
    QSqlQueryModel::setQuery(std::move(query)); // setam solicitarea in model
}

int BaseSqlQueryModel::rowCount(const QModelIndex &parent) const
{
    if (strcmp(this->parent()->metaObject()->className(), "QComboBox") == 0){
        return QSqlQueryModel::rowCount(parent) + 1; // adaugam randul gol (selecteaza)
    } else {
        return QSqlQueryModel::rowCount(parent);
    }
}

int BaseSqlQueryModel::columnCount(const QModelIndex &parent) const
{
    return QSqlQueryModel::columnCount(parent);
}

QVariant BaseSqlQueryModel::data(const QModelIndex &item, int role) const
{
    if (parent()->inherits("QComboBox"))
        return dataFromComboBox(item, role);
    else if (parent()->inherits("CatOrganizations"))
        return dataFromCatOrganizations(item, role);
    else if (parent()->parent()->inherits("ListDoc"))
        return dataFromListDoc(item, role);
    else if (parent()->inherits("DocPricing"))
        return dataFromDocPricing(item, role);
    else if (parent()->parent()->inherits("ListDocWebOrder"))
        return dataFromDocOrderEcho(item, role);
    else if (parent()->inherits("view_table"))
        return dataFrom_view_table_order(item, role);

    switch (m_modelParent) {
    case UserSettings:
        return dataFromUserSettings(item, role);
    case GeneralListForm:
        return dataFromGeneralListForm(item, role);
    case ListOrderReport:
        return dataFromDocOrderEcho(item, role);
    case ViewTableOrder:
        return dataFrom_view_table_order(item, role);
    case ViewTableReport:
        return QSqlQueryModel::data(item, role);
    default:
        return QSqlQueryModel::data(item, role);
    }
}

bool BaseSqlQueryModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return QSqlQueryModel::setHeaderData(section, orientation, value, role);
}

void BaseSqlQueryModel::setIdContract(const int _id)
{
    id_contract = _id;
}

QVariant BaseSqlQueryModel::dataFromComboBox(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);

    switch (role) {
    case Qt::UserRole:
        if (item.row() == 0)
            return 0;
        else
            return dataFromParent(item, Id);
    case Qt::DisplayRole:
        if (item.row() == 0)
            return tr("<<- Selectează ->>");
        else
            return dataFromParent(item, Data);
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFromUserSettings(const QModelIndex &item, int role) const
{
#if defined(Q_OS_WIN)
    QFont font;
    font.setPointSize(9);
#endif
    switch (role) {
    case Qt::UserRole:
        return dataFromParent(item, Id);
    case Qt::DisplayRole:
        return dataFromParent(item, Data);
    case Qt::FontRole:
#if defined(Q_OS_LINUX)
        return QSqlQueryModel::data(item, role);
#elif defined(Q_OS_WIN)
        return font;
#endif
    default:
        return QSqlQueryModel::data(item, role);
    }
}

QVariant BaseSqlQueryModel::dataFromGeneralListForm(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);

    switch (role) {
    case Qt::DisplayRole:
        if (item.column() == 1)
            return "";
        else if (m_typeCatalogs == Pacients && item.column() == 3)
            return value.toDate().toString("dd.MM.yyyy");
        return value;
    case Qt::EditRole:
        return value;
    case Qt::DecorationRole:
        if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/element_x32.png");
        else if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/element_delete_x32.png");
        else
            return value;
    case Qt::TextAlignmentRole:
        return value;
    case Qt::FontRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlQueryModel::data(item, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }
    case Qt::BackgroundRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));
        else
            return value;
    case Qt::ToolTipRole:
        return QString(QSqlQueryModel::data(QSqlQueryModel::index(item.row(), item.column())).toString());
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFromCatOrganizations(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);

    switch (role) {
    case Qt::DisplayRole:
        if (item.column() == 1)
            return "";
        else if (item.column() == 2)
            return value.toDate().toString("dd.MM.yyyy");
        return value;
    case Qt::EditRole:
        return value;
    case Qt::DecorationRole:
        if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/element_x32.png");
        else if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/element_delete_x32.png");
        else
            return value;
    case Qt::TextAlignmentRole:
        return value;
    case Qt::FontRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlQueryModel::data(item, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }
    case Qt::BackgroundRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 0), Qt::DisplayRole).toInt() == id_contract)
            return QBrush(QColor(194,250,255));
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));
        else
            return value;
    case Qt::ToolTipRole:
        return QString(QSqlQueryModel::data(QSqlQueryModel::index(item.row(), item.column())).toString());
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFromListDoc(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);

    switch (role) {
    case Qt::DisplayRole:
        if (item.column() == 1)
            return "";
        else if (item.column() == 3)
            return value.toDateTime().toString("dd.MM.yyyy hh:mm:ss");
        return value;
    case Qt::EditRole:
        return value;
    case Qt::DecorationRole:
        if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/document_write.png");
        else if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/document_remove.png");
        else if (item.column() == 1 && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 2)
            return QIcon(":img/document_accept.png");
        else
            return value;
    case Qt::TextAlignmentRole:
        if (item.column() == 2)
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        return value;
    case Qt::FontRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlQueryModel::data(item, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }
    case Qt::BackgroundRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 0), Qt::DisplayRole).toInt() == id_contract)
            return QBrush(QColor(194,250,255));
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), 1), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));
        else
            return value;
    case Qt::ToolTipRole:
        return QString(QSqlQueryModel::data(QSqlQueryModel::index(item.row(), item.column())).toString());
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFromDocPricing(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);
    QFont font;

    switch (role) {
    case Qt::DisplayRole:
        return value;
    case Qt::EditRole:
        return value;
    case Qt::FontRole:
#if defined(Q_OS_LINUX)
        Q_UNUSED(font)
        return value;
#elif defined(Q_OS_WIN)
        font.setPointSize(9);
        return font;
#endif
    case Qt::TextAlignmentRole:
        if (item.column() == 3) // sectia 'Cod'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        else if (item.column() == 5) // sectia 'Pret'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        return value;
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFromDocOrderEcho(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);

    switch (role) {
    case Qt::DisplayRole:
        if (item.column() == orderSection_deletionMark)
            return ""; // value;
        else if (item.column() == orderSection_attachedImages)
            return "";
        else if (item.column() == orderSection_cardPayment)
            return "";
        else if (item.column() == orderSection_dateDoc)
            return value.toDateTime().toString("dd.MM.yyyy hh:mm:ss");
        if (item.column() == orderSection_sum){
            double _num = value.toDouble();
            return QVariant(QString("%1").arg(_num, 0, 'f', 2));
        }
        return value;
    case Qt::UserRole:
        return value;
    case Qt::EditRole:
        return value;
    case Qt::DecorationRole:
        if (item.column() == orderSection_deletionMark && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/document_write.png");
        else if (item.column() == orderSection_deletionMark && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/document_remove.png");
        else if (item.column() == orderSection_deletionMark && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 2)
            return QIcon(":img/document_accept.png");
        else if (item.column() == orderSection_attachedImages && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon();
        else if (item.column() == orderSection_attachedImages && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/image-files.png");
        else if (item.column() == orderSection_attachedImages && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 2)
            return QIcon(":img/video.png");
        else if (item.column() == orderSection_cardPayment && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 0)
            return QIcon(":img/payment_cash.png");
        else if (item.column() == orderSection_cardPayment && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 1)
            return QIcon(":img/master_card.png");
        else if (item.column() == orderSection_cardPayment && QSqlQueryModel::data(item, Qt::DisplayRole).toInt() == 2)
            return QIcon(":img/payment_bank.png");
        else
            return value;
    case Qt::TextAlignmentRole:
        if(item.column() == orderSection_numberDoc ||
                item.column() == orderSection_sum ||
                item.column() == orderSection_user){
            return Qt::AlignHCenter;
        }
        return value;
    case Qt::FontRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), orderSection_deletionMark), Qt::DisplayRole).toInt() == 1){
            QFont font = QSqlQueryModel::data(item, Qt::FontRole).value<QFont>();  // traversarea textului
            font.setItalic(true);
            font.setStrikeOut(true);
            return font;
        } else {
#if defined(Q_OS_LINUX)
            return value;
#elif defined(Q_OS_WIN)
            QFont font;
            font.setPointSize(9);
            return font;
#endif
        }
    case Qt::BackgroundRole:
        if (QSqlQueryModel::data(QSqlQueryModel::index(item.row(), orderSection_deletionMark), Qt::DisplayRole).toInt() == 1) // DeletionMark - marked
            return QBrush(QColor(255,230,255));
        else
            return value;
    case Qt::ToolTipRole:
        return QString(QSqlQueryModel::data(QSqlQueryModel::index(item.row(), item.column())).toString());
    default:
        return value;
    }
}

QVariant BaseSqlQueryModel::dataFrom_view_table_order(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item, role);
    QFont font;

    switch (role) {
    case Qt::DisplayRole:
        if (item.column() == 5){
            double _num = value.toDouble();
            return QVariant(QString("%1").arg(_num, 0, 'f', 2));
        }
        return value;
    case Qt::EditRole:
        return value;
    case Qt::TextAlignmentRole:
        if (item.column() == 3) // sectia 'Cod'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        else if (item.column() == 5) // sectia 'Pret'
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
        return value;
    case Qt::FontRole:
#if defined(Q_OS_WIN)
        font.setPointSize(9);
        return font;
#elif defined(Q_OS_LINUX)
        Q_UNUSED(font)
        return value;
#endif
    default:
        return value;
    }
}
