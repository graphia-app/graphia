if(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
endif()

if(CMAKE_EXECUTABLE_FORMAT STREQUAL "ELF")
    # Each plugin uses its own copy of shared, even when something else in the process exports one
    add_link_options(LINKER:-Bsymbolic)
endif()

function(add_plugin_translations PLUGIN_NAME)
    qt_collect_translation_source_targets(SOURCE_TARGETS)
    set_property(DIRECTORY PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)

    qt_add_translations(${PLUGIN_NAME}
        SOURCE_TARGETS ${SOURCE_TARGETS}
        TS_FILE_BASE ${PLUGIN_NAME}
        TS_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations
        RESOURCE_PREFIX /i18n
        LRELEASE_OPTIONS -removeidentical
        ${SHARED_I18N_OPTIONS})
endfunction()

add_subdirectory(correlation)
add_subdirectory(generic)

if(NOT EMSCRIPTEN)
    add_subdirectory(websearch)
endif()
