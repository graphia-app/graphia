include(${CMAKE_SOURCE_DIR}/source/common.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/source/shared/utils/translator.cpp
)

list(APPEND HEADERS
    ${CMAKE_SOURCE_DIR}/source/shared/utils/translator.h
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/resources.qrc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
  qt_add_executable(MessageBox WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
else()
  qt_add_executable(MessageBox ${SOURCES} ${HEADERS} ${RESOURCES})
  install(TARGETS MessageBox RUNTIME DESTINATION bin)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
target_link_libraries(MessageBox PRIVATE Qt6::Widgets)
