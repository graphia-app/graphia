cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(${CMAKE_CURRENT_LIST_DIR}/breakpad/breakpad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cryptopp/cryptopp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/qcustomplot/qcustomplot.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/qt-qml-models/QtQmlModels.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/qtsingleapplication/qtsingleapplication.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/SortFilterProxyModel/SortFilterProxyModel.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/zlib/zlib.cmake)

add_library(thirdparty SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

# Disable all warnings
if(MSVC)
    set_target_properties(thirdparty PROPERTIES COMPILE_FLAGS "/w")

    # Filter out any /W flags that have been added by CMake
    string(REGEX REPLACE "\/W[0-9]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "\/W[0-9]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    set_target_properties(thirdparty PROPERTIES COMPILE_FLAGS "-w")
endif()

find_package(Qt5 COMPONENTS Core Qml Network Widgets PrintSupport REQUIRED)
target_link_libraries(thirdparty
    Qt5::Core
    Qt5::Qml
    Qt5::Network
    Qt5::Widgets
    Qt5::PrintSupport
)

if(APPLE)
    target_link_libraries(thirdparty "-framework CoreFoundation")
endif()
