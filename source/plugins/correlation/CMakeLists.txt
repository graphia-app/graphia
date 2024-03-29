include(${CMAKE_CURRENT_SOURCE_DIR}/../../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty_headers.cmake)

set(PluginName "correlation")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/columnannotation.h
    ${CMAKE_CURRENT_LIST_DIR}/correlation.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatavector.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotsaveimagecommand.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationtype.h
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.h
    ${CMAKE_CURRENT_LIST_DIR}/graphsizeestimateplotitem.h
    ${CMAKE_CURRENT_LIST_DIR}/hierarchicalclusteringcommand.h
    ${CMAKE_CURRENT_LIST_DIR}/importannotationscommand.h
    ${CMAKE_CURRENT_LIST_DIR}/importannotationskeydetection.h
    ${CMAKE_CURRENT_LIST_DIR}/knnprotograph.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.h
    ${CMAKE_CURRENT_LIST_DIR}/normaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/qcpcolumnannotations.h
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/columnannotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatavector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_discrete.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_continuous.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_columnannotations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotsaveimagecommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationtype.cpp
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graphsizeestimateplotitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hierarchicalclusteringcommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/importannotationscommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/importannotationskeydetection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qcpcolumnannotations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/correlation_qml.qrc
)

if(LINK_TYPE STREQUAL STATIC)
    qt_add_plugin(${PluginName} ${LINK_TYPE} CLASS_NAME CorrelationPlugin
        ${SOURCES} ${HEADERS} ${RESOURCES})
else()
    qt_add_library(${PluginName} ${LINK_TYPE} ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} PRIVATE thirdparty shared)

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Qml Quick QuickControls2 OpenGL Svg Widgets Xml)
target_link_libraries(${PluginName} PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::OpenGL
    Qt6::Svg
    Qt6::Widgets
    Qt6::Xml
)
