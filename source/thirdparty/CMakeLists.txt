cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty_headers.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(${CMAKE_CURRENT_LIST_DIR}/breakpad/breakpad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cryptopp/cryptopp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/expat/expat.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/qcustomplot/qcustomplot.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/matio/matio.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/hdf5/hdf5.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/qtsingleapplication/qtsingleapplication.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/SortFilterProxyModel/SortFilterProxyModel.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/zlib/zlib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xlsxio/xlsxio.cmake)

if(MSVC)
    # Need to put the shared library next to the binary, on Windows
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_library(thirdparty_static STATIC
    ${STATIC_THIRDPARTY_SOURCES}
    ${STATIC_THIRDPARTY_HEADERS}
    ${STATIC_THIRDPARTY_RESOURCES})

add_library(thirdparty SHARED
    ${SHARED_THIRDPARTY_SOURCES}
    ${SHARED_THIRDPARTY_HEADERS}
    ${SHARED_THIRDPARTY_RESOURCES})

# Disable unity build for third party files
foreach(FILE IN LISTS STATIC_THIRDPARTY_SOURCES)
    set_property(SOURCE ${FILE} PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)
endforeach()

foreach(FILE IN LISTS SHARED_THIRDPARTY_SOURCES)
    set_property(SOURCE ${FILE} PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)
endforeach()

# Disable all warnings
if(MSVC)
    set_target_properties(thirdparty PROPERTIES COMPILE_FLAGS "/w")
    set_target_properties(thirdparty_static PROPERTIES COMPILE_FLAGS "/w")

    # Filter out any /W flags that have been added by CMake
    string(REGEX REPLACE "\/W[0-9]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "\/W[0-9]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # Don't use /analyze when building third party code
    string(REPLACE "${VC_ANALYZE_FLAGS}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "${VC_ANALYZE_FLAGS}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    set_target_properties(thirdparty PROPERTIES COMPILE_FLAGS "-w")
    set_target_properties(thirdparty_static PROPERTIES COMPILE_FLAGS "-w")
endif()

find_package(Qt5 COMPONENTS Core Qml Quick QuickControls2 Network Widgets PrintSupport REQUIRED)
target_link_libraries(thirdparty
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Network
    Qt5::Widgets
    Qt5::PrintSupport
)
target_link_libraries(thirdparty_static
    Qt5::Core
    Qt5::Qml
    Qt5::Widgets
)

if(APPLE)
    target_link_libraries(thirdparty "-framework CoreFoundation")
elseif(MSVC)
    #NOP
else()
    install(TARGETS thirdparty LIBRARY DESTINATION lib)
endif()
