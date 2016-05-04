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

QT += qml quick widgets

SOURCES += main.cpp
RESOURCES += resources.qrc
