#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

gcc:QMAKE_CXXFLAGS += -std=c++11

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
    graph/genericgraphmodel.cpp \
    gl/sphere.cpp \
    gl/cylinder.cpp \
    graph/simplecomponentmanager.cpp \
    maths/boundingbox.cpp \
    layout/centreinglayout.cpp \
    layout/scalinglayout.cpp \
    layout/linearcomponentlayout.cpp \
    layout/circlepackingcomponentlayout.cpp \
    layout/radialcirclecomponentlayout.cpp \
    gl/quad.cpp \
    layout/layout.cpp \
    layout/collision.cpp \
    layout/spatialoctree.cpp \
    maths/plane.cpp \
    maths/boundingsphere.cpp \
    maths/ray.cpp \
    gl/transition.cpp \
    ui/selectionmanager.cpp

HEADERS  += mainwindow.h \
    graph/grapharray.h \
    parsers/graphfileparser.h \
    parsers/gmlfileparser.h \
    graph/graph.h \
    layout/randomlayout.h \
    utils.h \
    layout/eadeslayout.h \
    gl/openglwindow.h \
    gl/opengldebugmessagemodel.h \
    gl/camera.h \
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
    graph/graphmodel.h \
    gl/sphere.h \
    gl/cylinder.h \
    graph/componentmanager.h \
    graph/simplecomponentmanager.h \
    layout/layout.h \
    layout/centreinglayout.h \
    layout/sequencelayout.h \
    layout/scalinglayout.h \
    maths/boundingbox.h \
    layout/linearcomponentlayout.h \
    layout/circlepackingcomponentlayout.h \
    layout/radialcirclecomponentlayout.h \
    gl/quad.h \
    layout/collision.h \
    maths/ray.h \
    layout/spatialoctree.h \
    maths/plane.h \
    maths/interpolation.h \
    maths/constants.h \
    maths/boundingsphere.h \
    gl/transition.h \
    ui/selectionmanager.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    gl/shaders/ads.frag \
    gl/shaders/instancednodes.vert \
    gl/shaders/instancededges.vert \
    gl/shaders/instancedmarkers.vert \
    gl/shaders/marker.frag \
    gl/shaders/debuglines.vert \
    gl/shaders/debuglines.frag

RESOURCES += \
    resources.qrc
