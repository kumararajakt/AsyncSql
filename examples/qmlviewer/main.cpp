#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "asyncsql/asyncsqltablemodel.h"
#include "asyncsql/databaseconnection.h"

using namespace AsyncSql;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Set up database
    DatabaseConnection::setDefaultDatabaseName("databases/example.db");
    DatabaseConnection::setDefaultDriver(DatabaseConnection::SQLite);

    // Create and configure the model
    AsyncSqlTableModel *model = new AsyncSqlTableModel();
    model->setTable("sales");
    model->select();

    // Create QML engine
    QQmlApplicationEngine engine;

    // Expose model to QML
    engine.rootContext()->setContextProperty("sqlModel", model);
    engine.rootContext()->setContextProperty("app", &app);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
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
