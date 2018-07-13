include(${CMAKE_CURRENT_SOURCE_DIR}/../../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty_headers.cmake)

set(PluginName "skeleton")

add_definitions(-DQT_PLUGIN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/skeletonplugin.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/skeletonplugin.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/skeleton_qml.qrc
)

add_library(${PluginName} SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

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
