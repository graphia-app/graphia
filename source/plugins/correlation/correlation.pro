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
    correlationplotitem.cpp

HEADERS += \
    correlationplugin.h \
    loading/correlationfileparser.h \
    correlationplotitem.h

RESOURCES += \
    ui/qml.qrc

DISTFILES += correlationplugin.json

win32:CONFIG(release, debug|release): LIBS += -L../../thirdparty/release/ -lthirdparty
else:win32:CONFIG(debug, debug|release): LIBS += -L../../thirdparty/debug/ -lthirdparty
else:unix: LIBS += -L../../thirdparty/ -lthirdparty
