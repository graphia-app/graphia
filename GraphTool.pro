TEMPLATE = app

CONFIG += c++11

QT += qml quick opengl openglextensions

CONFIG(debug,debug|release) {
  DEFINES += _DEBUG
}

HEADERS += \
    source/commands/command.h \
    source/commands/commandmanager.h \
    source/commands/deleteselectednodescommand.h \
    source/graph/genericgraphmodel.h \
    source/graph/graph.h \
    source/graph/grapharray.h \
    source/graph/graphmodel.h \
    source/layout/barneshuttree.h \
    source/layout/centreinglayout.h \
    source/layout/collision.h \
    source/layout/forcedirectedlayout.h \
    source/layout/layout.h \
    source/layout/nodepositions.h \
    source/layout/octree.h \
    source/layout/randomlayout.h \
    source/layout/scalinglayout.h \
    source/layout/sequencelayout.h \
    source/loading/fileidentifier.h \
    source/loading/gmlfileparser.h \
    source/loading/gmlfiletype.h \
    source/loading/graphfileparser.h \
    source/loading/pairwisetxtfileparser.h \
    source/loading/pairwisetxtfiletype.h \
    source/maths/boundingbox.h \
    source/maths/boundingsphere.h \
    source/maths/conicalfrustum.h \
    source/maths/constants.h \
    source/maths/frustum.h \
    source/maths/interpolation.h \
    source/maths/line.h \
    source/maths/plane.h \
    source/maths/ray.h \
    source/rendering/primitives/cylinder.h \
    source/rendering/primitives/sphere.h \
    source/rendering/camera.h \
    source/rendering/graphcomponentrenderer.h \
    source/rendering/graphcomponentscene.h \
    source/rendering/graphoverviewscene.h \
    source/rendering/graphrenderer.h \
    source/rendering/scene.h \
    source/rendering/transition.h \
    source/ui/graphcommoninteractor.h \
    source/ui/graphcomponentinteractor.h \
    source/ui/graphoverviewinteractor.h \
    source/ui/interactor.h \
    source/ui/selectionmanager.h \
    source/utils/circularbuffer.h \
    source/utils/cpp1x_hacks.h \
    source/utils/deferredexecutor.h \
    source/utils/fixedsizestack.h \
    source/utils/namethread.h \
    source/utils/performancecounter.h \
    source/utils/semaphore.h \
    source/utils/threadpool.h \
    source/utils/utils.h \
    source/application.h \
    source/ui/document.h \
    source/ui/graphquickitem.h \
    source/rendering/opengldebuglogger.h \
    source/rendering/openglfunctions.h \
    source/utils/movablepointer.h \
    source/graph/abstractcomponentmanager.h \
    source/graph/componentmanager.h \
    source/utils/debugpauser.h \
    source/utils/singleton.h \
    source/transform/graphtransform.h \
    source/transform/identitytransform.h \
    source/transform/transformedgraph.h

SOURCES += \
    source/main.cpp \
    source/commands/command.cpp \
    source/commands/commandmanager.cpp \
    source/commands/deleteselectednodescommand.cpp \
    source/graph/genericgraphmodel.cpp \
    source/graph/graph.cpp \
    source/layout/barneshuttree.cpp \
    source/layout/centreinglayout.cpp \
    source/layout/collision.cpp \
    source/layout/forcedirectedlayout.cpp \
    source/layout/layout.cpp \
    source/layout/nodepositions.cpp \
    source/layout/randomlayout.cpp \
    source/layout/scalinglayout.cpp \
    source/loading/fileidentifier.cpp \
    source/loading/gmlfileparser.cpp \
    source/loading/gmlfiletype.cpp \
    source/loading/graphfileparser.cpp \
    source/loading/pairwisetxtfileparser.cpp \
    source/loading/pairwisetxtfiletype.cpp \
    source/maths/boundingbox.cpp \
    source/maths/boundingsphere.cpp \
    source/maths/conicalfrustum.cpp \
    source/maths/frustum.cpp \
    source/maths/plane.cpp \
    source/maths/ray.cpp \
    source/rendering/primitives/cylinder.cpp \
    source/rendering/primitives/sphere.cpp \
    source/rendering/camera.cpp \
    source/rendering/graphcomponentrenderer.cpp \
    source/rendering/graphcomponentscene.cpp \
    source/rendering/graphoverviewscene.cpp \
    source/rendering/graphrenderer.cpp \
    source/rendering/transition.cpp \
    source/ui/graphcommoninteractor.cpp \
    source/ui/graphcomponentinteractor.cpp \
    source/ui/graphoverviewinteractor.cpp \
    source/ui/selectionmanager.cpp \
    source/utils/deferredexecutor.cpp \
    source/utils/namethread.cpp \
    source/utils/performancecounter.cpp \
    source/utils/semaphore.cpp \
    source/utils/threadpool.cpp \
    source/utils/utils.cpp \
    source/application.cpp \
    source/ui/document.cpp \
    source/ui/graphquickitem.cpp \
    source/rendering/opengldebuglogger.cpp \
    source/rendering/openglfunctions.cpp \
    source/graph/abstractcomponentmanager.cpp \
    source/graph/componentmanager.cpp \
    source/utils/debugpauser.cpp \
    source/transform/transformedgraph.cpp

OTHER_FILES += \
    source/ui/qml/main.qml \
    source/ui/qml/DocumentUI.qml

RESOURCES += \
    source/rendering/shaders.qrc \
    source/ui/qml.qrc \
    source/icon-themes/icons.qrc
