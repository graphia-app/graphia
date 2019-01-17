include_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

add_definitions(-DHAVE_MEMMOVE)
add_definitions(-DXML_STATIC)
add_definitions(-DXML_POOR_ENTROPY)

list(APPEND SHARED_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/lib/loadlibrary.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlparse.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmlrole.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xmltok_ns.c
)
