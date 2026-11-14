include(${CMAKE_SOURCE_DIR}/source/common.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# The Qt targets app_core links against must also be visible here, otherwise
# Qt can't walk the tests' dependencies when finalising the executables
find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Qml Quick QuickControls2 OpenGL Svg Widgets Xml Network Test QuickTest)
if(NOT EMSCRIPTEN)
    find_package(Qt6 REQUIRED COMPONENTS WebEngineQuick)
endif()

if(WIN32 OR EMSCRIPTEN)
    # A DLL exports only what is marked for export, and wasm has no shared libraries,
    # so each test links its own copy
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

# An object library, so that its startup function is linked in without anything referring to it
add_library(scrambling_translator OBJECT common/scramblingtranslator.cpp)
target_link_libraries(scrambling_translator PRIVATE Qt6::Core)

if(UNIX AND NOT APPLE)
    find_program(XVFB_RUN xvfb-run)
endif()

function(add_unit_test NAME)
    cmake_parse_arguments(PARSE_ARGV 1 UNIT_TEST "NO_WASM;OPENGL_WINDOWS" "" "")
    set(SOURCES ${UNIT_TEST_UNPARSED_ARGUMENTS})

    qt_add_executable(${NAME} ${SOURCES})
    target_link_libraries(${NAME} PRIVATE ${APP_CORE_FOR_TESTS} Qt6::Test scrambling_translator)

    if(EMSCRIPTEN)
        # The wasm platform needs a browser, so under node only tests without a GUI can run
        list(GET SOURCES 0 MAIN_SOURCE)
        file(STRINGS ${MAIN_SOURCE} GUILESS_MAIN REGEX "QTEST_GUILESS_MAIN")
        if(UNIT_TEST_NO_WASM OR NOT GUILESS_MAIN)
            set_target_properties(${NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
            return()
        endif()

        # The host's filesystem holds the test data, and ctest needs main's exit code
        target_link_options(${NAME} PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)

        # wasm-opt's release passes take most of each test's link time
        target_link_options(${NAME} PRIVATE -O1)
        set_target_properties(${NAME} PROPERTIES
            QT_WASM_EXTRA_EXPORTED_METHODS ENV
            CROSSCOMPILING_EMULATOR "${CMAKE_CROSSCOMPILING_EMULATOR};${CMAKE_CURRENT_SOURCE_DIR}/run-wasm-test.js")
    endif()

    # Each test also runs with every translatable string scrambled, which exposes
    # anything that relies on a translation's text, or on its word order
    set(COMMAND ${NAME})
    if(UNIT_TEST_OPENGL_WINDOWS AND XVFB_RUN)
        # A display of its own, so that the windows never appear on the desktop
        set(COMMAND ${XVFB_RUN} -a $<TARGET_FILE:${NAME}>)
    endif()

    add_test(NAME ${NAME} COMMAND ${COMMAND})
    add_test(NAME ${NAME}_scrambled COMMAND ${COMMAND})
    set(TESTS ${NAME} ${NAME}_scrambled)

    if(UNIT_TEST_OPENGL_WINDOWS AND XVFB_RUN)
        # xvfb-run -a picks a free display number racily when started concurrently
        set_tests_properties(${TESTS} PROPERTIES RESOURCE_LOCK xvfb)
    endif()
    set_property(TEST ${NAME}_scrambled APPEND PROPERTY ENVIRONMENT "GRAPHIA_SCRAMBLE_TRANSLATIONS=1")

    # Without a console, Qt and QtTest log to the debugger or the journal instead
    set_property(TEST ${TESTS} APPEND PROPERTY ENVIRONMENT "QT_FORCE_STDERR_LOGGING=1")

    if(UNIX AND NOT APPLE)
        if(UNIT_TEST_OPENGL_WINDOWS)
            # Qt Quick has only its software renderer on the offscreen platform
            set_property(TEST ${TESTS} APPEND PROPERTY ENVIRONMENT "QT_QPA_PLATFORM=xcb")
        else()
            # The offscreen platform only has OpenGL through GLX, so elsewhere the native one is used
            set_property(TEST ${TESTS} APPEND PROPERTY ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        endif()
    endif()

    if(WIN32)
        # Windows has no rpath, so the shared libraries in the root of the build need to be on PATH
        set_tests_properties(${TESTS} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}")
    endif()
endfunction()

# Every scrambled run relies on the translator being installed for it
add_unit_test(tst_scramblingtranslator i18n/tst_scramblingtranslator.cpp)

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

add_unit_test(tst_qml qml/tst_qml.cpp)
target_link_libraries(tst_qml PRIVATE Qt6::QuickTest)
target_compile_definitions(tst_qml PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
    BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_qml generic)
add_unit_test(tst_string utils/tst_string.cpp)

if(UNIX AND NOT APPLE)
    # Qt takes the system locale from the environment on Linux, so also run in a
    # locale whose numbers differ from the C locale's in every respect
    add_test(NAME tst_string_sv_SE COMMAND tst_string)
    set_tests_properties(tst_string_sv_SE PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1;LC_ALL=sv_SE.UTF-8")
endif()

add_unit_test(tst_statistics utils/tst_statistics.cpp)
add_unit_test(tst_container utils/tst_container.cpp)
add_unit_test(tst_datastructures utils/tst_datastructures.cpp)
add_unit_test(tst_json utils/tst_json.cpp)
add_unit_test(tst_color utils/tst_color.cpp)
add_unit_test(tst_colorvisualisations visualisations/tst_colorvisualisations.cpp)
add_unit_test(tst_visualisationmapping visualisations/tst_visualisationmapping.cpp)
add_unit_test(tst_visualisations visualisations/tst_visualisations.cpp)
add_unit_test(tst_searchmanager ui/tst_searchmanager.cpp)
add_unit_test(tst_modelutils ui/tst_modelutils.cpp)
add_unit_test(tst_preferences ui/tst_preferences.cpp)
add_unit_test(tst_preferencesstore ui/tst_preferencesstore.cpp)
add_unit_test(tst_commandmanager commands/tst_commandmanager.cpp)
add_unit_test(tst_graphsizeestimate loading/tst_graphsizeestimate.cpp)
add_unit_test(tst_geometry maths/tst_geometry.cpp)
# Only the updater uses the cryptography, and the web build has no updater
add_unit_test(tst_crypto NO_WASM utils/tst_crypto.cpp)
add_unit_test(tst_plots ui/tst_plots.cpp)

add_unit_test(tst_namedicon ui/tst_namedicon.cpp)
target_compile_definitions(tst_namedicon PRIVATE ICON_THEMES_DIR="${CMAKE_SOURCE_DIR}/source/app/icon-themes")
if(NOT EMSCRIPTEN)
    # Relative, as the platform's arguments are split at colons, which a Windows path contains
    set(NAMEDICON_TESTS tst_namedicon tst_namedicon_scrambled)
    set_property(TEST ${NAMEDICON_TESTS} APPEND PROPERTY ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen:configfile=namedicon_screens.json")
    set_property(TEST ${NAMEDICON_TESTS} PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ui)
endif()
target_link_libraries(tst_crypto PRIVATE thirdparty_static thirdparty)
# The web build has no updater
add_unit_test(tst_updates NO_WASM utils/tst_updates.cpp)
target_link_libraries(tst_updates PRIVATE thirdparty_static thirdparty)
add_unit_test(tst_selectionmanager ui/tst_selectionmanager.cpp)
add_unit_test(tst_layout layout/tst_layout.cpp)
add_unit_test(tst_genericplugin loading/tst_genericplugin.cpp)
add_unit_test(tst_layoutthread layout/tst_layoutthread.cpp)
add_unit_test(tst_random utils/tst_random.cpp)
# The web build may not give a pool the two threads this relies on
add_unit_test(tst_threadpool NO_WASM utils/tst_threadpool.cpp)
add_unit_test(tst_nodeattributetablemodel plugins/tst_nodeattributetablemodel.cpp)

# The translation catalogues, as they are in the source tree
add_unit_test(tst_catalogues i18n/tst_catalogues.cpp)
target_compile_definitions(tst_catalogues PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}/source")

# Loading catalogues, with a few of the test's own
add_unit_test(tst_translator i18n/tst_translator.cpp)
qt_add_lrelease(
    TS_FILES
        i18n/catalogues/graphia_fr_FR.ts
        i18n/catalogues/graphia_es_ES.ts
        i18n/catalogues/testplugin_fr_FR.ts
        i18n/catalogues/testplugin_de_DE.ts
        i18n/catalogues/testplugin_en_GB.ts
    LRELEASE_TARGET tst_translator_lrelease
    QM_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_catalogues
    QM_FILES_OUTPUT_VARIABLE TEST_CATALOGUES)
qt_add_resources(tst_translator test_catalogues PREFIX /i18n
    BASE ${CMAKE_CURRENT_BINARY_DIR}/test_catalogues FILES ${TEST_CATALOGUES})

add_unit_test(tst_opengl rendering/tst_opengl.cpp)
add_unit_test(tst_camera rendering/tst_camera.cpp)
add_unit_test(tst_transition rendering/tst_transition.cpp)

# A Document shown in a window, drawn by the real renderer
add_unit_test(tst_rendering rendering/tst_rendering.cpp OPENGL_WINDOWS)
target_compile_definitions(tst_rendering PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_rendering generic)
add_unit_test(tst_interaction ui/tst_interaction.cpp OPENGL_WINDOWS)
target_compile_definitions(tst_interaction PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_interaction generic)

# Headless mode loads the real plugins, from the build directory
add_unit_test(tst_headless headless/tst_headless.cpp)
target_compile_definitions(tst_headless PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_headless generic correlation)

# A Document, loading files with the real plugins
add_unit_test(tst_document document/tst_document.cpp)
target_compile_definitions(tst_document PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_document generic correlation)

# The correlation plugin, through a Document
add_unit_test(tst_correlationplugin correlation/tst_correlationplugin.cpp)
target_compile_definitions(tst_correlationplugin PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_correlationplugin correlation)

# The plot in the correlation plugin's panel
add_unit_test(tst_correlationplot correlation/tst_correlationplot.cpp)
target_compile_definitions(tst_correlationplot PRIVATE BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(tst_correlationplot correlation)
