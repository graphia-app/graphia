list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile_unix.cpp #FIXME
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
