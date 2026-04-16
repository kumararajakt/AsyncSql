TEMPLATE = subdirs

SUBDIRS = asyncsql

# To build the example applications, pass CONFIG+=build_examples to qmake:
#   qmake CONFIG+=build_examples AsyncSql.pro
contains(CONFIG, build_examples) {
    SUBDIRS += databaseviewer qmlviewer listviewer

    databaseviewer.subdir  = examples/databaseviewer
    databaseviewer.depends = asyncsql

    qmlviewer.subdir  = examples/qmlviewer
    qmlviewer.depends = asyncsql

    listviewer.subdir  = examples/listviewer
    listviewer.depends = asyncsql
}
