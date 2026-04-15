#ifndef ASYNCSQLLISTMODEL_H
#define ASYNCSQLLISTMODEL_H

#include <QAbstractListModel>
#include <QtSql>
#include <QSqlDatabase>
#include "queryrequest.h"
#include "queryresult.h"

namespace AsyncSql {

// QAbstractListModel-based variant of AsyncSqlTableModel.
//
// Intended for QML consumers and other list-oriented views that do not need
// the table-model boilerplate (columnCount / headerData).  The API mirrors
// AsyncSqlTableModel; role-based access via roleNames() is fully supported.
class AsyncSqlListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
public:
    explicit AsyncSqlListModel(QObject *parent = nullptr);
    virtual ~AsyncSqlListModel();

    void beginTransaction();
    void commitTransaction();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int fieldIndex(const QString &) const;

    virtual void setFilter(const QString &);
    QString filter() const;
    virtual void setSort(int column, Qt::SortOrder);
    void setTable(const QString &);
    QString tableName() const;

    // Supply a raw SELECT base query (e.g. with JOINs).  The base class
    // select() will still append any active filter, sort order, and limit.
    // If unset the default "SELECT * FROM <tableName>" is used.
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

    // Called once on the worker thread immediately before the first select().
    // Override to run schema migrations or any one-time database setup.
    // Throw DatabaseException to abort the operation.
    virtual void onInit(QSqlDatabase &db);

    void setLastError(const QSqlError &);
    void setBusy(bool);

Q_SIGNALS:
    void execute(const QueryRequest &);
    void selected(bool successful);
    void submitted(bool successful);
    // Emitted when a CustomOperation completes (successful = false on error).
    // Consumers that rely on post-write reloads should connect to this signal.
    void executed(bool successful);
    void busyChanged(bool);

    void currentRowChanged(int);

protected Q_SLOTS:
    virtual bool getResults(const QueryResult &);

public Q_SLOTS:
    virtual void select();
    virtual void submitAll();
    void revertAll();

private:
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
};

} // namespace AsyncSql

#endif // ASYNCSQLLISTMODEL_H
