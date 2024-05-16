include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(plugins_qml
    URI app.graphia.Plugins
    STATIC
    QML_FILES
        PluginContent.qml
        BaseParameterDialog.qml
        Templates.qml
)

target_link_libraries(plugins_qml
    PRIVATE Qt6::Quick
)
