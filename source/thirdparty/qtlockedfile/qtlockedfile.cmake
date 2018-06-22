list(APPEND STATIC_THIRDPARTY_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.h
)

list(APPEND STATIC_THIRDPARTY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile.cpp
)

if(UNIX)
    list(APPEND STATIC_THIRDPARTY_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile_unix.cpp
    )
elseif(MSVC)
    list(APPEND STATIC_THIRDPARTY_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/qtlockedfile_win.cpp
    )
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
