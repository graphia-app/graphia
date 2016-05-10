TEMPLATE = app

include(../common.pri)

# Put the binary in the root of the build directory
DESTDIR = ../..
TARGET = "CrashReporter"

_PRODUCT_NAME = $$(PRODUCT_NAME)
isEmpty(_PRODUCT_NAME) {
    _PRODUCT_NAME = "GraphTool"
}

DEFINES += "PRODUCT_NAME=\"\\\"$$_PRODUCT_NAME\\\"\""

QT += qml quick widgets opengl openglextensions xml svg

SOURCES += main.cpp \
    ../app/rendering/openglfunctions.cpp
HEADERS += report.h \
    ../app/rendering/openglfunctions.h
RESOURCES += resources.qrc
