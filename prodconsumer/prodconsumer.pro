include(../global.pri)
TEMPLATE = app
CONFIG -= qt app_bundle
CONFIG += console c++11 thread

INCLUDEPATH += ../lib/include
DEPENDPATH += ../lib/include

DESTDIR = ../bin
TARGET = prodconsumer

LIBS += -L../bin -ltdmonitor

unix {
    TARGETDEPS += ../bin/libtdmonitor.a
}

SOURCES += \
    main.cpp
