include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

list(APPEND RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/resources.qrc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
  add_executable(MessageBox WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
else()
  add_executable(MessageBox ${SOURCES} ${HEADERS} ${RESOURCES})
  install(TARGETS MessageBox RUNTIME DESTINATION bin)
endif()

find_package(Qt6 COMPONENTS REQUIRED Widgets)
target_link_libraries(MessageBox Qt6::Widgets)
