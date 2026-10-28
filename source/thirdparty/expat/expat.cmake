include_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

add_definitions(-DHAVE_MEMMOVE)
add_definitions(-DXML_STATIC)
add_definitions(-DXML_POOR_ENTROPY)

set(EXPAT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/lib/loadlibrary.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlparse.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlrole.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_ns.c
)

if(NOT MSVC)
    # Only xlsxio uses it; exported, it takes the place of the system's expat
    # for every other library in the process, such as fontconfig and Mesa
    set_source_files_properties(${EXPAT_SOURCES} PROPERTIES
        COMPILE_OPTIONS -fvisibility=hidden
        COMPILE_DEFINITIONS XMLIMPORT=)
endif()

list(APPEND SHARED_THIRDPARTY_SOURCES ${EXPAT_SOURCES})
