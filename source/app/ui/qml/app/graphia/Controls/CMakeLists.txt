find_package(Qt6 COMPONENTS REQUIRED Quick)

qt_add_qml_module(controls_qml
    URI app.graphia.Controls
    STATIC
    QML_FILES
        AlertIcon.qml
        AttributeListSortMenu.qml
        ButtonMenu.qml
        CheckBoxList.qml
        ColorPickButton.qml
        DoubleSpinBox.qml
        DraggableList.qml
        FramedScrollView.qml
        GradientEditor.qml
        GradientKey.qml
        GradientSelector.qml
        Hamburger.qml
        HiddenSwitch.qml
        ListBox.qml
        MappingSelector.qml
        PaletteEditor.qml
        PaletteKey.qml
        PaletteSelector.qml
        TransformListSortMenu.qml
        TreeBox.qml
        TreeBoxSearch.qml
        TreeComboBox.qml
        Tutorial.qml
    SOURCES
        hovermousepassthrough.h hovermousepassthrough.cpp
)

target_link_libraries(controls_qml
    PRIVATE Qt6::Quick
)
