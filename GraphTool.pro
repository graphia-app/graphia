#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

QT       += core gui opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parsers/gmlfileparser.cpp \
    graph/graph.cpp \
    layout/randomlayout.cpp \
    layout/eadeslayout.cpp \
    gl/openglwindow.cpp \
    gl/opengldebugmessagemodel.cpp \
    gl/cameracontroller.cpp \
    gl/camerascene.cpp \
    gl/camera.cpp \
    gl/abstractscene.cpp \
    gl/cube.cpp \
    gl/material.cpp \
    gl/sampler.cpp \
    gl/torus.cpp \
    gl/texture.cpp \
    ui/graphview.cpp \
    ui/contentpanewidget.cpp \
    gl/graphscene.cpp \
    graph/genericgraphmodel.cpp

HEADERS  += mainwindow.h \
    graph/grapharray.h \
    parsers/graphfileparser.h \
    parsers/gmlfileparser.h \
    graph/graph.h \
    layout/layoutalgorithm.h \
    layout/randomlayout.h \
    utils.h \
    layout/eadeslayout.h \
    gl/openglwindow.h \
    gl/cameracontroller.h \
    gl/opengldebugmessagemodel.h \
    gl/camerascene.h \
    gl/camera.h \
    gl/camera_p.h \
    gl/abstractscene.h \
    gl/material.h \
    gl/torus.h \
    gl/texture.h \
    gl/cube.h \
    gl/sampler.h \
    ui/graphview.h \
    ui/contentpanewidget.h \
    gl/graphscene.h \
    graph/genericgraphmodel.h \
    graph/graphmodel.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    gl/shaders/instancedgeometry.frag \
    gl/shaders/instancedgeometry.vert

RESOURCES += \
    resources.qrc
