include(${CMAKE_SOURCE_DIR}/source/common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty_headers.cmake)

set(PluginName "generic")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Qml Quick QuickControls2 OpenGL Svg Widgets Xml)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/genericplugin.h
)

if(LINK_TYPE STREQUAL STATIC)
    qt_add_plugin(${PluginName} ${LINK_TYPE} CLASS_NAME GenericPlugin
        ${SOURCES} ${HEADERS})
else()
    qt_add_library(${PluginName} ${LINK_TYPE} ${SOURCES} ${HEADERS})
endif()

set(GENERIC_PLUGIN_QML_MODULE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/ui/qml/app/graphia/Plugins/Generic" PARENT_SCOPE)

add_subdirectory(ui/qml/app/graphia/Plugins/Generic)

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} PRIVATE generic_qmlplugin thirdparty shared)

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
