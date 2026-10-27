#ifndef ASYNCMODELREGISTER_H
#define ASYNCMODELREGISTER_H

#include <QObject>
#include <QMap>

namespace AsyncSql {
class AsyncSqlBaseModel;

class AsyncModelRegister : public QObject
{
    Q_OBJECT
public:
    enum MarkMethod {MarkAfterSelected, MarkAfterSubmitted};
    explicit AsyncModelRegister(QObject *parent = nullptr, MarkMethod method = MarkAfterSelected);

    void addModel(AsyncSqlBaseModel *model);
    bool removeModel(AsyncSqlBaseModel *model);
    bool removeAllModels();
    bool isDone() const;

    QList<AsyncSqlBaseModel *> models() const;
Q_SIGNALS:
    void allMarked(bool);
public Q_SLOTS:
    void mark();
    void clear();
private:
    QMap<AsyncSqlBaseModel *, bool> modelMap;
    MarkMethod method;
    int totalMarked;
};
}

#endif // ASYNCMODELREGISTER_H
