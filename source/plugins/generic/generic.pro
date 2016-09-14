TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick xml

TARGET = generic
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES +=

HEADERS += \
    genericplugin.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += genericplugin.json
