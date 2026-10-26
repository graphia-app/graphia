include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# The Qt targets app_core links against must also be visible here, otherwise
# Qt can't walk the tests' dependencies when finalising the executables
find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Qml Quick QuickControls2 OpenGL Svg Widgets Xml Network Test)
if(NOT EMSCRIPTEN)
    find_package(Qt6 REQUIRED COMPONENTS WebEngineQuick)
endif()

if(WIN32)
    # A DLL exports only what is marked for export, so each test links its own copy
    set(APP_CORE_FOR_TESTS app_core)
else()
    # One copy of the application's code for all the tests, rather than one in each
    add_library(app_core_shared SHARED)
    target_link_libraries(app_core_shared PRIVATE app_core)

    # Whole, as the tests call into them more widely than the application does
    target_link_libraries(app_core_shared PRIVATE
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,shared>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,thirdparty_static>")

    set(APP_CORE_FOR_TESTS app_core_shared thirdparty
        Qt6::Concurrent Qt6::Core Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::OpenGL
        Qt6::Svg Qt6::Widgets Qt6::Xml Qt6::Network)
endif()

function(add_unit_test NAME)
    qt_add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} PRIVATE ${APP_CORE_FOR_TESTS} Qt6::Test)

    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    if(WIN32)
        # Windows has no rpath, so the shared libraries in the root of the build need to be on PATH
        set_tests_properties(${NAME} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}")
    endif()
endfunction()
