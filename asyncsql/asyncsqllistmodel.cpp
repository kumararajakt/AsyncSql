#include "asyncsqllistmodel.h"

using namespace AsyncSql;

AsyncSqlListModel::AsyncSqlListModel(QObject *parent) :
    AsyncSqlBaseModel(parent)
{
}

AsyncSqlListModel::~AsyncSqlListModel()
{
}

Qt::ItemFlags AsyncSqlListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = AsyncSqlBaseModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;

    return theFlags;
}

int AsyncSqlListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant AsyncSqlListModel::data(const QModelIndex &index, int role) const
{
    if (records_.isEmpty())
        return QVariant();
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.count())
        return QVariant();

    if (role >= Qt::UserRole + 1) {
        const QByteArray roleName = roleNames().value(role);
        if (!roleName.isEmpty())
            return record(index.row()).value(fieldIndex(QString::fromUtf8(roleName)));
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int col = (index.column() >= 0 && index.column() < emptyRecord_.count()) ? index.column() : 0;
        return records_.at(index.row()).value(col);
    }

    return QVariant();
}

QHash<int, QByteArray> AsyncSqlListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (int i = 0; i < emptyRecord_.count(); ++i)
        roles[Qt::UserRole + i + 1] = emptyRecord_.fieldName(i).toUtf8();
    return roles;
}

bool AsyncSqlListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= records_.count())
        return false;

    QSqlRecord record = this->record(index.row());
    int fieldCol = index.column();

    if (role >= Qt::UserRole + 1) {
        const QByteArray roleName = roleNames().value(role);
        fieldCol = fieldIndex(QString::fromUtf8(roleName));
    }

    if (fieldCol < 0 || fieldCol >= record.count())
        return false;

    record.setValue(fieldCol, value);
    record.setGenerated(fieldCol, true);
    records_[index.row()] = record;
    updatedRecordMap_.insert(index.row(), record);

    Q_EMIT dataChanged(index, index);
    return true;
}

bool AsyncSqlListModel::insertRows(int row, int count, const QModelIndex &parent)
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

bool AsyncSqlListModel::removeRows(int row, int count, const QModelIndex &parent)
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
