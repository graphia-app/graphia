include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick Widgets)

if(EMSCRIPTEN)
    set(WASM_SOURCES wasmlocalfileaccess.h wasmlocalfileaccess.cpp)
endif()

qt_add_qml_module(utils_qml
    URI app.graphia.Utils
    STATIC
    QML_FILES
        AttributeUtils.js
        Constants.js
        CanvasPaletteHelper.qml
        MenuUtils.js
        Utils.js
    SOURCES
        modelcompleter.h modelcompleter.cpp
        nativeutils.h nativeutils.cpp
        preferences.h preferences.cpp
        sortfilterproxymodel.h sortfilterproxymodel.cpp
        tableproxymodel.h tableproxymodel.cpp
        ${WASM_SOURCES}
)

target_link_libraries(utils_qml PRIVATE shared)

target_link_libraries(utils_qml
    PRIVATE Qt6::Quick Qt6::Widgets
)
