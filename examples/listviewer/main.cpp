#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "asyncsql/asyncsqllistmodel.h"
#include "asyncsql/databaseconnection.h"

using namespace AsyncSql;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Set up database
    DatabaseConnection::setDefaultDatabaseName("databases/example.db");
    DatabaseConnection::setDefaultDriver(DatabaseConnection::SQLite);

    // Create and configure the list model
    AsyncSqlListModel *model = new AsyncSqlListModel();
    model->setTable("sales");
    model->select();

    // Create QML engine
    QQmlApplicationEngine engine;

    // Expose model to QML
    engine.rootContext()->setContextProperty("listModel", model);

    const QUrl url(QStringLiteral("qrc:/ListMain.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
