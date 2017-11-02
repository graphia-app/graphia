include_directories(${CMAKE_CURRENT_LIST_DIR}/../qtlockedfile)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/qtsingleapplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qtlocalpeer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qtlockedfile/qtlockedfile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qtlockedfile/qtlockedfile_unix.cpp #FIXME
)

