QT += core gui sql qml quick

CONFIG += c++11

TEMPLATE = app
TARGET = AsyncSqlListViewer

INCLUDEPATH += $$PWD/../../

LIBS += -L$$PWD/../../lib -lasyncsql

SOURCES += main.cpp

RESOURCES += qml.qrc

DESTDIR = $$PWD/../../build
