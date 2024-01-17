if(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
endif()

add_subdirectory(correlation)
add_subdirectory(generic)

if(NOT EMSCRIPTEN)
    add_subdirectory(websearch)
endif()
