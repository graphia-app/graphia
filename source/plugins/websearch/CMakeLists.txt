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

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/websearch_qml.qrc
)

add_library(${PluginName} SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} thirdparty shared)

find_package(Qt5 COMPONENTS Core Qml Quick QuickControls2 OpenGL OpenGLExtensions Svg PrintSupport Widgets Xml REQUIRED)
target_link_libraries(${PluginName}
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::OpenGL
    Qt5::OpenGLExtensions
    Qt5::Svg
    Qt5::PrintSupport
    Qt5::Widgets
    Qt5::Xml
)
