# AsyncSql — TODO

## In Progress

## Bugs

- [ ] `setOriginalRecords()` sets `records_` instead of `originalRecords_` — copy-paste error (`asyncsqltablemodel.cpp:571`)
- [ ] `submitAll()` assumes a single-column primary key everywhere (`primaryIndex_.fieldName(0)`) — composite keys are silently broken (`asyncsqltablemodel.cpp:123,136`, `queryworker.cpp:200,275`)

## Planned

### Features
- [ ] Database transaction handling (commented-out `unlockTables()` calls in `queryworker.cpp:89,326,329` hint at incomplete work)
- [ ] Support additional databases (PostgreSQL, etc.)
- [ ] CMake install rules (`install(TARGETS asyncsql ...)` + `find_package` support)
- [ ] Implement `customInsert()` and `customUpdate()` base class behaviour — currently empty stubs (`asyncsqltablemodel.cpp:634,640`)
- [ ] Fix QML TableView — horizontal header row showing column names
- [ ] Fix QML TableView — clicking a row should update `currentRow` on the model

### Code Quality
- [ ] Guard all `qDebug()` calls behind `QT_DEBUG` or a library-level logging flag — there are many left in `queryworker.cpp` and `asyncsqltablemodel.cpp`
- [ ] Make `QueryThread` singleton initialization thread-safe (currently a raw `static` pointer, not safe for concurrent first-call scenarios — `querythread.cpp:22`)
- [ ] Clean up `QueryWorker` in `QueryThread` destructor — worker is `new`-ed but never deleted (`querythread.cpp:33`)
- [ ] Remove or properly implement the large commented-out transaction-state block in `submitAll()` (`asyncsqltablemodel.cpp:461-470`)

### Security
- [ ] Sanitize or validate `tableName`, `filter`, and sort column values — they are interpolated directly into SQL strings with no escaping, which allows SQL injection if user-controlled values are passed in

## Done

- [x] Async wrapper around QSqlTableModel
- [x] Custom operation support (`QueryRequest::CustomOperation`)
- [x] `runBefore` / `runAfter` hooks on query requests
- [x] QML support (`currentRow`, `field()`, `customInsert()`, `customUpdate()`)
- [x] QML example (`examples/qmlviewer`)
- [x] Restructured repo as a proper static library with subdirs build
- [x] CMake build support
- [x] Updated README with linking instructions
