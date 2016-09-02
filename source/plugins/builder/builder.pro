TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick

TARGET = generic
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES +=

HEADERS += \
    builderplugin.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += builderplugin.json
