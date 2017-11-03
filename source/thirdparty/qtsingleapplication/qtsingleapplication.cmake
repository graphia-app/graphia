include(${CMAKE_CURRENT_LIST_DIR}/../qtlockedfile/qtlockedfile.cmake)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qtsingleapplication.h
    ${CMAKE_CURRENT_LIST_DIR}/qtlocalpeer.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qtsingleapplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qtlocalpeer.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
