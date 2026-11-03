include_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

add_definitions(-DHAVE_MEMMOVE)
add_definitions(-DXML_STATIC)
add_definitions(-DXML_POOR_ENTROPY)

set(EXPAT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/lib/xcs.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlparse.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlrole.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_ns.c
)

if(WIN32)
    list(APPEND EXPAT_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib/random_rand_s.c)
endif()

set_property(SOURCE ${EXPAT_SOURCES} APPEND PROPERTY
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR})

if(NOT MSVC)
    # Only xlsxio uses it; exported, it takes the place of the system's expat
    # for every other library in the process, such as fontconfig and Mesa
    set_source_files_properties(${EXPAT_SOURCES} PROPERTIES
        COMPILE_OPTIONS -fvisibility=hidden
        COMPILE_DEFINITIONS XMLIMPORT=)
endif()

list(APPEND SHARED_THIRDPARTY_SOURCES ${EXPAT_SOURCES})
