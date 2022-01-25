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
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatarow.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/correlationtype.h
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.h
    ${CMAKE_CURRENT_LIST_DIR}/graphsizeestimateplotitem.h
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.h
    ${CMAKE_CURRENT_LIST_DIR}/normaliser.h
    ${CMAKE_CURRENT_LIST_DIR}/qcpcolumnannotations.h
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/columnannotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationdatarow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationnodeattributetablemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_discrete.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_continuous.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplotitem_columnannotations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/correlationplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/featurescaling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/graphsizeestimateplotitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loading/correlationfileparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qcpcolumnannotations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/quantilenormaliser.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui/correlation_qml.qrc
)

add_library(${PluginName} SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

if(NOT APPLE AND NOT MSVC)
    install(TARGETS ${PluginName} LIBRARY DESTINATION lib/${PROJECT_NAME}/plugins)
endif()

target_link_libraries(${PluginName} thirdparty shared)

find_package(Qt5 COMPONENTS Core Qml Quick OpenGL OpenGLExtensions PrintSupport Svg Widgets Xml REQUIRED)
target_link_libraries(${PluginName}
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::OpenGL
    Qt5::OpenGLExtensions
    Qt5::PrintSupport
    Qt5::Svg
    Qt5::Widgets
    Qt5::Xml
)
