#include "asyncsqlbasemodel.h"
#include "queryrequest.h"
#include "queryresult.h"
#include "querythread.h"
#include <QSqlDatabase>
#include <QDebug>

using namespace AsyncSql;

AsyncSqlBaseModel::AsyncSqlBaseModel(QObject *parent) :
    QAbstractItemModel(parent),
    order_(Qt::AscendingOrder),
    sortColumn_(-1),
    limit_(-1),
    submitCalled_(false),
    error_(QSqlError()),
    foreignKeyFlag_(true),
    selectedSignalSuppressed_(false),
    initDone_(false),
    currentRow_(-1),
    busy_(false)
{
    connect(this, SIGNAL(execute(const QueryRequest &)),
            &QueryThread::instance(), SLOT(execute(const QueryRequest &)));
}

AsyncSqlBaseModel::~AsyncSqlBaseModel()
{
}

QModelIndex AsyncSqlBaseModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= rowCount() || column < 0 || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex AsyncSqlBaseModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

Qt::ItemFlags AsyncSqlBaseModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractItemModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;

    return theFlags;
}

int AsyncSqlBaseModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return records_.count();
}

int AsyncSqlBaseModel::fieldIndex(const QString &field) const
{
    return emptyRecord_.indexOf(field);
}

void AsyncSqlBaseModel::setFilter(const QString &filter)
{
    filter_ = filter;
}

QString AsyncSqlBaseModel::filter() const
{
    return filter_;
}

void AsyncSqlBaseModel::setSort(int column, Qt::SortOrder order)
{
    sortColumn_ = column;
    order_ = order;
}

void AsyncSqlBaseModel::setTable(const QString &tableName)
{
    tableName_ = tableName;
}

QString AsyncSqlBaseModel::tableName() const
{
    return tableName_;
}

void AsyncSqlBaseModel::setSelectQuery(const QString &query)
{
    selectQuery_ = query;
}

QString AsyncSqlBaseModel::selectQuery() const
{
    return selectQuery_;
}

void AsyncSqlBaseModel::setLimit(int limit)
{
    limit_ = limit;
}

int AsyncSqlBaseModel::limit() const
{
    return limit_;
}

bool AsyncSqlBaseModel::isDirty() const
{
    return !insertedRows_.isEmpty() || !updatedRecordMap_.isEmpty() || !removedRows_.isEmpty();
}

bool AsyncSqlBaseModel::isDirty(const QModelIndex &index) const
{
    return insertedRows_.contains(index.row())
            || !updatedRecordMap_.value(index.row()).field(index.column()).isGenerated()
            || removedRows_.contains(index.row());
}

QSqlRecord AsyncSqlBaseModel::record() const
{
    return emptyRecord_;
}

QSqlRecord AsyncSqlBaseModel::record(int row) const
{
    if (row < 0 || row >= records_.count())
        return emptyRecord_;

    return records_.at(row);
}

bool AsyncSqlBaseModel::setRecord(int row, const QSqlRecord &record)
{
    if (row < 0 || row >= records_.count() || record.count() != this->record().count())
        return false;

    records_[row] = record;
    updatedRecordMap_.insert(row, record);

    Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
    return true;
}

bool AsyncSqlBaseModel::appendRecord(const QSqlRecord &record)
{
    if (record.count() != this->record().count())
        return false;

    const int newRow = records_.count();
    beginInsertRows(QModelIndex(), newRow, newRow);
    records_ << record;
    insertedRows_ << newRow;
    endInsertRows();

    return true;
}

QList<QSqlRecord> AsyncSqlBaseModel::insertedRecords() const
{
    QList<QSqlRecord> records;
    for (int row : insertedRows_)
        records << records_.at(row);

    return records;
}

QMap<int, QSqlRecord> AsyncSqlBaseModel::updatedRecords() const
{
    return updatedRecordMap_;
}

QList<QSqlRecord> AsyncSqlBaseModel::removedRecords() const
{
    QList<QSqlRecord> records;
    for (int row : removedRows_)
        records << records_.at(row);

    return records;
}

QSqlError AsyncSqlBaseModel::lastError() const
{
    return error_;
}

QSqlRecord AsyncSqlBaseModel::lastRecord() const
{
    return lastRecord_;
}

void AsyncSqlBaseModel::setForeignKeyFlag(bool flag)
{
    foreignKeyFlag_ = flag;
}

bool AsyncSqlBaseModel::foreignKeyFlag() const
{
    return foreignKeyFlag_;
}

void AsyncSqlBaseModel::setCurrentRow(int row)
{
    if (currentRow_ == row)
        return;

    currentRow_ = row;
    Q_EMIT currentRowChanged(row);
}

int AsyncSqlBaseModel::currentRow() const
{
    return currentRow_;
}

QVariant AsyncSqlBaseModel::field(const QString &columnName) const
{
    return data(index(currentRow_, fieldIndex(columnName)), Qt::DisplayRole);
}

void AsyncSqlBaseModel::customInsert(const QVariantMap &)
{
}

void AsyncSqlBaseModel::customUpdate(const QVariantMap &)
{
}

bool AsyncSqlBaseModel::isBusy() const
{
    return busy_;
}

void AsyncSqlBaseModel::setBusy(bool busy)
{
    if (busy_ == busy)
        return;

    busy_ = busy;
    Q_EMIT busyChanged(busy);
}

void AsyncSqlBaseModel::setRecords(const QList<QSqlRecord> &records)
{
    records_ = records;
}

void AsyncSqlBaseModel::setOriginalRecords(const QList<QSqlRecord> &records)
{
    originalRecords_ = records;
}

QList<QSqlRecord> AsyncSqlBaseModel::records() const
{
    return records_;
}

QList<QSqlRecord> AsyncSqlBaseModel::originalRecords() const
{
    return originalRecords_;
}

void AsyncSqlBaseModel::setSelectedSignalSuppressed(bool s)
{
    selectedSignalSuppressed_ = s;
}

bool AsyncSqlBaseModel::isSelectedSignalSuppressed() const
{
    return selectedSignalSuppressed_;
}

bool AsyncSqlBaseModel::validateModel()
{
    return true;
}

QList<int> AsyncSqlBaseModel::insertedRows() const
{
    return insertedRows_;
}

QSqlIndex AsyncSqlBaseModel::primaryIndex() const
{
    return primaryIndex_;
}

void AsyncSqlBaseModel::onInit(QSqlDatabase &)
{
}

void AsyncSqlBaseModel::setLastError(const QSqlError &e)
{
    error_ = e;
}

void AsyncSqlBaseModel::beginTransaction()
{
    QueryRequest request(this, QString(), tableName_, QueryRequest::BeginTransaction);
    Q_EMIT execute(request);
}

void AsyncSqlBaseModel::commitTransaction()
{
    QueryRequest request(this, QString(), tableName_, QueryRequest::CommitTransaction);
    Q_EMIT execute(request);
}

void AsyncSqlBaseModel::select()
{
    QString query = selectQuery_.trimmed().isEmpty()
        ? QStringLiteral("SELECT * FROM %1").arg(tableName_)
        : selectQuery_.trimmed();

    if (!filter_.trimmed().isEmpty())
        query += QStringLiteral(" WHERE ") + filter_.trimmed();

    if (sortColumn_ >= 0) {
        const QString col = emptyRecord_.fieldName(sortColumn_);
        if (!col.isEmpty()) {
            switch (order_) {
            case Qt::AscendingOrder:
                query += QStringLiteral(" ORDER BY %1").arg(col);
                break;
            case Qt::DescendingOrder:
                query += QStringLiteral(" ORDER BY %1 DESC").arg(col);
                break;
            default:
                break;
            }
        }
    }

    if (limit_ > 0)
        query += QStringLiteral(" LIMIT %1").arg(limit_);

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

    Q_EMIT execute(request);
}

void AsyncSqlBaseModel::submitAll()
{
    if (insertedRows_.isEmpty() && updatedRecordMap_.isEmpty() && removedRows_.isEmpty()) {
        Q_EMIT submitted(true);
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

    QueryRequest request(this, QString(), tableName_, QueryRequest::BatchSubmit);
    request.setPrimaryIndex(primaryIndex_);

    QList<QSqlRecord> inserted;
    for (int i = 0; i < insertedRows_.count(); ++i)
        inserted << records_.at(insertedRows_.at(i));

    QList<QSqlRecord> updated = updatedRecordMap_.values();

    QList<QSqlRecord> removed;
    for (int i = 0; i < removedRows_.count(); ++i)
        removed << records_.at(removedRows_.at(i));

    request.setBatchRecords(inserted, updated, removed);

    submitCalled_ = true;
    setBusy(true);
    Q_EMIT execute(request);
}

void AsyncSqlBaseModel::revertAll()
{
    beginResetModel();
    records_ = originalRecords_;
    insertedRows_.clear();
    updatedRecordMap_.clear();
    removedRows_.clear();
    endResetModel();
}

bool AsyncSqlBaseModel::getResults(const AsyncSql::QueryResult &result)
{
    if (result.getReceiver() && result.getReceiver() != this)
        return false;

    setBusy(false);

    if (error_.isValid())
        return false;

    if (result.getError().isValid()) {
        error_ = result.getError();
        submitCalled_ = false;

        if (result.getRequestType() == QueryRequest::Select)
            Q_EMIT selected(false);
        else if (result.getRequestType() == QueryRequest::CustomOperation)
            Q_EMIT executed(false);
        else
            Q_EMIT submitted(false);

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
            Q_EMIT selected(true);
        endResetModel();
    }
        break;
    case QueryRequest::BatchSubmit:
        insertedRows_.clear();
        updatedRecordMap_.clear();
        removedRows_.clear();
        submitCalled_ = false;
        Q_EMIT submitted(true);
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
    case QueryRequest::BeginTransaction:
    case QueryRequest::CommitTransaction:
    case QueryRequest::Command:
        break;
    case QueryRequest::CustomOperation:
        Q_EMIT executed(true);
        break;
    case QueryRequest::None:
    default:
        break;
    }

    if (!insertedRows_.count() && !updatedRecordMap_.count() && !removedRows_.count() && submitCalled_) {
        submitCalled_ = false;
        Q_EMIT submitted(true);
    }

    return true;
}
