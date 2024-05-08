include(${CMAKE_SOURCE_DIR}/source/common.cmake)

add_definitions(-DPRODUCT_NAME="${PROJECT_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS REQUIRED Concurrent Qml Quick QuickControls2 Xml Widgets)

list(APPEND HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/../../shared/utils/preferences.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    qt_add_executable(UpdateEditor WIN32 ${SOURCES} ${HEADERS})
else()
    qt_add_executable(UpdateEditor ${SOURCES} ${HEADERS})
endif()

set_source_files_properties(../../app/icon/Icon16x16.png PROPERTIES QT_RESOURCE_ALIAS Icon16x16.png)
set_source_files_properties(../../app/icon/Icon32x32.png PROPERTIES QT_RESOURCE_ALIAS Icon32x32.png)
set_source_files_properties(../../app/icon/Icon48x48.png PROPERTIES QT_RESOURCE_ALIAS Icon48x48.png)
set_source_files_properties(../../app/icon/Icon64x64.png PROPERTIES QT_RESOURCE_ALIAS Icon64x64.png)
set_source_files_properties(../../app/icon/Icon128x128.png PROPERTIES QT_RESOURCE_ALIAS Icon128x128.png)
set_source_files_properties(../../app/icon/Icon256x256.png PROPERTIES QT_RESOURCE_ALIAS Icon256x256.png)
set_source_files_properties(../../app/icon/Icon512x512.png PROPERTIES QT_RESOURCE_ALIAS Icon512x512.png)

qt_add_qml_module(UpdateEditor
    URI app.graphia.UpdateEditor
    QML_FILES
        Main.qml
        CommandsDialog.qml
    RESOURCES
        ../../app/icon/Icon16x16.png
        ../../app/icon/Icon32x32.png
        ../../app/icon/Icon48x48.png
        ../../app/icon/Icon64x64.png
        ../../app/icon/Icon128x128.png
        ../../app/icon/Icon256x256.png
        ../../app/icon/Icon512x512.png
)

target_link_libraries(UpdateEditor PRIVATE
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Xml
    Qt6::Widgets
)

target_link_libraries(UpdateEditor PRIVATE shared controls_qmlplugin utils_qmlplugin)
