include(${CMAKE_CURRENT_SOURCE_DIR}/../../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty_headers.cmake)

set(PluginName "websearch")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/websearchplugin.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/websearchplugin.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/websearch_qml.qrc
)

if(LINK_TYPE STREQUAL STATIC)
    qt_add_plugin(${PluginName} ${LINK_TYPE} CLASS_NAME WebSearchPlugin
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

if(NOT EMSCRIPTEN)
    find_package(Qt6 COMPONENTS REQUIRED WebEngineQuick)
    target_link_libraries(${PluginName} PRIVATE Qt6::WebEngineQuick)
endif()
