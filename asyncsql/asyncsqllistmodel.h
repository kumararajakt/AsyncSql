#ifndef ASYNCSQLLISTMODEL_H
#define ASYNCSQLLISTMODEL_H

#include "asyncsqlbasemodel.h"

namespace AsyncSql {

// QAbstractListModel-like variant of AsyncSqlTableModel inheriting AsyncSqlBaseModel.
//
// Intended for QML consumers and list views. Mirrors AsyncSqlTableModel
// without the 2D table boilerplate (headerData). Role-based access via
// roleNames() is fully supported.
class AsyncSqlListModel : public AsyncSqlBaseModel
{
    Q_OBJECT
public:
    explicit AsyncSqlListModel(QObject *parent = nullptr);
    virtual ~AsyncSqlListModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
};

}

#endif // ASYNCSQLLISTMODEL_H
