#ifndef ASYNCSQLBASEMODEL_H
#define ASYNCSQLBASEMODEL_H

#include <QAbstractItemModel>
#include <QtSql>
#include <QSqlDatabase>
#include "queryrequest.h"
#include "queryresult.h"

namespace AsyncSql {

class AsyncSqlBaseModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(int chunkSize READ chunkSize WRITE setChunkSize NOTIFY chunkSizeChanged)
public:
    explicit AsyncSqlBaseModel(QObject *parent = nullptr);
    virtual ~AsyncSqlBaseModel();

    void beginTransaction();
    void commitTransaction();

    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;
    virtual void fetchMore(const QModelIndex &parent = QModelIndex()) override;

    void setChunkSize(int size);
    int chunkSize() const;

    int fieldIndex(const QString &) const;

    virtual void setFilter(const QString &);
    QString filter() const;
    virtual void setSort(int column, Qt::SortOrder);
    void setTable(const QString &);
    QString tableName() const;

    void setSelectQuery(const QString &query);
    QString selectQuery() const;

    bool isDirty() const;
    bool isDirty(const QModelIndex &index) const;

    virtual QSqlRecord record() const;
    QSqlRecord record(int row) const;
    bool setRecord(int row, const QSqlRecord &);
    bool appendRecord(const QSqlRecord &);

    QList<QSqlRecord> insertedRecords() const;
    QMap<int, QSqlRecord> updatedRecords() const;
    QList<QSqlRecord> removedRecords() const;

    virtual void setLimit(int);
    int limit() const;

    QSqlError lastError() const;
    QSqlRecord lastRecord() const;

    void setForeignKeyFlag(bool);
    bool foreignKeyFlag() const;

    void setCurrentRow(int);
    int currentRow() const;

    Q_INVOKABLE virtual QVariant field(const QString &columnName) const;
    Q_INVOKABLE virtual void customInsert(const QVariantMap &);
    Q_INVOKABLE virtual void customUpdate(const QVariantMap &);

    bool isBusy() const;

protected:
    void setRecords(const QList<QSqlRecord> &);
    void setOriginalRecords(const QList<QSqlRecord> &);
    QList<QSqlRecord> records() const;
    QList<QSqlRecord> originalRecords() const;

    void setSelectedSignalSuppressed(bool);
    bool isSelectedSignalSuppressed() const;
    virtual bool validateModel();
    QList<int> insertedRows() const;
    QSqlIndex primaryIndex() const;

    virtual void onInit(QSqlDatabase &db);

    void setLastError(const QSqlError &);
    void setBusy(bool);

    QString filter_;
    QString tableName_;
    QString selectQuery_;
    Qt::SortOrder order_;
    int sortColumn_;
    int limit_;
    QSqlIndex primaryIndex_;
    QList<QSqlRecord> records_, originalRecords_;
    QMap<int, QSqlRecord> updatedRecordMap_, originalUpdatedRecordMap_;
    QList<int> insertedRows_, originalInsertedRows_;
    QList<int> removedRows_, originalRemovedRows_;
    QSqlRecord emptyRecord_;
    bool submitCalled_;
    QSqlError error_;
    QSqlRecord lastRecord_;
    bool foreignKeyFlag_;
    bool selectedSignalSuppressed_;
    bool initDone_;
    int currentRow_;
    bool busy_;
    int chunkSize_;
    bool moreAvailable_;

Q_SIGNALS:
    void execute(const QueryRequest &);
    void selected(bool successful);
    void submitted(bool successful);
    void executed(bool successful);
    void busyChanged(bool);
    void currentRowChanged(int);
    void chunkSizeChanged(int);
    void moreFetched(bool successful);

public Q_SLOTS:
    virtual bool getResults(const AsyncSql::QueryResult &);
    virtual void select();
    virtual void submitAll();
    void revertAll();
};

}

#endif // ASYNCSQLBASEMODEL_H
