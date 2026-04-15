TEMPLATE = subdirs

SUBDIRS = asyncsql \
          databaseviewer \
          qmlviewer

databaseviewer.subdir  = examples/databaseviewer
databaseviewer.depends = asyncsql

qmlviewer.subdir  = examples/qmlviewer
qmlviewer.depends = asyncsql
