QT += core gui sql widgets

CONFIG += c++11

TEMPLATE = app
TARGET = AsyncSqlViewer

INCLUDEPATH += $$PWD/../../

LIBS += -L$$PWD/../../lib -lasyncsql

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    optionsdialog.cpp

HEADERS += \
    mainwindow.h \
    optionsdialog.h

FORMS += \
    mainwindow.ui \
    optionsdialog.ui

DESTDIR = $$PWD/../../build
