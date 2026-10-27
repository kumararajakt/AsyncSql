#include <QTest>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTemporaryFile>

#include "asyncsql/asyncsqltablemodel.h"
#include "asyncsql/asyncsqllistmodel.h"
#include "asyncsql/asyncmodelregister.h"
#include "asyncsql/databaseconnection.h"

using namespace AsyncSql;

class TestAsyncSql : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testSelect();
    void testDirtyAndRevert();
    void testInsertAndSubmit();
    void testUpdateAndSubmit();
    void testDeleteAndSubmit();
    void testCompositeKey();
    void testListModel();
    void testModelRegister();
    void testChunkedLoading();

private:
    QTemporaryFile m_dbFile;
};

void TestAsyncSql::initTestCase()
{
    QVERIFY(m_dbFile.open());
    const QString dbPath = m_dbFile.fileName();
    m_dbFile.close();

    // Configure connection defaults
    DatabaseConnection::setDefaultDriver(DatabaseConnection::SQLite);
    DatabaseConnection::setDefaultDatabaseName(dbPath);

    // Populate database schema
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("setup_connection"));
        db.setDatabaseName(dbPath);
        QVERIFY(db.open());

        QSqlQuery q(db);
        QVERIFY(q.exec(QStringLiteral("CREATE TABLE users ("
                                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "name TEXT NOT NULL, "
                                      "email TEXT NOT NULL)")));

        QVERIFY(q.exec(QStringLiteral("INSERT INTO users (name, email) VALUES ('Alice', 'alice@example.com')")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO users (name, email) VALUES ('Bob', 'bob@example.com')")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO users (name, email) VALUES ('Charlie', 'charlie@example.com')")));

        // Composite primary key table
        QVERIFY(q.exec(QStringLiteral("CREATE TABLE order_items ("
                                      "order_id INTEGER NOT NULL, "
                                      "item_id INTEGER NOT NULL, "
                                      "quantity INTEGER NOT NULL, "
                                      "PRIMARY KEY (order_id, item_id))")));

        QVERIFY(q.exec(QStringLiteral("INSERT INTO order_items (order_id, item_id, quantity) VALUES (1, 101, 2)")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO order_items (order_id, item_id, quantity) VALUES (1, 102, 5)")));

        // Large dataset table for chunked loading
        QVERIFY(q.exec(QStringLiteral("CREATE TABLE items ("
                                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                      "title TEXT NOT NULL)")));

        db.transaction();
        for (int i = 1; i <= 25; ++i) {
            q.prepare(QStringLiteral("INSERT INTO items (title) VALUES (?)"));
            q.addBindValue(QStringLiteral("Item %1").arg(i));
            q.exec();
        }
        db.commit();

        db.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("setup_connection"));
}

void TestAsyncSql::cleanupTestCase()
{
}

void TestAsyncSql::testSelect()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy spy(&model, &AsyncSqlTableModel::selected);
    model.select();

    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("Alice"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QStringLiteral("Bob"));
    QCOMPARE(model.data(model.index(2, 1)).toString(), QStringLiteral("Charlie"));
}

void TestAsyncSql::testDirtyAndRevert()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy spy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(spy.wait(3000));

    QVERIFY(!model.isDirty());

    // Modify a cell
    QVERIFY(model.setData(model.index(0, 1), QStringLiteral("Alicia")));
    QVERIFY(model.isDirty());
    QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("Alicia"));

    // Revert
    model.revertAll();
    QVERIFY(!model.isDirty());
    QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("Alice"));
}

void TestAsyncSql::testInsertAndSubmit()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy selectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    const int initialRows = model.rowCount();

    // Append record
    QSqlRecord newRec = model.record();
    newRec.setValue(QStringLiteral("name"), QStringLiteral("David"));
    newRec.setValue(QStringLiteral("email"), QStringLiteral("david@example.com"));
    QVERIFY(model.appendRecord(newRec));

    QCOMPARE(model.rowCount(), initialRows + 1);
    QVERIFY(model.isDirty());

    QSignalSpy submitSpy(&model, &AsyncSqlTableModel::submitted);
    model.submitAll();
    QVERIFY(submitSpy.wait(3000));
    QCOMPARE(submitSpy.count(), 1);
    QCOMPARE(submitSpy.takeFirst().at(0).toBool(), true);
    QVERIFY(!model.isDirty());

    // Re-select to confirm DB persistence
    QSignalSpy reselectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(reselectSpy.wait(3000));
    QCOMPARE(model.rowCount(), initialRows + 1);
}

void TestAsyncSql::testUpdateAndSubmit()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy selectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    // Update first row
    QVERIFY(model.setData(model.index(0, 1), QStringLiteral("Alice In Wonderland")));
    QVERIFY(model.isDirty());

    QSignalSpy submitSpy(&model, &AsyncSqlTableModel::submitted);
    model.submitAll();
    QVERIFY(submitSpy.wait(3000));
    QCOMPARE(submitSpy.takeFirst().at(0).toBool(), true);

    // Re-select to confirm
    QSignalSpy reselectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(reselectSpy.wait(3000));
    QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("Alice In Wonderland"));
}

void TestAsyncSql::testDeleteAndSubmit()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy selectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    const int initialRows = model.rowCount();

    // Remove the last row
    QVERIFY(model.removeRows(initialRows - 1, 1));
    QVERIFY(model.isDirty());

    QSignalSpy submitSpy(&model, &AsyncSqlTableModel::submitted);
    model.submitAll();
    QVERIFY(submitSpy.wait(3000));
    QCOMPARE(submitSpy.takeFirst().at(0).toBool(), true);

    // Re-select
    QSignalSpy reselectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(reselectSpy.wait(3000));
    QCOMPARE(model.rowCount(), initialRows - 1);
}

void TestAsyncSql::testCompositeKey()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("order_items"));

    QSignalSpy selectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    QCOMPARE(model.rowCount(), 2);

    // Update quantity on row 0 (order_id=1, item_id=101)
    const int qtyCol = model.fieldIndex(QStringLiteral("quantity"));
    QVERIFY(qtyCol >= 0);
    QVERIFY(model.setData(model.index(0, qtyCol), 10));

    QSignalSpy submitSpy(&model, &AsyncSqlTableModel::submitted);
    model.submitAll();
    QVERIFY(submitSpy.wait(3000));
    QCOMPARE(submitSpy.takeFirst().at(0).toBool(), true);

    // Re-select to confirm update with composite key
    QSignalSpy reselectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(reselectSpy.wait(3000));
    QCOMPARE(model.data(model.index(0, qtyCol)).toInt(), 10);
    // Row 1 quantity should remain unchanged (5)
    QCOMPARE(model.data(model.index(1, qtyCol)).toInt(), 5);
}

void TestAsyncSql::testListModel()
{
    AsyncSqlListModel model;
    model.setTable(QStringLiteral("users"));

    QSignalSpy selectSpy(&model, &AsyncSqlListModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    QVERIFY(model.rowCount() > 0);
    QCOMPARE(model.columnCount(), 1);

    // Verify roleNames
    QHash<int, QByteArray> roles = model.roleNames();
    QVERIFY(!roles.isEmpty());

    // Field access via currentRow
    model.setCurrentRow(0);
    QCOMPARE(model.currentRow(), 0);
    QCOMPARE(model.field(QStringLiteral("name")).toString(), QStringLiteral("Alice In Wonderland"));

    // customUpdate
    model.customUpdate({{QStringLiteral("email"), QStringLiteral("wonderland@example.com")}});
    QVERIFY(model.isDirty());

    QSignalSpy submitSpy(&model, &AsyncSqlListModel::submitted);
    model.submitAll();
    QVERIFY(submitSpy.wait(3000));
    QCOMPARE(submitSpy.takeFirst().at(0).toBool(), true);
}

void TestAsyncSql::testModelRegister()
{
    AsyncSqlTableModel tableModel;
    tableModel.setTable(QStringLiteral("users"));

    AsyncSqlListModel listModel;
    listModel.setTable(QStringLiteral("order_items"));

    AsyncModelRegister reg(nullptr, AsyncModelRegister::MarkAfterSelected);
    reg.addModel(&tableModel);
    reg.addModel(&listModel);

    QSignalSpy spy(&reg, &AsyncModelRegister::allMarked);

    tableModel.select();
    listModel.select();

    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.count(), 1);
    QVERIFY(reg.isDone());
}

void TestAsyncSql::testChunkedLoading()
{
    AsyncSqlTableModel model;
    model.setTable(QStringLiteral("items"));
    model.setChunkSize(10); // 10 items per chunk (total 25 items)

    QSignalSpy selectSpy(&model, &AsyncSqlTableModel::selected);
    model.select();
    QVERIFY(selectSpy.wait(3000));

    // First chunk loaded
    QCOMPARE(model.rowCount(), 10);
    QVERIFY(model.canFetchMore());

    // Fetch next chunk
    QSignalSpy fetchSpy(&model, &AsyncSqlTableModel::moreFetched);
    model.fetchMore();
    QVERIFY(fetchSpy.wait(3000));
    QCOMPARE(model.rowCount(), 20);
    QVERIFY(model.canFetchMore());

    // Fetch remaining chunk (5 items left)
    QSignalSpy fetchSpy2(&model, &AsyncSqlTableModel::moreFetched);
    model.fetchMore();
    QVERIFY(fetchSpy2.wait(3000));
    QCOMPARE(model.rowCount(), 25);

    // No more left
    QVERIFY(!model.canFetchMore());
}

QTEST_MAIN(TestAsyncSql)
#include "tst_asyncsql.moc"
