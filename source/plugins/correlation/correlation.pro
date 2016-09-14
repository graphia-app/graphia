TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

QT += core gui qml quick xml

TARGET = correlation
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    correlationplugin.cpp \
    loading/correlationfileparser.cpp \
    attributestablemodel.cpp

HEADERS += \
    correlationplugin.h \
    loading/correlationfileparser.h \
    attributestablemodel.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += correlationplugin.json
