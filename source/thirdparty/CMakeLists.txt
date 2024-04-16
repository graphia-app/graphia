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
include(${CMAKE_CURRENT_LIST_DIR}/zlib/zlib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xlsxio/xlsxio.cmake)

if(MSVC)
    # Need to put the shared library next to the binary, on Windows
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # Disable for breakpad
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
    endif()
endif()

if(LINK_TYPE STREQUAL STATIC)
    # In the static build case, thirdparty is static itself so we have no need for
    # thirdparty_static, therefore all the static sources get added to thirdparty whereas...
    list(APPEND SHARED_THIRDPARTY_HEADERS ${STATIC_THIRDPARTY_HEADERS})
    list(APPEND SHARED_THIRDPARTY_SOURCES ${STATIC_THIRDPARTY_SOURCES})
    list(APPEND SHARED_THIRDPARTY_RESOURCES ${STATIC_THIRDPARTY_RESOURCES})

    # ...thirdparty_static is now redundant so it's just made empty so as not to disrupt the rest of the build
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_static_dummy.cpp "// This file is intentionally left blank\n")
    qt_add_library(thirdparty_static STATIC ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_static_dummy.cpp)
else()
    qt_add_library(thirdparty_static STATIC
        ${STATIC_THIRDPARTY_SOURCES}
        ${STATIC_THIRDPARTY_HEADERS}
        ${STATIC_THIRDPARTY_RESOURCES})
endif()

qt_add_library(thirdparty ${LINK_TYPE}
    ${SHARED_THIRDPARTY_SOURCES}
    ${SHARED_THIRDPARTY_HEADERS}
    ${SHARED_THIRDPARTY_RESOURCES})

target_link_libraries(thirdparty PRIVATE ${CMAKE_DL_LIBS})

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

    # Required by HDF5
    target_link_libraries(thirdparty PRIVATE shlwapi.lib)
else()
    set_target_properties(thirdparty PROPERTIES COMPILE_FLAGS "-w")
    set_target_properties(thirdparty_static PROPERTIES COMPILE_FLAGS "-w")
endif()

find_package(Qt6 COMPONENTS REQUIRED Core Qml Quick QuickControls2 Network Widgets PrintSupport)
target_link_libraries(thirdparty PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Widgets
    Qt6::PrintSupport
)
target_link_libraries(thirdparty_static PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Widgets
)

if(UNIX AND NOT APPLE)
    install(TARGETS thirdparty LIBRARY DESTINATION lib)
endif()
