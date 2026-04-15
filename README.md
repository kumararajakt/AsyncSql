# Async SQL

The asynchronous version of **QSqlTableModel**.

I'm sure you love **QSqlTableModel** as much as I do. The only problem with it is that it's synchronous (which means that all queries run on the UI thread). This makes them unusable for large data sets or complex database queries or slow database access over a network (because it would cause your UI to freeze occasionally).

The solution? Combine the ease of using **QSqlTableModel** with a powerful single-thread backend. It's basically **QSqlTableModel** on steroids!

I conceived the concept of this project after reading [this article](http://www.linuxjournal.com/article/9602?page=0,0) about a year ago. Thanks Dave Berton!

**NOTE**: I will make this article sound a lot more professional later. Let me have my fun for now!

## Features
- Works for the **SQLite** and **MySQL** databases (only)
- Allows for limit setting
- Can be used to display tables that requires foreign key (it's still pretty difficult for now, but I've done it before)
- Every other feature that the **QSqlTableModel** has
- Allows for custom operations on the database thread (see example below)

## How to use
The project itself is an example of how to use this library. However, here's a preview:

    // In header file
    AsyncSql::AsyncSqlTableModel *model;
    ...
    
    // In source file
    using namespace AsyncSql;
    ...
    // Set database connection options using the DatabaseConnection class
    DatabaseConnection::setDefaultDriver(DatabaseConnection::SQLite);
    ...
    model = new AsyncSqlTableModel(this);
    model->setTable("sales");
    
    // Using C++11
    connect(model, &AsyncSqlTableModel::selected, [this](bool successful)
    {
        // Display the first record if successful.
        if(successful)
            qDebug() << "Hello AsyncSql!" << model->record(0);
        else
            qDebug() << "Error discovered: " << model->lastError().text();
    });
    
You can view the table with a **QTableView**, since **AsyncTableModel** inherits **QAbstractTableModel**.
    
    // In header file...
    QTableView *view;
    
    // In source file...
    view = new QTableView(this);
    view->setModel(model);

## Custom operations
In some occasions, you may want to run specific commands that are not provided by this library. A good example would be creating a user on a MySQL database. To achieve this, use the **AsyncSql::QueryRequest::setCustomOperation()** function.

    // Assume user name and password are defined
    // using namespace AsyncSql;
    QueryRequest request(this, "" /*no query*/, tableName(), QueryRequest::CustomOperation);
    
    // Using C++11 ...
    request.setCustomOperation([userName, password](QSqlDatabase db)
    {
        // db is the connection object used on the query thread
        QSqlQuery qry(db);
        
        // Create user and grant access to database on localhost only
        qry.prepare(QString("CREATE USER '%1'@'localhost' IDENTIFIED BY '%2'").arg(userName, password));
        
        // If query fails, throw an exception that would be caught by the thread
        if(!qry.exec())
            throw DatabaseException(qry.lastError(), tr("Failed to create user %1 on localhost.").arg(userName));
           
        // More commands...
    }
    
    emit execute(request);
    
    // Connect to the AsyncSql::AsyncSqlTableModel::executed() signal to check if operation was performed successfully.
    
    
The functions **AsyncSql::QueryRequest::runBefore()** and **AsyncSql::QueryRequest::runAfter()** are also provided to conveniently run commands before or after a query respectively.

## Building the library

### qmake

```bash
qmake AsyncSql.pro
make
```

This produces `lib/libasyncsql.a` and the example binaries under `build/`.

### CMake

```bash
cmake -B build .
cmake --build build
```

---

## Linking in your own project

### qmake

Copy (or clone) this repository somewhere, build it once to produce the static library, then in your own `.pro` file:

```qmake
# Path to the AsyncSql repository root
ASYNCSQL_ROOT = /path/to/AsyncSql

INCLUDEPATH += $$ASYNCSQL_ROOT
LIBS        += -L$$ASYNCSQL_ROOT/lib -lasyncsql
```

Then include headers with the `asyncsql/` prefix:

```cpp
#include "asyncsql/asyncsqltablemodel.h"
#include "asyncsql/databaseconnection.h"
```

### CMake

If you have the AsyncSql source tree available (e.g. as a git submodule), use `add_subdirectory`:

```cmake
add_subdirectory(path/to/AsyncSql)

target_link_libraries(YourTarget PRIVATE asyncsql)
```

The `asyncsql` target already propagates its include directories, so no extra `target_include_directories` call is needed.

Alternatively, build and install AsyncSql first and consume it via `find_package` (once install rules are wired up).

---

## Dependencies
- [Qt 6] framework

## Todo

See [TODO.md](TODO.md).

Lastly, as I always say:

**Please report all bugs. 
Also, I am FAR FROM perfect. If you see anything that can be done better, please notify me.**

License
----

MIT

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [Qt 6]: <https://www.qt.io>
