include_directories(${CMAKE_CURRENT_LIST_DIR})

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/qqmlsortfilterproxymodel.h
    ${CMAKE_CURRENT_LIST_DIR}/filter.h
    ${CMAKE_CURRENT_LIST_DIR}/sorter.h
    ${CMAKE_CURRENT_LIST_DIR}/proxyrole.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qqmlsortfilterproxymodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/proxyrole.cpp
)

