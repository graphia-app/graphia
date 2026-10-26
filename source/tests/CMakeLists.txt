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

add_unit_test(tst_mutablegraph graph/tst_mutablegraph.cpp)
add_unit_test(tst_graphtransformconfigparser transform/tst_graphtransformconfigparser.cpp)
add_unit_test(tst_attribute attributes/tst_attribute.cpp)
add_unit_test(tst_conditionfncreator attributes/tst_conditionfncreator.cpp)
add_unit_test(tst_enrichmentcalculator attributes/tst_enrichmentcalculator.cpp)
add_unit_test(tst_enrichmenttablemodel attributes/tst_enrichmenttablemodel.cpp)
add_unit_test(tst_structuraltransforms transform/tst_structuraltransforms.cpp)
add_unit_test(tst_analysistransforms transform/tst_analysistransforms.cpp)
add_unit_test(tst_attributetransforms transform/tst_attributetransforms.cpp)
add_unit_test(tst_transformcache transform/tst_transformcache.cpp)
add_unit_test(tst_transformfactories transform/tst_transformfactories.cpp)

add_unit_test(tst_correlation correlation/tst_correlation.cpp)
target_link_libraries(tst_correlation PRIVATE correlation_core)

# The correlation plot's column annotations, which are built into the plugin's QML module
add_unit_test(tst_columnannotations correlation/tst_columnannotations.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/correlation/columnannotation.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/correlation/ui/qml/Graphia/Plugins/Correlation/qcpcolumnannotations.cpp)

add_unit_test(tst_tabulardata loading/tst_tabulardata.cpp)
add_unit_test(tst_tabulardataparser loading/tst_tabulardataparser.cpp)
add_unit_test(tst_graphparsers loading/tst_graphparsers.cpp)
add_unit_test(tst_savers loading/tst_savers.cpp)
add_unit_test(tst_nativeloader loading/tst_nativeloader.cpp)
add_unit_test(tst_commands commands/tst_commands.cpp)
add_unit_test(tst_visualisationconfigparser ui/tst_visualisationconfigparser.cpp)

add_unit_test(tst_string utils/tst_string.cpp)

if(UNIX AND NOT APPLE)
    # Qt takes the system locale from the environment on Linux, so also run in a
    # locale whose numbers differ from the C locale's in every respect
    add_test(NAME tst_string_sv_SE COMMAND tst_string)
    set_tests_properties(tst_string_sv_SE PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LC_ALL=sv_SE.UTF-8")
endif()

add_unit_test(tst_statistics utils/tst_statistics.cpp)
add_unit_test(tst_container utils/tst_container.cpp)
add_unit_test(tst_datastructures utils/tst_datastructures.cpp)
add_unit_test(tst_json utils/tst_json.cpp)
add_unit_test(tst_color utils/tst_color.cpp)
add_unit_test(tst_colorvisualisations visualisations/tst_colorvisualisations.cpp)
add_unit_test(tst_visualisationmapping visualisations/tst_visualisationmapping.cpp)
add_unit_test(tst_visualisations visualisations/tst_visualisations.cpp)
add_unit_test(tst_commandmanager commands/tst_commandmanager.cpp)
add_unit_test(tst_graphsizeestimate loading/tst_graphsizeestimate.cpp)
add_unit_test(tst_random utils/tst_random.cpp)
