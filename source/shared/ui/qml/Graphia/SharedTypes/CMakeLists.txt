include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(sharedtypes_qml
    URI Graphia.SharedTypes
    STATIC
    SOURCES
        attributeflag.h
        elementtype.h elementtype.cpp
        pairwisecolumntype.h
        valuetype.h
)

target_link_libraries(sharedtypes_qml
    PRIVATE Qt6::Quick
)
