add_definitions(-DZLIB_DLL)

list(APPEND SHARED_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/adler32.c
    ${CMAKE_CURRENT_LIST_DIR}/compress.c
    ${CMAKE_CURRENT_LIST_DIR}/crc32.c
    ${CMAKE_CURRENT_LIST_DIR}/deflate.c
    ${CMAKE_CURRENT_LIST_DIR}/gzclose.c
    ${CMAKE_CURRENT_LIST_DIR}/gzlib.c
    ${CMAKE_CURRENT_LIST_DIR}/gzread.c
    ${CMAKE_CURRENT_LIST_DIR}/gzwrite.c
    ${CMAKE_CURRENT_LIST_DIR}/infback.c
    ${CMAKE_CURRENT_LIST_DIR}/inffast.c
    ${CMAKE_CURRENT_LIST_DIR}/inflate.c
    ${CMAKE_CURRENT_LIST_DIR}/inftrees.c
    ${CMAKE_CURRENT_LIST_DIR}/trees.c
    ${CMAKE_CURRENT_LIST_DIR}/uncompr.c
    ${CMAKE_CURRENT_LIST_DIR}/zutil.c
)
