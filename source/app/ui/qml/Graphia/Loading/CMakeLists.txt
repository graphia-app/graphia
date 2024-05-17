include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick Concurrent)

qt_add_qml_module(loading_qml
    URI Graphia.Loading
    STATIC
    SOURCES
        adjacencymatrixtabulardataparser.h adjacencymatrixtabulardataparser.cpp
        tabulardataparser.h tabulardataparser.cpp
)

target_link_libraries(loading_qml
    PRIVATE Qt6::Quick Qt6::Concurrent
)
