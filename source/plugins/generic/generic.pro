TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick

TARGET = generic
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    genericplugin.cpp

HEADERS += \
    genericplugin.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += genericplugin.json
