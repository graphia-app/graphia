find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(utils_qml
    URI app.graphia.Utils
    STATIC
    QML_FILES
        AttributeUtils.js
        Constants.js
        CanvasPaletteHelper.qml
        MenuUtils.js
        Utils.js
        VisualisationUtils.js
    SOURCES
        modelcompleter.h modelcompleter.cpp
        sortfilterproxymodel.h sortfilterproxymodel.cpp
        tableproxymodel.h tableproxymodel.cpp
)

target_link_libraries(utils_qml PRIVATE shared)

target_link_libraries(utils_qml
    PRIVATE Qt6::Quick
)
