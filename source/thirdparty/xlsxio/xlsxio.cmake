include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_definitions(-DUSE_MINIZIP)
add_definitions(-DBUILD_XLSXIO_DLL)

list(APPEND SHARED_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/lib/xlsxio_read.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/xlsxio_read_sharedstrings.c
)
