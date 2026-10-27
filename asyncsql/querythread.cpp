#include "querythread.h"
#include "queryworker.h"
#include <QDebug>
#include "queryrequest.h"
#include "queryresult.h"
#include <QCoreApplication>
#include <mutex>

using namespace AsyncSql;

QueryThread *QueryThread::this_thread = nullptr;
QueryThread::TransactionState QueryThread::transactionState = QueryThread::Unset;

QueryThread::QueryThread(QObject *parent) :
    QThread(parent)
{
    qRegisterMetaType<QueryRequest>();
    qRegisterMetaType<QueryResult>();
    qRegisterMetaType<AsyncSql::QueryResult>();

    m_worker = new QueryWorker();
    m_worker->moveToThread(this);

    connect(this, &QueryThread::queue, m_worker, &QueryWorker::execute);
    connect(m_worker, &QueryWorker::resultsReady, this, [this](const QueryResult &result) {
        Q_EMIT queryFinished(result);

        QObject *receiver = result.getReceiver();
        if (receiver) {
            QMetaObject::invokeMethod(receiver, "getResults",
                                      Qt::QueuedConnection,
                                      Q_ARG(AsyncSql::QueryResult, result));
        }
    });

    connect(this, &QThread::finished, m_worker, &QObject::deleteLater);
}

const QueryThread &QueryThread::instance()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        this_thread = new QueryThread();
        if (QCoreApplication::instance()) {
            QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [t = this_thread]() {
                t->quit();
                t->wait();
                delete t;
                this_thread = nullptr;
            });
        }
        this_thread->start();
    });

    return *this_thread;
}

void QueryThread::run() {
    qDebug() << "Query thread started.";
    exec();
}

void QueryThread::execute(const QueryRequest &request)
{
    qDebug() << "QueryThread::execute() got this query: " << request.getQuery();
    Q_EMIT queue(request); // queues to worker
}

QString QueryThread::getSqliteConnectionName()
{
    return QueryWorker::getSqliteConnectionName();
}

QString QueryThread::getMysqlConnectionName()
{
    return QueryWorker::getMysqlConnectionName();
}

void QueryThread::setTransactionState(QueryThread::TransactionState state)
{
    transactionState = state;
}

QueryThread::TransactionState QueryThread::getTransactionState()
{
    return transactionState;
}
