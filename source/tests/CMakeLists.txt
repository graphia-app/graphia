include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# The Qt targets appcore links against must also be visible here, otherwise
# Qt can't walk the tests' dependencies when finalising the executables
find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Qml Quick QuickControls2 OpenGL Svg Widgets Xml Network Test)
if(NOT EMSCRIPTEN)
    find_package(Qt6 REQUIRED COMPONENTS WebEngineQuick)
endif()

function(add_graphia_test NAME)
    qt_add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} PRIVATE appcore Qt6::Test)

    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    if(WIN32)
        # Windows has no rpath, so the shared libraries in the root of the build need to be on PATH
        set_tests_properties(${NAME} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}")
    endif()
endfunction()

add_graphia_test(tst_mutablegraph graph/tst_mutablegraph.cpp)
add_graphia_test(tst_graphtransformconfigparser transform/tst_graphtransformconfigparser.cpp)
add_graphia_test(tst_conditionfncreator attributes/tst_conditionfncreator.cpp)
add_graphia_test(tst_enrichmenttablemodel attributes/tst_enrichmenttablemodel.cpp)
add_graphia_test(tst_structuraltransforms transform/tst_structuraltransforms.cpp)
add_graphia_test(tst_analysistransforms transform/tst_analysistransforms.cpp)
add_graphia_test(tst_attributetransforms transform/tst_attributetransforms.cpp)

add_graphia_test(tst_correlation correlation/tst_correlation.cpp)
target_link_libraries(tst_correlation PRIVATE correlationcore)

add_graphia_test(tst_tabulardata loading/tst_tabulardata.cpp)
add_graphia_test(tst_graphparsers loading/tst_graphparsers.cpp)
add_graphia_test(tst_savers loading/tst_savers.cpp)
add_graphia_test(tst_nativeloader loading/tst_nativeloader.cpp)
