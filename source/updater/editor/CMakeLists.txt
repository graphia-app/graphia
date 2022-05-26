include(${CMAKE_CURRENT_SOURCE_DIR}/../../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/thirdparty_headers.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/../../shared/utils/preferences.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/resources.qrc
    ${CMAKE_CURRENT_LIST_DIR}/../../shared/ui/shared.qrc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    add_executable(UpdateEditor WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
else()
    add_executable(UpdateEditor ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

find_package(Qt6 COMPONENTS REQUIRED Qml Quick QuickControls2 Xml Widgets)
target_link_libraries(UpdateEditor
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Xml
    Qt6::Widgets
)

target_link_libraries(UpdateEditor shared)
