include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick)

set_source_files_properties(FontPointSize.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(controls_qml
    URI app.graphia.Controls
    STATIC
    QML_FILES
        AlertIcon.qml
        AttributeListSortMenu.qml
        ButtonMenu.qml
        CheckBoxList.qml
        ColorPickButton.qml
        DataTable.qml
        DelayedBusyIndicator.qml
        DoubleSpinBox.qml
        DraggableList.qml
        FloatingButton.qml
        FontPointSize.qml
        FramedScrollView.qml
        GradientEditor.qml
        GradientKey.qml
        GradientSelector.qml
        Hamburger.qml
        HelpTooltip.qml
        HiddenSwitch.qml
        Hubble.qml
        ListBox.qml
        ListTab.qml
        ListTabView.qml
        MappingSelector.qml
        NodeAttributeTableView.qml
        OpenFileDialog.qml
        PaletteEditor.qml
        PaletteKey.qml
        PaletteSelector.qml
        PlatformMenu.qml
        PlatformMenuBar.qml
        PlatformMenuItem.qml
        PlatformMenuSeparator.qml
        PointingCursorOnHoverLink.qml
        SaveFileDialog.qml
        ScrollableTextArea.qml
        ScrollBarCornerFiller.qml
        SlidingPanel.qml
        SplitViewHandle.qml
        TabBarButton.qml
        ToolBarButton.qml
        ToolBarSeparator.qml
        TransformListSortMenu.qml
        TreeBox.qml
        TreeBoxSearch.qml
        TreeComboBox.qml
        Tutorial.qml
        Wizard.qml

        +nativemenu/PlatformMenu.qml
        +nativemenu/PlatformMenuBar.qml
        +nativemenu/PlatformMenuItem.qml
        +nativemenu/PlatformMenuSeparator.qml
        +macos/ToolBarButton.qml
        +wasm/OpenFileDialog.qml
        +wasm/SaveFileDialog.qml
    SOURCES
        controlcolors.h controlcolors.cpp
        hovermousepassthrough.h hovermousepassthrough.cpp
        namedicon.h namedicon.cpp
        outline.h outline.cpp
        visualisationmappingplot.h visualisationmappingplot.cpp
)

target_link_libraries(controls_qml
    PRIVATE Qt6::Quick
)
