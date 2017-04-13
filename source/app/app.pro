TEMPLATE = app

include(../common.pri)
include(../shared/shared.pri)
include(../thirdparty/thirdparty.pri)

# Put the binary in the root of the build directory
DESTDIR = ../..
TARGET = "GraphTool"

_PRODUCT_NAME = $$(PRODUCT_NAME)
!isEmpty(_PRODUCT_NAME) {
    TARGET = $$_PRODUCT_NAME
}

DEFINES += "PRODUCT_NAME=\"\\\"$$TARGET\\\"\""

RC_ICONS = icon/Icon.ico # Windows
ICON = icon/Icon.icns # OSX

QT += qml quick widgets opengl openglextensions xml svg

HEADERS += \
    application.h \
    commands/applytransformationscommand.h \
    commands/commandmanager.h \
    commands/deleteselectednodescommand.h \
    commands/selectnodescommand.h \
    graph/componentmanager.h \
    graph/elementiddistinctsetcollection.h \
    graph/filter.h \
    graph/graph.h \
    graph/graphconsistencychecker.h \
    graph/graphmodel.h \
    graph/mutablegraph.h \
    layout/barneshuttree.h \
    layout/centreinglayout.h \
    layout/circlepackcomponentlayout.h \
    layout/collision.h \
    layout/componentlayout.h \
    layout/forcedirectedlayout.h \
    layout/layout.h \
    layout/layoutsettings.h \
    layout/nodepositions.h \
    layout/octree.h \
    layout/powerof2gridcomponentlayout.h \
    layout/randomlayout.h \
    layout/scalinglayout.h \
    layout/sequencelayout.h \
    loading/parserthread.h \
    maths/boundingbox.h \
    maths/boundingsphere.h \
    maths/circle.h \
    maths/conicalfrustum.h \
    maths/constants.h \
    maths/frustum.h \
    maths/interpolation.h \
    maths/line.h \
    maths/plane.h \
    maths/ray.h \
    rendering/camera.h \
    rendering/compute/gpucomputejob.h \
    rendering/compute/gpucomputethread.h \
    rendering/compute/sdfcomputejob.h \
    rendering/glyphmap.h \
    rendering/graphcomponentrenderer.h \
    rendering/graphcomponentscene.h \
    rendering/graphoverviewscene.h \
    rendering/graphrenderer.h \
    rendering/opengldebuglogger.h \
    rendering/openglfunctions.h \
    rendering/primitives/rectangle.h \
    rendering/primitives/sphere.h \
    rendering/scene.h \
    rendering/transition.h \
    transform/edgecontractiontransform.h \
    transform/filtertransform.h \
    transform/graphtransform.h \
    transform/transformedgraph.h \
    ui/document.h \
    ui/graphcommoninteractor.h \
    ui/graphcomponentinteractor.h \
    ui/graphoverviewinteractor.h \
    ui/graphquickitem.h \
    ui/interactor.h \
    ui/searchmanager.h \
    ui/selectionmanager.h \
    utils/qmlcontainerwrapper.h \
    utils/qmlenum.h \
    utils/shadertools.h \
    graph/graphcomponent.h \
    transform/graphtransformconfig.h \
    transform/graphtransformconfigparser.h \
    transform/conditionfncreator.h \
    utils/qmlenum.h \
    layout/fastinitiallayout.h \
    rendering/primitives/arrow.h \
    ui/visualisations/colorvisualisationchannel.h \
    ui/visualisations/elementvisual.h \
    ui/visualisations/visualisationchannel.h \
    transform/fieldtype.h \
    ui/visualisations/sizevisualisationchannel.h \
    ui/visualisations/textvisualisationchannel.h \
    commands/applyvisualisationscommand.h \
    ui/visualisations/visualisationconfig.h \
    ui/visualisations/visualisationconfigparser.h \
    graph/elementtype.h \
    ui/visualisations/visualisationbuilder.h \
    attributes/conditionfncreator.h \
    attributes/condtionfnops.h \
    attributes/attribute.h \
    attributes/valuetype.h \
    ui/visualisations/colorgradient.h \
    crashtype.h \
    ui/visualisations/visualisationinfo.h \
    ui/alert.h \
    transform/transforminfo.h

SOURCES += \
    application.cpp \
    commands/applytransformationscommand.cpp \
    commands/commandmanager.cpp \
    commands/deleteselectednodescommand.cpp \
    graph/componentmanager.cpp \
    graph/graph.cpp \
    graph/graphconsistencychecker.cpp \
    graph/graphmodel.cpp \
    graph/mutablegraph.cpp \
    layout/barneshuttree.cpp \
    layout/centreinglayout.cpp \
    layout/circlepackcomponentlayout.cpp \
    layout/collision.cpp \
    layout/componentlayout.cpp \
    layout/forcedirectedlayout.cpp \
    layout/layout.cpp \
    layout/layoutsettings.cpp \
    layout/nodepositions.cpp \
    layout/powerof2gridcomponentlayout.cpp \
    layout/randomlayout.cpp \
    layout/scalinglayout.cpp \
    loading/parserthread.cpp \
    main.cpp \
    maths/boundingbox.cpp \
    maths/boundingsphere.cpp \
    maths/conicalfrustum.cpp \
    maths/frustum.cpp \
    maths/plane.cpp \
    maths/ray.cpp \
    rendering/camera.cpp \
    rendering/compute/gpucomputethread.cpp \
    rendering/compute/sdfcomputejob.cpp \
    rendering/glyphmap.cpp \
    rendering/graphcomponentrenderer.cpp \
    rendering/graphcomponentscene.cpp \
    rendering/graphoverviewscene.cpp \
    rendering/graphrenderer.cpp \
    rendering/opengldebuglogger.cpp \
    rendering/openglfunctions.cpp \
    rendering/primitives/rectangle.cpp \
    rendering/primitives/sphere.cpp \
    rendering/transition.cpp \
    transform/edgecontractiontransform.cpp \
    transform/filtertransform.cpp \
    transform/graphtransform.cpp \
    transform/transformedgraph.cpp \
    ui/document.cpp \
    ui/graphcommoninteractor.cpp \
    ui/graphcomponentinteractor.cpp \
    ui/graphoverviewinteractor.cpp \
    ui/graphquickitem.cpp \
    ui/searchmanager.cpp \
    ui/selectionmanager.cpp \
    transform/graphtransformconfigparser.cpp \
    transform/graphtransformconfig.cpp \
    layout/fastinitiallayout.cpp \
    rendering/primitives/arrow.cpp \
    ui/visualisations/colorvisualisationchannel.cpp \
    ui/visualisations/sizevisualisationchannel.cpp \
    ui/visualisations/textvisualisationchannel.cpp \
    commands/applyvisualisationscommand.cpp \
    ui/visualisations/visualisationconfig.cpp \
    ui/visualisations/visualisationconfigparser.cpp \
    graph/elementtype.cpp \
    attributes/attribute.cpp \
    ui/visualisations/colorgradient.cpp \
    attributes/conditionfncreator.cpp

RESOURCES += \
    icon/mainicon.qrc \
    icon-themes/icons.qrc \
    rendering/shaders.qrc \
    ui/qml.qrc

mac {
    LIBS += -framework CoreFoundation
}
