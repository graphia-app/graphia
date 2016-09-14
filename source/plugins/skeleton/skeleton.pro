TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick xml

TARGET = skeleton
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    skeletonplugin.cpp

HEADERS += \
    skeletonplugin.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += skeletonplugin.json
