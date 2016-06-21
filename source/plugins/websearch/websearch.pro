TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick webengine

TARGET = websearch
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    websearchplugin.cpp

HEADERS += \
    websearchplugin.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += websearchplugin.json
