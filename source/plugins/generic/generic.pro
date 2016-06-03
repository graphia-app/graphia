TEMPLATE = lib

include(../../common.pri)

QT += core gui qml quick

TARGET = generic
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    genericplugin.cpp

HEADERS += \
    genericplugin.h

DISTFILES += genericplugin.json
