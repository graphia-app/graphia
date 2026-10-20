include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# The Qt targets appcore links against must also be visible here, otherwise
# Qt can't walk the tests' dependencies when finalising the executables
find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Qml Quick QuickControls2 OpenGL Svg Widgets Xml Network Test QuickTest)
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
add_graphia_test(tst_attribute attributes/tst_attribute.cpp)
add_graphia_test(tst_conditionfncreator attributes/tst_conditionfncreator.cpp)
add_graphia_test(tst_enrichmentcalculator attributes/tst_enrichmentcalculator.cpp)
add_graphia_test(tst_enrichmenttablemodel attributes/tst_enrichmenttablemodel.cpp)
add_graphia_test(tst_structuraltransforms transform/tst_structuraltransforms.cpp)
add_graphia_test(tst_analysistransforms transform/tst_analysistransforms.cpp)
add_graphia_test(tst_attributetransforms transform/tst_attributetransforms.cpp)
add_graphia_test(tst_transformcache transform/tst_transformcache.cpp)

add_graphia_test(tst_correlation correlation/tst_correlation.cpp)
target_link_libraries(tst_correlation PRIVATE correlationcore)

add_graphia_test(tst_tabulardata loading/tst_tabulardata.cpp)
add_graphia_test(tst_tabulardataparser loading/tst_tabulardataparser.cpp)
add_graphia_test(tst_graphparsers loading/tst_graphparsers.cpp)
add_graphia_test(tst_savers loading/tst_savers.cpp)
add_graphia_test(tst_nativeloader loading/tst_nativeloader.cpp)
add_graphia_test(tst_commands commands/tst_commands.cpp)
add_graphia_test(tst_visualisationconfigparser ui/tst_visualisationconfigparser.cpp)

add_graphia_test(tst_qml qml/tst_qml.cpp)
target_link_libraries(tst_qml PRIVATE Qt6::QuickTest)
target_compile_definitions(tst_qml PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_graphia_test(tst_string utils/tst_string.cpp)

if(UNIX AND NOT APPLE)
    # Qt takes the system locale from the environment on Linux, so also run in a
    # locale whose numbers differ from the C locale's in every respect
    add_test(NAME tst_string_sv_SE COMMAND tst_string)
    set_tests_properties(tst_string_sv_SE PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LC_ALL=sv_SE.UTF-8")
endif()

add_graphia_test(tst_statistics utils/tst_statistics.cpp)
add_graphia_test(tst_container utils/tst_container.cpp)
add_graphia_test(tst_datastructures utils/tst_datastructures.cpp)
add_graphia_test(tst_json utils/tst_json.cpp)
add_graphia_test(tst_color utils/tst_color.cpp)
add_graphia_test(tst_colorvisualisations visualisations/tst_colorvisualisations.cpp)
add_graphia_test(tst_visualisationmapping visualisations/tst_visualisationmapping.cpp)
add_graphia_test(tst_visualisations visualisations/tst_visualisations.cpp)
add_graphia_test(tst_searchmanager ui/tst_searchmanager.cpp)
add_graphia_test(tst_modelutils ui/tst_modelutils.cpp)
add_graphia_test(tst_commandmanager commands/tst_commandmanager.cpp)
add_graphia_test(tst_graphsizeestimate loading/tst_graphsizeestimate.cpp)
add_graphia_test(tst_geometry maths/tst_geometry.cpp)
add_graphia_test(tst_crypto utils/tst_crypto.cpp)
target_link_libraries(tst_crypto PRIVATE thirdparty_static thirdparty)
add_graphia_test(tst_selectionmanager ui/tst_selectionmanager.cpp)
add_graphia_test(tst_layout layout/tst_layout.cpp)
add_graphia_test(tst_genericplugin loading/tst_genericplugin.cpp)
add_graphia_test(tst_layoutthread layout/tst_layoutthread.cpp)
add_graphia_test(tst_random utils/tst_random.cpp)
add_graphia_test(tst_nodeattributetablemodel plugins/tst_nodeattributetablemodel.cpp)

# Headless mode loads the real plugins, from the build directory
add_graphia_test(tst_headless headless/tst_headless.cpp)
target_compile_definitions(tst_headless PRIVATE GRAPHIA_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_headless generic correlation)

# A Document, loading files with the real plugins
add_graphia_test(tst_document document/tst_document.cpp)
target_compile_definitions(tst_document PRIVATE GRAPHIA_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_document generic correlation)

# The correlation plugin, through a Document
add_graphia_test(tst_correlationplugin correlation/tst_correlationplugin.cpp)
target_compile_definitions(tst_correlationplugin PRIVATE GRAPHIA_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_correlationplugin correlation)

if(GRAPHIA_SANITIZERS)
    # The plugins each have their own copy of the shared library, as the tests do,
    # which AddressSanitizer reports as violating the one definition rule
    set_property(TEST tst_headless tst_document tst_correlationplugin APPEND PROPERTY
        ENVIRONMENT_MODIFICATION "ASAN_OPTIONS=string_append::detect_odr_violation=0")
endif()
