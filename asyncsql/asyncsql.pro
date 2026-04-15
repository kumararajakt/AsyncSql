QT += core sql
QT -= gui

TEMPLATE = lib
CONFIG += staticlib c++11

TARGET = asyncsql

DESTDIR = $$PWD/../lib

SOURCES += \
    asyncsqltablemodel.cpp \
    asyncmodelregister.cpp \
    databaseconnection.cpp \
    databaseexception.cpp \
    queryrequest.cpp \
    queryresult.cpp \
    querythread.cpp \
    queryworker.cpp

HEADERS += \
    asyncsqltablemodel.h \
    asyncmodelregister.h \
    databaseconnection.h \
    databaseexception.h \
    queryrequest.h \
    queryresult.h \
    querythread.h \
    queryworker.h
