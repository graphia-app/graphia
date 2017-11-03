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

add_library(thirdparty STATIC ${SOURCES} ${HEADERS} ${RESOURCES})

find_package(Qt5 COMPONENTS Core Qml Network Widgets REQUIRED)
target_link_libraries(thirdparty
    Qt5::Core
    Qt5::Qml
    Qt5::Network
    Qt5::Widgets
)
