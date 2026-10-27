#ifndef ASYNCSQLTABLEMODEL_H
#define ASYNCSQLTABLEMODEL_H

#include "asyncsqlbasemodel.h"

namespace AsyncSql {

class AsyncSqlTableModel : public AsyncSqlBaseModel
{
    Q_OBJECT
public:
    explicit AsyncSqlTableModel(QObject *parent = nullptr);
    virtual ~AsyncSqlTableModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                                  int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;
};

}

#endif // ASYNCSQLTABLEMODEL_H
