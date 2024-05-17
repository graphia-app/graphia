include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(PluginName "correlation")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Qml Quick QuickControls2 OpenGL Svg Widgets Xml)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/columnannotation.h
    ${CMAKE_CURRENT_LIST_DIR}/correlation.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatavector.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationtype.h
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.h
    ${CMAKE_CURRENT_LIST_DIR}/hierarchicalclusteringcommand.h
    ${CMAKE_CURRENT_LIST_DIR}/importannotationscommand.h
    ${CMAKE_CURRENT_LIST_DIR}/knnprotograph.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.h
    ${CMAKE_CURRENT_LIST_DIR}/normaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/softmaxnormaliser.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/columnannotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatavector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hierarchicalclusteringcommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/importannotationscommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/softmaxnormaliser.cpp
)

if(LINK_TYPE STREQUAL STATIC)
    qt_add_plugin(${PluginName} ${LINK_TYPE} CLASS_NAME CorrelationPlugin
        ${SOURCES} ${HEADERS})
else()
    qt_add_library(${PluginName} ${LINK_TYPE} ${SOURCES} ${HEADERS})
endif()

add_subdirectory(ui/qml/Graphia/Plugins/Correlation)

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} PRIVATE correlation_qmlplugin thirdparty shared)

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
