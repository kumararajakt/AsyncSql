#include "asyncsqllistmodel.h"
#include "queryrequest.h"
#include "queryresult.h"
#include "querythread.h"
#include <QSqlDatabase>
#include <algorithm>

using namespace AsyncSql;

AsyncSqlListModel::AsyncSqlListModel(QObject *parent) :
    limit_(-1),
    sortColumn_(-1),
    order_(Qt::AscendingOrder),
    selectedSignalSuppressed_(false),
    error_(QSqlError()),
    submitCalled_(false),
    currentRow_(-1),
    foreignKeyFlag_(true),
    initDone_(false),
    busy_(false),
    QAbstractListModel(parent)
{
    connect(this, SIGNAL(execute(const QueryRequest &)),
            &QueryThread::instance(), SLOT(execute(const QueryRequest &)));
    connect(&QueryThread::instance(), SIGNAL(queryFinished(const QueryResult &)),
            this, SLOT(getResults(const QueryResult &)));
}

AsyncSqlListModel::~AsyncSqlListModel()
{
}

Qt::ItemFlags AsyncSqlListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractListModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    return theFlags;
}

QVariant AsyncSqlListModel::data(const QModelIndex &index, int role) const
{
    if (records_.isEmpty())
        return QVariant();
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.count())
        return QVariant();

    if (role >= Qt::UserRole + 1)
        return record(index.row()).value(fieldIndex(roleNames()[role]));
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return records_.at(index.row()).value(0);

    return QVariant();
}

QHash<int, QByteArray> AsyncSqlListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (int i = 0; i < emptyRecord_.count(); ++i)
        roles[Qt::UserRole + 1 + i] = emptyRecord_.fieldName(i).toUtf8();
    return roles;
}

int AsyncSqlListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : records_.count();
}

bool AsyncSqlListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (records_.isEmpty())
        return false;
    if (!index.isValid() || role != Qt::EditRole || index.row() < 0 || index.row() >= records_.count()
            || index.column() < 0 || index.column() >= records_.first().count())
        return false;

    QSqlRecord rec = records_[index.row()];
    rec.setValue(index.column(), value);
    records_[index.row()] = rec;

    if (!insertedRows_.contains(index.row()) && !removedRows_.contains(index.row()) &&
            !updatedRecordMap_.contains(index.row()))
    {
        QSqlRecord updated(emptyRecord_);
        for (int i = 0; i < updated.count(); ++i)
            updated.setGenerated(i, false);
        updated.setValue(primaryIndex_.fieldName(0),
                         index.sibling(index.row(), updated.indexOf(primaryIndex_.fieldName(0))).data(Qt::EditRole));
        updated.setValue(index.column(), value);
        updated.setGenerated(index.column(), true);
        updatedRecordMap_.insert(index.row(), updated);
    }
    else if (!insertedRows_.contains(index.row()) && !removedRows_.contains(index.row()) &&
             updatedRecordMap_.contains(index.row()))
    {
        QSqlRecord updated(updatedRecordMap_.value(index.row()));
        updated.setValue(primaryIndex_.fieldName(0),
                         this->index(index.row(), updated.indexOf(primaryIndex_.fieldName(0))).data(Qt::EditRole));
        updated.setValue(index.column(), value);
        updated.setGenerated(index.column(), true);
        updatedRecordMap_.insert(index.row(), updated);
    }

    emit dataChanged(index, index);
    return true;
}

bool AsyncSqlListModel::insertRows(int row, int count, const QModelIndex &)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        records_.insert(row, emptyRecord_);
        if (!insertedRows_.contains(row))
            insertedRows_ << row;
    }
    endInsertRows();
    return true;
}

bool AsyncSqlListModel::removeRows(int row, int count, const QModelIndex &)
{
    if ((row < 0) || (row >= records_.count()) || records_.isEmpty())
        return false;

    if (originalRecords_.contains(records_.at(row))) {
        if (!removedRows_.contains(row))
            removedRows_ << row;
        insertedRows_.removeAll(row);
        return true;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        records_.removeAt(row);
    endRemoveRows();

    insertedRows_.removeAll(row);
    return true;
}

void AsyncSqlListModel::select()
{
    QString query = selectQuery_.trimmed().isEmpty()
        ? QString("SELECT * FROM %1").arg(tableName_)
        : selectQuery_.trimmed();

    if (!filter_.trimmed().isEmpty())
        query += " WHERE " + filter_.trimmed();

    if (sortColumn_ >= 0) {
        switch (order_) {
        case Qt::AscendingOrder:
            query += QString(" ORDER BY %1");
            break;
        case Qt::DescendingOrder:
            query += QString(" ORDER BY %1 DESC");
            break;
        default:
            break;
        }
    }

    if (limit_ >= 0)
        query += " LIMIT " + QString::number(limit_);

    if (records_.count())
        removeRows(0, records_.count());
    records_.clear();
    originalRecords_.clear();
    insertedRows_.clear();
    removedRows_.clear();
    updatedRecordMap_.clear();
    originalInsertedRows_.clear();
    originalRemovedRows_.clear();
    originalUpdatedRecordMap_.clear();
    error_ = QSqlError();

    setBusy(true);

    QueryRequest request(this, query, tableName_, QueryRequest::Select);
    request.setSortColumn(sortColumn_);

    if (!initDone_) {
        initDone_ = true;
        request.setRunBefore([this](QSqlDatabase db){ onInit(db); });
    }

    emit execute(request);
}

bool AsyncSqlListModel::getResults(const QueryResult &result)
{
    if (static_cast<AsyncSqlListModel *>(result.getReceiver()) != this)
        return false;

    setBusy(false);

    if (error_.isValid())
        return false;

    if (result.getError().isValid()) {
        error_ = result.getError();

        if (result.getRequestType() == QueryRequest::Select)
            emit selected(false);
        else if (result.getRequestType() == QueryRequest::CustomOperation)
            emit executed(false);
        else
            emit submitted(false);

        submitCalled_ = false;
        return false;
    }

    switch (result.getRequestType()) {
    case QueryRequest::Select:
    {
        beginResetModel();
        records_ = result.getRecords();
        originalRecords_ = result.getRecords();
        primaryIndex_ = result.getPrimaryIndex();
        lastRecord_ = result.getLastRecord();
        emptyRecord_ = result.getRecord();

        if (!selectedSignalSuppressed_)
            emit selected(true);
        endResetModel();
    }
        break;
    case QueryRequest::Insert:
        insertedRows_.clear();
        break;
    case QueryRequest::Update:
        updatedRecordMap_.clear();
        break;
    case QueryRequest::Delete:
        removedRows_.clear();
        break;
    case QueryRequest::CustomOperation:
        emit executed(true);
        break;
    case QueryRequest::BeginTransaction:
    case QueryRequest::CommitTransaction:
    case QueryRequest::Command:
    case QueryRequest::None:
    default:
        break;
    }

    if (!insertedRows_.count() && !updatedRecordMap_.count() && !removedRows_.count() && submitCalled_) {
        submitCalled_ = false;
        emit submitted(true);
    }

    return true;
}

void AsyncSqlListModel::setFilter(const QString &filter)
{
    filter_ = filter;
}

QString AsyncSqlListModel::filter() const
{
    return filter_;
}

void AsyncSqlListModel::setSort(int column, Qt::SortOrder order)
{
    sortColumn_ = column;
    order_ = order;
}

void AsyncSqlListModel::setTable(const QString &tableName)
{
    tableName_ = tableName;
}

QString AsyncSqlListModel::tableName() const
{
    return tableName_;
}

void AsyncSqlListModel::setSelectQuery(const QString &query)
{
    selectQuery_ = query;
}

QString AsyncSqlListModel::selectQuery() const
{
    return selectQuery_;
}

bool AsyncSqlListModel::isDirty() const
{
    return !insertedRows_.isEmpty() || !updatedRecordMap_.isEmpty() || !removedRows_.isEmpty();
}

bool AsyncSqlListModel::isDirty(const QModelIndex &index) const
{
    return insertedRows_.contains(index.row())
        || !updatedRecordMap_.value(index.row()).field(index.column()).isGenerated()
        || removedRows_.contains(index.row());
}

QList<QSqlRecord> AsyncSqlListModel::insertedRecords() const
{
    QList<QSqlRecord> records;
    for (auto &row : insertedRows_)
        records << records_.at(row);
    return records;
}

QMap<int, QSqlRecord> AsyncSqlListModel::updatedRecords() const
{
    return updatedRecordMap_;
}

QList<QSqlRecord> AsyncSqlListModel::removedRecords() const
{
    QList<QSqlRecord> records;
    for (auto &row : removedRows_)
        records << records_.at(row);
    return records;
}

void AsyncSqlListModel::setLastError(const QSqlError &e)
{
    error_ = e;
}

QSqlError AsyncSqlListModel::lastError() const
{
    return error_;
}

void AsyncSqlListModel::setForeignKeyFlag(bool flag)
{
    foreignKeyFlag_ = flag;
}

bool AsyncSqlListModel::foreignKeyFlag() const
{
    return foreignKeyFlag_;
}

void AsyncSqlListModel::setCurrentRow(int row)
{
    if (currentRow_ == row)
        return;
    currentRow_ = row;
    emit currentRowChanged(row);
}

int AsyncSqlListModel::currentRow() const
{
    return currentRow_;
}

QVariant AsyncSqlListModel::field(const QString &columnName) const
{
    return data(index(currentRow_, 0), Qt::UserRole + 1 + fieldIndex(columnName));
}

void AsyncSqlListModel::beginTransaction()
{
    QueryRequest request(this, QString(), tableName_, QueryRequest::BeginTransaction);
    emit execute(request);
}

void AsyncSqlListModel::commitTransaction()
{
    QueryRequest request(this, QString(), tableName_, QueryRequest::CommitTransaction);
    emit execute(request);
}

bool AsyncSqlListModel::validateModel()
{
    return true;
}

void AsyncSqlListModel::submitAll()
{
    if (insertedRows_.isEmpty() && updatedRecordMap_.isEmpty() && removedRows_.isEmpty()) {
        emit submitted(true);
        return;
    }
    if (!validateModel())
        return;
    if (submitCalled_)
        return;

    originalUpdatedRecordMap_ = updatedRecordMap_;
    originalInsertedRows_ = insertedRows_;
    originalRemovedRows_ = removedRows_;

    error_ = QSqlError();

    QueryRequest request(this);
    request.setTableName(tableName_);
    request.setPrimaryIndex(primaryIndex_);
    QList<QSqlRecord> records;

    if (!updatedRecordMap_.isEmpty()) {
        request.setRequestType(QueryRequest::Update);
        records = updatedRecordMap_.values();
        request.setRecords(records);
        emit execute(request);
    }

    if (!insertedRows_.isEmpty()) {
        request.setRequestType(QueryRequest::Insert);
        records.clear();
        for (int i = 0; i < insertedRows_.count(); ++i)
            records << records_.at(insertedRows_.at(i));
        request.setRecords(records);
        emit execute(request);
        records.clear();
    }

    if (!removedRows_.isEmpty()) {
        request.setRequestType(QueryRequest::Delete);
        for (int i = 0; i < removedRows_.count(); ++i)
            records << records_.at(removedRows_.at(i));
        request.setRecords(records);
        emit execute(request);
        records.clear();
    }

    submitCalled_ = true;
}

void AsyncSqlListModel::revertAll()
{
    beginResetModel();
    records_ = originalRecords_;
    insertedRows_.clear();
    updatedRecordMap_.clear();
    removedRows_.clear();
    endResetModel();
}

QSqlRecord AsyncSqlListModel::record() const
{
    return emptyRecord_;
}

QSqlRecord AsyncSqlListModel::record(int row) const
{
    if ((row < 0) || (row >= records_.count()))
        return emptyRecord_;
    return records_.at(row);
}

bool AsyncSqlListModel::appendRecord(const QSqlRecord &record)
{
    if (record.count() != this->record().count())
        return false;

    bool inserted = insertRow(rowCount());
    int row = rowCount() - 1;

    for (int i = 0; i < record.count(); ++i) {
        QModelIndex cell = index(row, 0);
        setData(cell, record.value(i), Qt::UserRole + 1 + i);
    }

    return inserted;
}

void AsyncSqlListModel::setSelectedSignalSuppressed(bool s)
{
    selectedSignalSuppressed_ = s;
}

bool AsyncSqlListModel::isSelectedSignalSuppressed() const
{
    return selectedSignalSuppressed_;
}

void AsyncSqlListModel::setRecords(const QList<QSqlRecord> &records)
{
    records_ = records;
}

void AsyncSqlListModel::setOriginalRecords(const QList<QSqlRecord> &records)
{
    originalRecords_ = records;
}

QList<QSqlRecord> AsyncSqlListModel::records() const
{
    return records_;
}

QList<QSqlRecord> AsyncSqlListModel::originalRecords() const
{
    return originalRecords_;
}

QSqlRecord AsyncSqlListModel::lastRecord() const
{
    return lastRecord_;
}

void AsyncSqlListModel::setBusy(bool b)
{
    if (busy_ == b)
        return;
    busy_ = b;
    emit busyChanged(b);
}

void AsyncSqlListModel::setLimit(int limit)
{
    limit_ = limit;
}

int AsyncSqlListModel::limit() const
{
    return limit_;
}

int AsyncSqlListModel::fieldIndex(const QString &fieldName) const
{
    if (fieldName.trimmed().isEmpty())
        return -1;
    return record().indexOf(fieldName);
}

QList<int> AsyncSqlListModel::insertedRows() const
{
    return insertedRows_;
}

QSqlIndex AsyncSqlListModel::primaryIndex() const
{
    return primaryIndex_;
}

bool AsyncSqlListModel::setRecord(int row, const QSqlRecord &record)
{
    if ((row < 0) || (row >= rowCount()))
        return false;
    if (record.count() != emptyRecord_.count())
        return false;

    for (int column = 0; column < record.count(); ++column) {
        QModelIndex cell = this->index(row, 0);
        setData(cell, record.value(column), Qt::UserRole + 1 + column);
    }

    return true;
}

void AsyncSqlListModel::customInsert(const QVariantMap &values)
{
    if (values.isEmpty())
        return;
}

void AsyncSqlListModel::customUpdate(const QVariantMap &values)
{
    if (values.isEmpty())
        return;
}

bool AsyncSqlListModel::isBusy() const
{
    return busy_;
}

void AsyncSqlListModel::onInit(QSqlDatabase &)
{
    // Default: no-op.  Override to run schema migrations or one-time setup
    // on the worker thread before the first select().
}
