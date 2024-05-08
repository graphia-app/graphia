include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick)

set_source_files_properties(${GENERIC_PLUGIN_QML_MODULE_DIR}/MatrixParameters.qml PROPERTIES
    QT_RESOURCE_ALIAS MatrixParameters.qml
)
set_source_files_properties(${GENERIC_PLUGIN_QML_MODULE_DIR}/PairwiseParameters.qml PROPERTIES
    QT_RESOURCE_ALIAS PairwiseParameters.qml
)
set_source_files_properties(${GENERIC_PLUGIN_QML_MODULE_DIR}/matrix.svg PROPERTIES
    QT_RESOURCE_ALIAS matrix.svg
)
set_source_files_properties(${GENERIC_PLUGIN_QML_MODULE_DIR}/table.svg PROPERTIES
    QT_RESOURCE_ALIAS table.svg
)

qt_add_qml_module(websearch_qml
    URI app.graphia.Plugins.WebSearch
    STATIC
    QML_FILES
        Main.qml
        ${GENERIC_PLUGIN_QML_MODULE_DIR}/MatrixParameters.qml
        ${GENERIC_PLUGIN_QML_MODULE_DIR}/PairwiseParameters.qml
    RESOURCES
        globe.svg
        ${GENERIC_PLUGIN_QML_MODULE_DIR}/matrix.svg
        ${GENERIC_PLUGIN_QML_MODULE_DIR}/table.svg
)

message(${GENERIC_PLUGIN_QML_MODULE_DIR})

target_link_libraries(websearch_qml
    PRIVATE Qt6::Quick
)
