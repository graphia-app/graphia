TEMPLATE = lib

include(../../common.pri)
include(../../shared/shared.pri)

DEFINES += QCUSTOMPLOT_USE_OPENGL

QT += core gui qml quick xml printsupport

TARGET = correlation
DESTDIR = ../../../plugins

CONFIG += plugin

SOURCES += \
    correlationplugin.cpp \
    loading/correlationfileparser.cpp \
    thirdparty/qcustomplot.cpp \
    customplotitem.cpp

HEADERS += \
    correlationplugin.h \
    loading/correlationfileparser.h \
    thirdparty/qcustomplot.h \
    customplotitem.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += correlationplugin.json
