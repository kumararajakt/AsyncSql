# AsyncSql — TODO

## In Progress

## Bugs

- [x] `setOriginalRecords()` sets `records_` instead of `originalRecords_` — copy-paste error (`asyncsqltablemodel.cpp:571`)
- [x] `submitAll()` assumes a single-column primary key everywhere (`primaryIndex_.fieldName(0)`) — composite keys are silently broken (`asyncsqltablemodel.cpp:123,136`, `queryworker.cpp:200,275`)

## Planned

### Features
- [x] Database transaction handling (batch transactions with rollback in `QueryWorker::batchSubmit`)
- [ ] Support additional databases (PostgreSQL, etc.)
- [x] CMake install rules (`install(TARGETS asyncsql ...)` + `find_package` support)
- [x] Implement `customInsert()` and `customUpdate()` base class behaviour
- [ ] Fix QML TableView — horizontal header row showing column names
- [ ] Fix QML TableView — clicking a row should update `currentRow` on the model
- [x] Add `setSelectQuery(QString)` setter so subclasses can supply a raw SELECT without having to override `select()` entirely — checkd's `TaskModel` overrides `select()` just to emit a custom query string, which means the base class `filter_`/`sort_` state is silently ignored
- [x] Add a `QAbstractListModel`-based variant (or make the base class switchable) — checkd and most QML consumers are list models; the `QAbstractTableModel` base forces `columnCount()` and `headerData()` overrides that go unused
- [x] Add a virtual `initDb()` / `onInit()` hook called once before the first `select()` — checkd works around the absence of this by chaining a `CustomOperation` through the `executed` signal, which is fragile and requires an extra round-trip through the background thread

### Code Quality
- [ ] Guard all `qDebug()` calls behind `QT_DEBUG` or a library-level logging flag — there are many left in `queryworker.cpp` and `asyncsqltablemodel.cpp`
- [x] Make `QueryThread` singleton initialization thread-safe (`std::call_once` in `QueryThread::instance()`)
- [x] Clean up `QueryWorker` in `QueryThread` destructor — worker lifecycle tied to thread cleanup via `deleteLater`
- [x] Remove or properly implement the large commented-out transaction-state block in `submitAll()` (`asyncsqltablemodel.cpp:461-470`)
- [x] Replace bare `signals:`/`slots:`/`emit` keywords with `Q_SIGNALS`/`Q_SLOTS`/`Q_EMIT` throughout all headers and sources — required for projects (like checkd) that build with `QT_NO_KEYWORDS` (KDE policy); currently worked around by `-UQT_NO_KEYWORDS` in the CMake target compile options
- [x] Replace `0` with `nullptr` and `""` with `QString()` in default parameter values (`queryrequest.h`, `queryresult.h`) — avoids `-Wzero-as-null-pointer-constant` and deprecated-literal warnings under strict compiler settings
- [x] Document the `executed(bool)` signal — it fires when a `CustomOperation` completes but is not mentioned in the README; consumers like checkd rely on it for post-write reloads

### Security
- [ ] Sanitize or validate `tableName`, `filter`, and sort column values — they are interpolated directly into SQL strings with no escaping, which allows SQL injection if user-controlled values are passed in

## Done

- [x] Async wrapper around QSqlTableModel
- [x] Custom operation support (`QueryRequest::CustomOperation`)
- [x] `runBefore` / `runAfter` hooks on query requests
- [x] QML support (`currentRow`, `field()`, `customInsert()`, `customUpdate()`)
- [x] QML example (`examples/qmlviewer`)
- [x] Restructured repo as a proper static library with subdirs build
- [x] CMake build support — standalone and `add_subdirectory()` subproject modes, example apps skipped when used as a subproject
- [x] Updated README with linking instructions
- [x] `virtual roleNames()` override — required for QML role-based access in subclasses
- [x] KDE compiler-flag compatibility — CMake target undefines `QT_NO_KEYWORDS`, `QT_NO_CAST_FROM_ASCII`, etc. and re-enables exceptions so the library compiles cleanly under KDE's strict settings
- [x] Fixed `setOriginalRecords()` setting `records_` instead of `originalRecords_` in `AsyncSqlTableModel`
- [x] Guarded `SET autocommit` in `QueryWorker` to execute only for MySQL connections (avoiding SQLite syntax error)
- [x] Restricted `setLastRecord()` to insert operations and avoided redundant queries on `Select`
- [x] Added `asyncsqllistmodel` to `asyncsql/asyncsql.pro`
- [x] Fixed public include directory in `CMakeLists.txt` for example builds and external consumers
- [x] Extracted common base class `AsyncSqlBaseModel` eliminating ~600 lines of duplicate model logic
- [x] Atomic batch transactions in `submitAll()` via `QueryRequest::BatchSubmit`
- [x] Targeted signal dispatching using `QMetaObject::invokeMethod` with `QPointer<QObject>`, eliminating $O(N)$ broadcast wakeups
- [x] Multi-column composite primary key support in `UPDATE` and `DELETE` queries
- [x] `AsyncModelRegister` now accepts any `AsyncSqlBaseModel`, supporting both table and list models
- [x] Chunked loading / incremental windowing via `canFetchMore()` and `fetchMore()`
- [x] Comprehensive automated unit test suite with 11 `QTest` test cases passing in CTest
- [x] CMake `install` rules for library targets and headers
