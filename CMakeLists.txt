cmake_minimum_required(VERSION 3.5)

if(NOT "$ENV{PRODUCT_NAME}" STREQUAL "")
    project($ENV{PRODUCT_NAME} LANGUAGES C CXX)
else()
    project("Graphia" LANGUAGES C CXX)
endif()

option(UNITY_BUILD "Perform a unity build" OFF)

find_package(Qt5Widgets REQUIRED)
if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.14.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.14!")
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)

include_directories(source)

add_subdirectory(source/thirdparty)
add_subdirectory(source/shared)
add_subdirectory(source/app)
add_subdirectory(source/plugins)
add_subdirectory(source/crashreporter)
add_subdirectory(source/messagebox)
add_subdirectory(source/updater)
add_subdirectory(source/updater/editor)
