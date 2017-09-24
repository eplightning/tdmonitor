include(../global.pri)
TEMPLATE = lib
CONFIG -= qt app_bundle
CONFIG += static c++11 thread

INCLUDEPATH += include
DEPENDPATH += include

DESTDIR = ../bin
TARGET = tdmonitor

SOURCES += \
    selector.cpp \
    misc_utils.cpp \
    tcp.cpp \
    tcp_manager.cpp \
    packet.cpp \
    packets/core.cpp \
    event.cpp \
    marshaller.cpp \
    token.cpp \
    cluster.cpp \
    cluster_loop.cpp \
    monitor.cpp

HEADERS += \
    include/tdmonitor/misc_utils.h \
    include/tdmonitor/packet.h \
    include/tdmonitor/selector.h \
    include/tdmonitor/tcp_manager.h \
    include/tdmonitor/tcp.h \
    include/tdmonitor/types.h \
    include/tdmonitor/packets/core.h \
    include/tdmonitor/event.h \
    include/tdmonitor/marshaller.h \
    include/tdmonitor/token.h \
    include/tdmonitor/monitor.h \
    include/tdmonitor/cluster.h \
    include/tdmonitor/cluster_loop.h

unix:!macx {
    SOURCES += \
        selector/selector_epoll.cpp

    HEADERS += \
        selector/selector_epoll.h
}

macx {
    SOURCES += \
        selector/selector_kqueue.cpp

    HEADERS += \
        selector/selector_kqueue.h
}
