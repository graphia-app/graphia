include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(generic_qml
    URI app.graphia.Plugins.Generic
    STATIC
    QML_FILES
        Main.qml
        MatrixParameters.qml
        PairwiseParameters.qml
    RESOURCES
        matrix.svg
        table.svg
        tools.svg
)

target_link_libraries(generic_qml
    PRIVATE Qt6::Quick
)
