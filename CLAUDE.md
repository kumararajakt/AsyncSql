# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

This is a Qt 6 project using qmake. To build:

```bash
qmake AsyncSql.pro
make
```

The project requires Qt 6 with the `core`, `gui`, `sql`, and `widgets` modules. Configured with C++11 (`CONFIG += c++11`).

The `TARGET` is `AsyncSql` and `TEMPLATE = app` — it builds the library together with the `databaseviewer` example as a single application.

## Architecture

The library lives entirely in `asyncsql/` and provides an asynchronous wrapper around Qt's SQL model/query APIs. All classes are in the `AsyncSql` namespace.

**Threading model:**
- `QueryThread` is a singleton background `QThread` that owns a `QueryWorker`.
- `QueryWorker` runs on the background thread and holds the actual `QSqlDatabase` connection. It performs all SQL operations (select, insert, update, delete, transactions, custom operations).
- `AsyncSqlTableModel` lives on the UI thread. It emits `execute(QueryRequest)` signals which are routed to `QueryThread::execute()`, then forwarded to `QueryWorker`. Results come back via `QueryResult` through `queryFinished` → `getResults()`.

**Key classes:**
- `AsyncSqlTableModel` — `QAbstractTableModel` subclass; the primary API for consumers. Mirrors much of `QSqlTableModel`'s interface (setTable, setFilter, setSort, select, submitAll, revertAll, record, insertRows, removeRows). Supports QML via `currentRow`, `field()`, `customInsert()`, `customUpdate()`.
- `QueryRequest` — value type that encapsulates a pending operation: request type (`Select`, `Insert`, `Update`, `Delete`, `BeginTransaction`, `CommitTransaction`, `Command`, `CustomOperation`), query string, records, and optional `std::function<void(QSqlDatabase)>` lambdas for `runBefore`, `runAfter`, and `customOperation`.
- `QueryResult` — value type returned from the worker with the result records, error, and last record.
- `DatabaseConnection` — holds connection parameters (driver, host, port, user, password, database name, connect options). Has both per-instance and global `static` defaults, which are set once at startup before any model is created.
- `AsyncModelRegister` — coordinates multiple models; tracks which have completed their `selected` or `submitted` signals and emits `allMarked(bool)` when all are done.
- `DatabaseException` — wraps a `QSqlError` with an optional human-readable message; thrown inside custom operation lambdas to signal failure back to the worker.

**Supported databases:** SQLite and MySQL only (hardcoded connection name helpers: `getSqliteConnectionName()`, `getMysqlConnectionName()`).

**Custom operations pattern:** Create a `QueryRequest` with type `CustomOperation`, call `setCustomOperation(lambda)`, and `emit execute(request)` from the model subclass. Throw `DatabaseException` inside the lambda to report failure. Use `runBefore`/`runAfter` lambdas for pre/post hooks on any request type.

## Example app

`examples/databaseviewer/` is a Qt Widgets app that exercises the library against `databases/example.db` (SQLite). It is compiled as part of the main `.pro` file — there is no separate build step for the example.
