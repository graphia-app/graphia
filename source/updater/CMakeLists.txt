include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/thirdparty_headers.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/../shared/utils/preferences.h
    ${CMAKE_CURRENT_LIST_DIR}/installer.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/installer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/resources.qrc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    qt_add_executable(Updater WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
else()
    qt_add_executable(Updater ${SOURCES} ${HEADERS} ${RESOURCES})
    install(TARGETS Updater RUNTIME DESTINATION bin)
endif()

find_package(Qt6 COMPONENTS REQUIRED Concurrent Qml Quick QuickControls2 Xml Widgets)
target_link_libraries(Updater PRIVATE
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Xml
    Qt6::Widgets
)

target_link_libraries(Updater PRIVATE shared controls_qmlplugin utils_qmlplugin)
