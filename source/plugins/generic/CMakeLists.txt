include(${CMAKE_CURRENT_SOURCE_DIR}/../../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty.cmake)

set(PluginName "generic")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/genericplugin.h
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/qml.qrc
)

add_library(${PluginName} SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(${PluginName} thirdparty shared)

find_package(Qt5 COMPONENTS Core Qml Quick OpenGL OpenGLExtensions PrintSupport Widgets Xml REQUIRED)
target_link_libraries(${PluginName}
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::OpenGL
    Qt5::OpenGLExtensions
    Qt5::PrintSupport
    Qt5::Widgets
    Qt5::Xml
)
