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


SOURCES += source/main.cpp\
    source/mainwindow.cpp \
    source/parsers/gmlfileparser.cpp \
    source/graph/graph.cpp \
    source/layout/randomlayout.cpp \
    source/layout/eadeslayout.cpp \
    source/gl/openglwindow.cpp \
    source/gl/opengldebugmessagemodel.cpp \
    source/gl/camera.cpp \
    source/gl/abstractscene.cpp \
    source/gl/cube.cpp \
    source/gl/material.cpp \
    source/gl/sampler.cpp \
    source/gl/torus.cpp \
    source/gl/texture.cpp \
    source/ui/graphview.cpp \
    source/ui/contentpanewidget.cpp \
    source/gl/graphscene.cpp \
    source/graph/genericgraphmodel.cpp \
    source/gl/sphere.cpp \
    source/gl/cylinder.cpp \
    source/graph/simplecomponentmanager.cpp \
    source/maths/boundingbox.cpp \
    source/layout/centreinglayout.cpp \
    source/layout/scalinglayout.cpp \
    source/layout/linearcomponentlayout.cpp \
    source/layout/circlepackingcomponentlayout.cpp \
    source/layout/radialcirclecomponentlayout.cpp \
    source/gl/quad.cpp \
    source/layout/layout.cpp \
    source/layout/collision.cpp \
    source/layout/spatialoctree.cpp \
    source/maths/plane.cpp \
    source/maths/boundingsphere.cpp \
    source/maths/ray.cpp \
    source/gl/transition.cpp \
    source/ui/selectionmanager.cpp \
    source/maths/frustum.cpp

HEADERS  += source/mainwindow.h \
    source/graph/grapharray.h \
    source/parsers/graphfileparser.h \
    source/parsers/gmlfileparser.h \
    source/graph/graph.h \
    source/layout/randomlayout.h \
    source/utils.h \
    source/layout/eadeslayout.h \
    source/gl/openglwindow.h \
    source/gl/opengldebugmessagemodel.h \
    source/gl/camera.h \
    source/gl/abstractscene.h \
    source/gl/material.h \
    source/gl/torus.h \
    source/gl/texture.h \
    source/gl/cube.h \
    source/gl/sampler.h \
    source/ui/graphview.h \
    source/ui/contentpanewidget.h \
    source/gl/graphscene.h \
    source/graph/genericgraphmodel.h \
    source/graph/graphmodel.h \
    source/gl/sphere.h \
    source/gl/cylinder.h \
    source/graph/componentmanager.h \
    source/graph/simplecomponentmanager.h \
    source/layout/layout.h \
    source/layout/centreinglayout.h \
    source/layout/sequencelayout.h \
    source/layout/scalinglayout.h \
    source/maths/boundingbox.h \
    source/layout/linearcomponentlayout.h \
    source/layout/circlepackingcomponentlayout.h \
    source/layout/radialcirclecomponentlayout.h \
    source/gl/quad.h \
    source/layout/collision.h \
    source/maths/ray.h \
    source/layout/spatialoctree.h \
    source/maths/plane.h \
    source/maths/interpolation.h \
    source/maths/constants.h \
    source/maths/boundingsphere.h \
    source/gl/transition.h \
    source/ui/selectionmanager.h \
    source/maths/frustum.h \
    source/maths/line.h

FORMS    += source/mainwindow.ui

OTHER_FILES += \
    source/gl/shaders/ads.frag \
    source/gl/shaders/instancednodes.vert \
    source/gl/shaders/instancededges.vert \
    source/gl/shaders/instancedmarkers.vert \
    source/gl/shaders/marker.frag \
    source/gl/shaders/debuglines.vert \
    source/gl/shaders/debuglines.frag \
    source/gl/shaders/screen.vert \
    source/gl/shaders/screen.frag \
    source/gl/shaders/selection.frag \
    source/gl/shaders/2d.vert \
    source/gl/shaders/selectionMarker.frag

RESOURCES += \
    source/resources.qrc
