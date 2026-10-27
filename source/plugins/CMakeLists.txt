if(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
endif()

if(CMAKE_EXECUTABLE_FORMAT STREQUAL "ELF")
    # Each plugin uses its own copy of shared, even when something else in the process exports one
    add_link_options(LINKER:-Bsymbolic)
endif()

add_subdirectory(correlation)
add_subdirectory(generic)

if(NOT EMSCRIPTEN)
    add_subdirectory(websearch)
endif()
