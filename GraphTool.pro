#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

gcc:QMAKE_CXXFLAGS += -std=c++11 -g

QT       += core gui opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphTool
TEMPLATE = app


SOURCES += \
    source/gl/camera.cpp \
    source/gl/graphscene.cpp \
    source/gl/material.cpp \
    source/gl/opengldebugmessagemodel.cpp \
    source/gl/openglwindow.cpp \
    source/gl/primitives/cube.cpp \
    source/gl/primitives/cylinder.cpp \
    source/gl/primitives/quad.cpp \
    source/gl/primitives/sphere.cpp \
    source/gl/sampler.cpp \
    source/gl/texture.cpp \
    source/gl/transition.cpp \
    source/graph/genericgraphmodel.cpp \
    source/graph/graph.cpp \
    source/graph/simplecomponentmanager.cpp \
    source/layout/centreinglayout.cpp \
    source/layout/circlepackingcomponentlayout.cpp \
    source/layout/collision.cpp \
    source/layout/eadeslayout.cpp \
    source/layout/layout.cpp \
    source/layout/linearcomponentlayout.cpp \
    source/layout/radialcirclecomponentlayout.cpp \
    source/layout/randomlayout.cpp \
    source/layout/scalinglayout.cpp \
    source/layout/spatialoctree.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/maths/boundingbox.cpp \
    source/maths/boundingsphere.cpp \
    source/maths/frustum.cpp \
    source/maths/plane.cpp \
    source/maths/ray.cpp \
    source/parsers/gmlfileparser.cpp \
    source/ui/commandmanager.cpp \
    source/ui/contentpanewidget.cpp \
    source/ui/graphview.cpp \
    source/ui/selectionmanager.cpp

HEADERS += \
    source/gl/camera.h \
    source/gl/graphscene.h \
    source/gl/material.h \
    source/gl/opengldebugmessagemodel.h \
    source/gl/openglwindow.h \
    source/gl/primitives/cube.h \
    source/gl/primitives/cylinder.h \
    source/gl/primitives/quad.h \
    source/gl/primitives/sphere.h \
    source/gl/sampler.h \
    source/gl/scene.h \
    source/gl/texture.h \
    source/gl/transition.h \
    source/graph/componentmanager.h \
    source/graph/genericgraphmodel.h \
    source/graph/grapharray.h \
    source/graph/graph.h \
    source/graph/graphmodel.h \
    source/graph/simplecomponentmanager.h \
    source/layout/centreinglayout.h \
    source/layout/circlepackingcomponentlayout.h \
    source/layout/collision.h \
    source/layout/eadeslayout.h \
    source/layout/layout.h \
    source/layout/linearcomponentlayout.h \
    source/layout/radialcirclecomponentlayout.h \
    source/layout/randomlayout.h \
    source/layout/scalinglayout.h \
    source/layout/sequencelayout.h \
    source/layout/spatialoctree.h \
    source/mainwindow.h \
    source/maths/boundingbox.h \
    source/maths/boundingsphere.h \
    source/maths/constants.h \
    source/maths/frustum.h \
    source/maths/interpolation.h \
    source/maths/line.h \
    source/maths/plane.h \
    source/maths/ray.h \
    source/parsers/gmlfileparser.h \
    source/parsers/graphfileparser.h \
    source/ui/commandmanager.h \
    source/ui/contentpanewidget.h \
    source/ui/graphview.h \
    source/ui/selectionmanager.h \
    source/utils.h

FORMS    += source/mainwindow.ui

OTHER_FILES += \
    source/gl/shaders/2d.vert \
    source/gl/shaders/ads.frag \
    source/gl/shaders/debuglines.frag \
    source/gl/shaders/debuglines.vert \
    source/gl/shaders/instancededges.vert \
    source/gl/shaders/instancedmarkers.vert \
    source/gl/shaders/instancednodes.vert \
    source/gl/shaders/marker.frag \
    source/gl/shaders/screen.frag \
    source/gl/shaders/screen.vert \
    source/gl/shaders/selection.frag \
    source/gl/shaders/selectionMarker.frag

RESOURCES += \
    source/resources.qrc
