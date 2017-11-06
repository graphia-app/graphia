list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.cpp
)

if(UNIX)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile_unix.cpp
    )
elseif(MSVC)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile_win.cpp
    )
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
