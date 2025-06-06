include(${CMAKE_SOURCE_DIR}/source/common.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")
add_definitions(-DAPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/ui/licensing/generate-OSS.sh
    ${CMAKE_CURRENT_LIST_DIR}/ui/licensing/update-license.pl
    ${CMAKE_CURRENT_LIST_DIR}/icon-themes/generate-icons-qrc.sh
    ${CMAKE_CURRENT_LIST_DIR}/icon/generate.sh
)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/application.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/attribute.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/attributeedits.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/conditionfncreator.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/condtionfnops.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/enrichmentcalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/attributes/enrichmenttablemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/applytransformscommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/applyvisualisationscommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/cloneattributecommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/commandmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/deletenodescommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/editattributecommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/importattributescommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/removeattributescommand.h
    ${CMAKE_CURRENT_LIST_DIR}/commands/selectnodescommand.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/componentmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/elementiddistinctsetcollection_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/elementiddistinctsetcollection.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphcomponent.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphconsistencychecker.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphfilter.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/graph.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/mutablegraph.h
    ${CMAKE_CURRENT_LIST_DIR}/graph/qmlelementid.h
    ${CMAKE_CURRENT_LIST_DIR}/headless.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/barneshuttree.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/centreinglayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/circlepackcomponentlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/collision.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/componentlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/fastinitiallayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/forcedirectedlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutsettings.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/nodepositions.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/powerof2gridcomponentlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/randomlayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/scalinglayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/sequencelayout.h
    ${CMAKE_CURRENT_LIST_DIR}/layout/spatialtree.h
    ${CMAKE_CURRENT_LIST_DIR}/limitconstants.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/gmlsaver.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/graphmlsaver.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/isaver.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/jsongraphsaver.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/nativeloader.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/parserthread.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/pairwisesaver.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/saverfactory.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/nativesaver.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/boundingbox.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/boundingsphere.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/circle.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/conicalfrustum.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/frustum.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/interpolation.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/line.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/plane.h
    ${CMAKE_CURRENT_LIST_DIR}/maths/ray.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/camera.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/doublebufferedtexture.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/glyphmap.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphcomponentrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphcomponentscene.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphoverviewscene.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphrenderercore.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphrenderertypes.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/opengldebuglogger.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/openglfunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/arrow.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/rectangle.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/sphere.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/scene.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/screenshotrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/shadertools.h
    ${CMAKE_CURRENT_LIST_DIR}/rendering/transition.h
    ${CMAKE_CURRENT_LIST_DIR}/preferences.h
    ${CMAKE_CURRENT_LIST_DIR}/tracking.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformattributeparameter.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformconfigparser.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformparameter.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transformcache.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transformedgraph.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforminfo.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/attributesynthesistransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/averageattributetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/betweennesstransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/conditionalattributetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/contractbyattributetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/combineattributestransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/eccentricitytransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/edgecontractiontransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/edgereductiontransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/forwardmultielementattributetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/separatebyattributetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/knntransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/louvaintransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/percentnntransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/filtertransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/mcltransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/pageranktransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/spanningtreetransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/typecasttransform.h
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/removeleavestransform.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/alert.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/document.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/findtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphcommoninteractor.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphcomponentinteractor.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphoverviewinteractor.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/interactor.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/searchmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/selectionmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/colorvisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/elementvisual.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/sizevisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/sharedtextvisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textvisual.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textvisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textcolorvisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textsizevisualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationbuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationconfigparser.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationinfo.h
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationmapping.h
    ${CMAKE_CURRENT_LIST_DIR}/watchdog.h
)

list(APPEND APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/attributes/attribute.cpp
    ${CMAKE_CURRENT_LIST_DIR}/attributes/conditionfncreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/attributes/enrichmentcalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/attributes/enrichmenttablemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/applytransformscommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/applyvisualisationscommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/cloneattributecommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/commandmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/deletenodescommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/editattributecommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/importattributescommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commands/removeattributescommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graph/componentmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphconsistencychecker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graph/graph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graph/graphmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graph/mutablegraph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/headless.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/centreinglayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/circlepackcomponentlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/collision.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/componentlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/fastinitiallayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/forcedirectedlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/layoutsettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/nodepositions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/powerof2gridcomponentlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/randomlayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layout/scalinglayout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/gmlsaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/graphmlsaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/jsongraphsaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/nativeloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/parserthread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/pairwisesaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/nativesaver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/saverfactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/boundingbox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/boundingsphere.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/conicalfrustum.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/frustum.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/plane.cpp
    ${CMAKE_CURRENT_LIST_DIR}/maths/ray.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/doublebufferedtexture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/glyphmap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphcomponentrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphcomponentscene.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphoverviewscene.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/graphrenderercore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/opengldebuglogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/openglfunctions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/arrow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/rectangle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/primitives/sphere.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/screenshotrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/shadertools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rendering/transition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tracking.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformconfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransformconfigparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/graphtransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transformcache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transformedgraph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/attributesynthesistransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/averageattributetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/betweennesstransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/conditionalattributetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/contractbyattributetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/combineattributestransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/eccentricitytransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/edgecontractiontransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/edgereductiontransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/forwardmultielementattributetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/separatebyattributetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/knntransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/louvaintransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/percentnntransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/filtertransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/mcltransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/pageranktransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/spanningtreetransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/typecasttransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/removeleavestransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphcommoninteractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphcomponentinteractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/graphoverviewinteractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/searchmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/selectionmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/colorvisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/sizevisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/sharedtextvisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textvisual.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textvisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textcolorvisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/textsizevisualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationconfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationconfigparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui/visualisations/visualisationmapping.cpp
    ${CMAKE_CURRENT_LIST_DIR}/watchdog.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/icon/mainicon.qrc
    ${CMAKE_CURRENT_LIST_DIR}/icon-themes/icons.qrc
    ${CMAKE_CURRENT_LIST_DIR}/rendering/shaders.qrc
    ${CMAKE_CURRENT_LIST_DIR}/transform/transforms/images.qrc
    ${CMAKE_CURRENT_LIST_DIR}/ui/imagery.qrc
    ${CMAKE_CURRENT_LIST_DIR}/ui/licensing.qrc
)

if(NOT EMSCRIPTEN)
    list(APPEND HEADERS ${CMAKE_CURRENT_LIST_DIR}/updates/updater.h)
    list(APPEND APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/updates/updater.cpp)
endif()

if(EMSCRIPTEN)
    list(APPEND RESOURCES ${CMAKE_CURRENT_LIST_DIR}/wasm_resource_files.qrc)
endif()

set(QML_IMPORT_PATH "${QML_IMPORT_PATH};${CMAKE_CURRENT_LIST_DIR}/ui/qml/;${CMAKE_CURRENT_BINARY_DIR}/ui/qml/" PARENT_SCOPE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

list(APPEND SOURCES ${APP_SOURCES})

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE Icon.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER app.graphia.${PROJECT_NAME})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${Version})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${Version})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${Version})
    set(MACOSX_BUNDLE_COPYRIGHT ${Copyright})
    string(REPLACE "(c)" "©" MACOSX_BUNDLE_COPYRIGHT "${MACOSX_BUNDLE_COPYRIGHT}")
    string(REPLACE "\\n" "\n" MACOSX_BUNDLE_COPYRIGHT "${MACOSX_BUNDLE_COPYRIGHT}")
    set(MACOSX_BUNDLE_NATIVE_EXTENSION ${NativeExtension})
    set(MACOSX_BUNDLE_BUNDLE_URL_NAME app.graphia.${PROJECT_NAME})
    set(MACOSX_BUNDLE_BUNDLE_URL_SCHEME ${NativeExtension})
    set(ICON ${CMAKE_CURRENT_LIST_DIR}/icon/Icon.icns)
    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES} ${ICON})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/misc/MacOSXBundleInfo.plist.in)
elseif(MSVC)
    set(ICON ${CMAKE_CURRENT_LIST_DIR}/icon/Icon.rc)

    qt_add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} ${ICON})
else()
    qt_add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

    foreach(RESOLUTION 1024x1024 512x512 256x256 128x128 64x64 48x48 32x32 16x16)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/icon/Icon${RESOLUTION}.png
            DESTINATION share/icons/hicolor/${RESOLUTION}/apps
            RENAME ${PROJECT_NAME}.png)
    endforeach()

    install(FILES ${CMAKE_CURRENT_LIST_DIR}/icon/Icon.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME ${PROJECT_NAME}.svg)

    configure_file(${CMAKE_CURRENT_LIST_DIR}/application.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop
        DESTINATION share/applications)
endif()

if(EMSCRIPTEN)
    set_property(TARGET ${PROJECT_NAME} PROPERTY QT_WASM_EXTRA_EXPORTED_METHODS "ENV")
endif()

if(UNIX)
    file(WRITE ${PROJECT_BINARY_DIR}/variables.sh "export PRODUCT_NAME=\"${PROJECT_NAME}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.sh "export VERSION=\"${Version}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.sh "export COPYRIGHT=\"${Copyright}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.sh "export PUBLISHER=\"${Publisher}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.sh "export NATIVE_EXTENSION=\"${NativeExtension}\"\n")
elseif(MSVC)
    file(WRITE ${PROJECT_BINARY_DIR}/variables.ps1 "\$PRODUCT_NAME=\"${PROJECT_NAME}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.ps1 "\$VERSION=\"${Version}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.ps1 "\$COPYRIGHT=\"${Copyright}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.ps1 "\$PUBLISHER=\"${Publisher}\"\n")
    file(APPEND ${PROJECT_BINARY_DIR}/variables.ps1 "\$NATIVE_EXTENSION=\"${NativeExtension}\"\n")
endif()

add_subdirectory(ui/qml/Graphia)
target_link_libraries(${PROJECT_NAME} PRIVATE main_qmlplugin controls_qmlplugin loading_qmlplugin plugins_qmlplugin utils_qmlplugin)

target_link_libraries(${PROJECT_NAME} PRIVATE shared sharedtypes_qmlplugin)
target_link_libraries(${PROJECT_NAME} PRIVATE thirdparty_static thirdparty)

if(LINK_TYPE STREQUAL STATIC)
    # Link with all of the available plugins in the static build case
    get_property(PLUGIN_DIRECTORIES DIRECTORY "${PROJECT_SOURCE_DIR}/source/plugins" PROPERTY SUBDIRECTORIES)

    foreach(PLUGIN_DIRECTORY IN LISTS PLUGIN_DIRECTORIES)
        get_property(PLUGIN_TARGET DIRECTORY "${PLUGIN_DIRECTORY}" PROPERTY BUILDSYSTEM_TARGETS)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${PLUGIN_TARGET})
    endforeach()
endif()

find_package(Qt6 COMPONENTS REQUIRED Concurrent Core Qml Quick QuickControls2 OpenGL Svg Widgets Xml Network)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::OpenGL
    Qt6::Svg
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
)

if(NOT EMSCRIPTEN AND Qt6_VERSION VERSION_LESS "6.8.0")
    find_package(Qt6 COMPONENTS REQUIRED WebEngineQuick)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::WebEngineQuick)
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_gl_LIBRARY})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

add_definitions(-DQCUSTOMPLOT_USE_LIBRARY)
