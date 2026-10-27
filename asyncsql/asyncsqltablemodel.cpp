#include "asyncsqltablemodel.h"

using namespace AsyncSql;

AsyncSqlTableModel::AsyncSqlTableModel(QObject *parent) :
    AsyncSqlBaseModel(parent)
{
}

AsyncSqlTableModel::~AsyncSqlTableModel()
{
}

Qt::ItemFlags AsyncSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = AsyncSqlBaseModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;

    return theFlags;
}

QVariant AsyncSqlTableModel::data(const QModelIndex &index, int role) const
{
    if (records_.isEmpty())
        return QVariant();
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.count()
            || index.column() < 0 || index.column() >= records_.first().count())
        return QVariant();

    if (role >= Qt::UserRole + 1)
        return record(index.row()).value(fieldIndex(QString::fromUtf8(roleNames()[role])));
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return records_.at(index.row()).value(index.column());

    return QVariant();
}

QHash<int, QByteArray> AsyncSqlTableModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (int i = 0; i < emptyRecord_.count(); ++i)
        roles[Qt::UserRole + i + 1] = emptyRecord_.fieldName(i).toUtf8();
    return roles;
}

QVariant AsyncSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
            return emptyRecord_.fieldName(section);
    }
    return QVariant();
}

int AsyncSqlTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return emptyRecord_.count();
}

bool AsyncSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role != Qt::EditRole)
        return false;

    if (records_.isEmpty())
        return false;
    if (index.row() >= records_.count())
        return false;

    QSqlRecord record = this->record(index.row());
    if (index.column() >= record.count())
        return false;

    record.setValue(index.column(), value);
    records_[index.row()] = record;
    updatedRecordMap_.insert(index.row(), record);

    Q_EMIT dataChanged(index, index);
    return true;
}

bool AsyncSqlTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || role != Qt::EditRole)
        return false;

    emptyRecord_.setValue(section, value);
    Q_EMIT headerDataChanged(orientation, section, section);
    return true;
}

bool AsyncSqlTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || count <= 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        records_.insert(row + i, emptyRecord_);
        insertedRows_.append(row + i);
    }
    endInsertRows();

    return true;
}

bool AsyncSqlTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row + count > records_.count() || count <= 0)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        removedRows_.append(row + i);
    }
    endRemoveRows();

    return true;
}

QModelIndexList AsyncSqlTableModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          Qt::MatchFlags flags) const
{
    switch (value.typeId()) {
    case QMetaType::Int:
    case QMetaType::Double:
    {
        int count = 0;
        QModelIndexList foundIndexes;
        for (int i = 0; i < rowCount(); ++i) {
            QModelIndex cell = index(i, start.column());

            if (cell.data(Qt::EditRole).toDouble() == value.toDouble()) {
                foundIndexes << cell;
                count++;
            }

            if (count == hits)
                break;
        }

        return foundIndexes;
    }
    default:
        break;
    }

    return AsyncSqlBaseModel::match(start, role, value, hits, flags);
}
