include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(PluginName "websearch")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Qml Quick QuickControls2 OpenGL Svg Widgets Xml)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/websearchplugin.h
)

if(LINK_TYPE STREQUAL STATIC)
    qt_add_plugin(${PluginName} ${LINK_TYPE} CLASS_NAME WebSearchPlugin
        ${SOURCES} ${HEADERS})
else()
    qt_add_library(${PluginName} ${LINK_TYPE} ${SOURCES} ${HEADERS})
endif()

add_subdirectory(ui/qml/Graphia/Plugins/WebSearch)

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} PRIVATE websearch_qmlplugin thirdparty shared)

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

if(NOT EMSCRIPTEN AND Qt6_VERSION VERSION_LESS "6.8.0")
    find_package(Qt6 COMPONENTS REQUIRED WebEngineQuick)
    target_link_libraries(${PluginName} PRIVATE Qt6::WebEngineQuick)
endif()
