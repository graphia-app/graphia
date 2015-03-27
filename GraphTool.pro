#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

gcc:QMAKE_CXXFLAGS += -std=c++11 -g

# ThreadSanitizer settings
#gcc:QMAKE_CXXFLAGS += -fsanitize=thread -fPIE
#LIBS += -ltsan
#QMAKE_LFLAGS += -pie

QT       += core gui opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphTool
TEMPLATE = app


SOURCES += \
    source/rendering/camera.cpp \
    source/rendering/material.cpp \
    source/rendering/opengldebugmessagemodel.cpp \
    source/rendering/openglwindow.cpp \
    source/rendering/primitives/cube.cpp \
    source/rendering/primitives/cylinder.cpp \
    source/rendering/primitives/quad.cpp \
    source/rendering/primitives/sphere.cpp \
    source/rendering/sampler.cpp \
    source/rendering/texture.cpp \
    source/rendering/transition.cpp \
    source/graph/genericgraphmodel.cpp \
    source/graph/graph.cpp \
    source/graph/simplecomponentmanager.cpp \
    source/layout/centreinglayout.cpp \
    source/layout/collision.cpp \
    source/layout/layout.cpp \
    source/layout/randomlayout.cpp \
    source/layout/scalinglayout.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/maths/boundingbox.cpp \
    source/maths/boundingsphere.cpp \
    source/maths/frustum.cpp \
    source/maths/plane.cpp \
    source/maths/ray.cpp \
    source/ui/selectionmanager.cpp \
    source/rendering/graphcomponentscene.cpp \
    source/ui/graphcomponentinteractor.cpp \
    source/ui/graphwidget.cpp \
    source/ui/mainwidget.cpp \
    source/graph/componentmanager.cpp \
    source/utils/utils.cpp \
    source/utils/namethread.cpp \
    source/commands/commandmanager.cpp \
    source/commands/command.cpp \
    source/commands/deleteselectednodescommand.cpp \
    source/layout/barneshuttree.cpp \
    source/layout/forcedirectedlayout.cpp \
    source/utils/threadpool.cpp \
    source/utils/semaphore.cpp \
    source/layout/nodepositions.cpp \
    source/maths/conicalfrustum.cpp \
    source/rendering/graphcomponentrenderer.cpp \
    source/utils/deferredexecutor.cpp \
    source/utils/performancecounter.cpp \
    source/rendering/graphcomponentrenderersreference.cpp \
    source/rendering/graphoverviewscene.cpp \
    source/ui/graphoverviewinteractor.cpp \
    source/rendering/graphrenderer.cpp \
    source/ui/graphcommoninteractor.cpp \
    source/loading/gmlfileparser.cpp \
    source/loading/graphfileparser.cpp \
    source/loading/fileidentifier.cpp \
    source/loading/gmlfiletype.cpp \
    source/loading/pairwisetxtfiletype.cpp \
    source/loading/pairwisetxtfileparser.cpp

HEADERS += \
    source/rendering/camera.h \
    source/rendering/material.h \
    source/rendering/opengldebugmessagemodel.h \
    source/rendering/openglwindow.h \
    source/rendering/primitives/cube.h \
    source/rendering/primitives/cylinder.h \
    source/rendering/primitives/quad.h \
    source/rendering/primitives/sphere.h \
    source/rendering/sampler.h \
    source/rendering/scene.h \
    source/rendering/texture.h \
    source/rendering/transition.h \
    source/graph/componentmanager.h \
    source/graph/genericgraphmodel.h \
    source/graph/grapharray.h \
    source/graph/graph.h \
    source/graph/graphmodel.h \
    source/graph/simplecomponentmanager.h \
    source/layout/centreinglayout.h \
    source/layout/collision.h \
    source/layout/layout.h \
    source/layout/randomlayout.h \
    source/layout/scalinglayout.h \
    source/layout/sequencelayout.h \
    source/mainwindow.h \
    source/maths/boundingbox.h \
    source/maths/boundingsphere.h \
    source/maths/constants.h \
    source/maths/frustum.h \
    source/maths/interpolation.h \
    source/maths/line.h \
    source/maths/plane.h \
    source/maths/ray.h \
    source/ui/selectionmanager.h \
    source/rendering/graphcomponentscene.h \
    source/ui/graphcomponentinteractor.h \
    source/ui/interactor.h \
    source/ui/graphwidget.h \
    source/ui/mainwidget.h \
    source/utils/utils.h \
    source/utils/namethread.h \
    source/utils/unique_lock_with_side_effects.h \
    source/commands/commandmanager.h \
    source/commands/command.h \
    source/commands/deleteselectednodescommand.h \
    source/layout/octree.h \
    source/layout/barneshuttree.h \
    source/layout/forcedirectedlayout.h \
    source/utils/threadpool.h \
    source/utils/semaphore.h \
    source/layout/nodepositions.h \
    source/utils/circularbuffer.h \
    source/utils/fixedsizestack.h \
    source/maths/conicalfrustum.h \
    source/rendering/graphcomponentrenderer.h \
    source/rendering/graphcomponentrenderersreference.h \
    source/utils/deferredexecutor.h \
    source/utils/performancecounter.h \
    source/ui/graphoverviewinteractor.h \
    source/rendering/graphoverviewscene.h \
    source/rendering/graphrenderer.h \
    source/ui/graphcommoninteractor.h \
    source/loading/gmlfileparser.h \
    source/loading/graphfileparser.h \
    source/loading/fileidentifier.h \
    source/loading/gmlfiletype.h \
    source/loading/pairwisetxtfiletype.h \
    source/loading/pairwisetxtfileparser.h \
    source/utils/cpp1x_hacks.h

FORMS    += source/mainwindow.ui

OTHER_FILES += \
    source/rendering/shaders/2d.vert \
    source/rendering/shaders/ads.frag \
    source/rendering/shaders/debuglines.frag \
    source/rendering/shaders/debuglines.vert \
    source/rendering/shaders/instancededges.vert \
    source/rendering/shaders/instancedmarkers.vert \
    source/rendering/shaders/instancednodes.vert \
    source/rendering/shaders/marker.frag \
    source/rendering/shaders/screen.frag \
    source/rendering/shaders/screen.vert \
    source/rendering/shaders/selection.frag \
    source/rendering/shaders/selectionMarker.frag \
    GraphTool.supp

RESOURCES += \
    source/resources.qrc
