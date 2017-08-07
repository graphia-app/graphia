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
    attributes/attribute.h \
    attributes/availableattributesmodel.h \
    attributes/conditionfncreator.h \
    attributes/condtionfnops.h \
    auth/auth.h \
    commands/applytransformscommand.h \
    commands/applyvisualisationscommand.h \
    commands/commandmanager.h \
    commands/deleteselectednodescommand.h \
    commands/selectnodescommand.h \
    crashtype.h \
    graph/componentmanager.h \
    graph/elementiddistinctsetcollection.h \
    graph/filter.h \
    graph/graphcomponent.h \
    graph/graphconsistencychecker.h \
    graph/graph.h \
    graph/graphmodel.h \
    graph/mutablegraph.h \
    layout/barneshuttree.h \
    layout/centreinglayout.h \
    layout/circlepackcomponentlayout.h \
    layout/collision.h \
    layout/componentlayout.h \
    layout/fastinitiallayout.h \
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
    rendering/primitives/arrow.h \
    rendering/primitives/rectangle.h \
    rendering/primitives/sphere.h \
    rendering/scene.h \
    rendering/shadertools.h \
    rendering/transition.h \
    transform/graphtransformconfig.h \
    transform/graphtransformconfigparser.h \
    transform/graphtransform.h \
    transform/graphtransformparameter.h \
    transform/transformcache.h \
    transform/transformedgraph.h \
    transform/transforminfo.h \
    transform/transforms/eccentricitytransform.h \
    transform/transforms/edgecontractiontransform.h \
    transform/transforms/filtertransform.h \
    transform/transforms/mcltransform.h \
    transform/transforms/pageranktransform.h \
    ui/alert.h \
    ui/document.h \
    ui/graphcommoninteractor.h \
    ui/graphcomponentinteractor.h \
    ui/graphoverviewinteractor.h \
    ui/graphquickitem.h \
    ui/interactor.h \
    ui/searchmanager.h \
    ui/selectionmanager.h \
    ui/visualisations/colorgradient.h \
    ui/visualisations/colorvisualisationchannel.h \
    ui/visualisations/defaultgradients.h \
    ui/visualisations/elementvisual.h \
    ui/visualisations/sizevisualisationchannel.h \
    ui/visualisations/textvisualisationchannel.h \
    ui/visualisations/visualisationbuilder.h \
    ui/visualisations/visualisationchannel.h \
    ui/visualisations/visualisationconfig.h \
    ui/visualisations/visualisationconfigparser.h \
    ui/visualisations/visualisationinfo.h \
    transform/availabletransformsmodel.h

SOURCES += \
    application.cpp \
    auth/auth.cpp \
    attributes/attribute.cpp \
    attributes/availableattributesmodel.cpp \
    attributes/conditionfncreator.cpp \
    commands/applytransformscommand.cpp \
    commands/applyvisualisationscommand.cpp \
    commands/commandmanager.cpp \
    commands/deleteselectednodescommand.cpp \
    graph/componentmanager.cpp \
    graph/graphconsistencychecker.cpp \
    graph/graph.cpp \
    graph/graphmodel.cpp \
    graph/mutablegraph.cpp \
    layout/barneshuttree.cpp \
    layout/centreinglayout.cpp \
    layout/circlepackcomponentlayout.cpp \
    layout/collision.cpp \
    layout/componentlayout.cpp \
    layout/fastinitiallayout.cpp \
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
    rendering/primitives/arrow.cpp \
    rendering/primitives/rectangle.cpp \
    rendering/primitives/sphere.cpp \
    rendering/transition.cpp \
    transform/graphtransformconfig.cpp \
    transform/graphtransformconfigparser.cpp \
    transform/graphtransform.cpp \
    transform/transformcache.cpp \
    transform/transformedgraph.cpp \
    transform/transforms/eccentricitytransform.cpp \
    transform/transforms/edgecontractiontransform.cpp \
    transform/transforms/filtertransform.cpp \
    transform/transforms/mcltransform.cpp \
    transform/transforms/pageranktransform.cpp \
    ui/document.cpp \
    ui/graphcommoninteractor.cpp \
    ui/graphcomponentinteractor.cpp \
    ui/graphoverviewinteractor.cpp \
    ui/graphquickitem.cpp \
    ui/searchmanager.cpp \
    ui/selectionmanager.cpp \
    ui/visualisations/colorgradient.cpp \
    ui/visualisations/colorvisualisationchannel.cpp \
    ui/visualisations/sizevisualisationchannel.cpp \
    ui/visualisations/textvisualisationchannel.cpp \
    ui/visualisations/visualisationconfig.cpp \
    ui/visualisations/visualisationconfigparser.cpp \
    transform/availabletransformsmodel.cpp

RESOURCES += \
    icon/mainicon.qrc \
    icon-themes/icons.qrc \
    auth/keys.qrc \
    rendering/shaders.qrc \
    ui/qml.qrc

mac {
    LIBS += -framework CoreFoundation
}
