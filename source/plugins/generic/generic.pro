TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick

TARGET = generic
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    genericplugin.cpp \
    loading/gmlfileparser.cpp \
    loading/pairwisetxtfileparser.cpp

HEADERS += \
    genericplugin.h \
    loading/gmlfileparser.h \
    loading/pairwisetxtfileparser.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += genericplugin.json
