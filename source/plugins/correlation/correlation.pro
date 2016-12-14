TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)
include(../../thirdparty/thirdparty.pri)

QT += core gui qml quick xml widgets printsupport

TARGET = correlation
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    correlationplugin.cpp \
    loading/correlationfileparser.cpp \
    correlationplotitem.cpp

HEADERS += \
    correlationplugin.h \
    loading/correlationfileparser.h \
    correlationplotitem.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += correlationplugin.json
