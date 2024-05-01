find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(sharedcontrols_qml
    URI app.graphia.Shared.Controls
    STATIC
    QML_FILES
        PointingCursorOnHoverLink.qml
        ScrollableTextArea.qml
        SplitViewHandle.qml
    SOURCES
        outline.h outline.cpp
)

target_link_libraries(sharedcontrols_qml
    PRIVATE Qt6::Quick
)
