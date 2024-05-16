include(${CMAKE_SOURCE_DIR}/source/common.cmake)

add_subdirectory(Controls)
add_subdirectory(Loading)
add_subdirectory(Plugins)
add_subdirectory(Utils)

find_package(Qt6 COMPONENTS REQUIRED Quick Concurrent Widgets)

qt_add_qml_module(main_qml
    URI app.graphia
    STATIC
    QML_FILES
        AboutDialog.qml
        AboutPluginsDialog.qml
        AddBookmark.qml
        AddTemplateDialog.qml
        CaptureScreenshotDialog.qml
        ChooserDialog.qml
        CloneAttributeDialog.qml
        CreateTransformDialog.qml
        CreateVisualisationDialog.qml
        DebugRectangle.qml
        DownloadProgress.qml
        EditAttributeDialog.qml
        EnrichmentResults.qml
        EnrichmentWizard.qml
        Find.qml
        ImportAttributesDialog.qml
        LatestChangesDialog.qml
        LayoutSetting.qml
        LayoutSettings.qml
        Main.qml
        ManageNamedListDialog.qml
        NewUpdate.qml
        OpenUrlDialog.qml
        OptionsAppearance.qml
        OptionsDefaults.qml
        OptionsDialog.qml
        OptionsMisc.qml
        OptionsNetwork.qml
        RemoveAttributesDialog.qml
        ShowPaletteDialog.qml
        TabUI.qml
        TextDialog.qml
        Tracking.qml
        Transform.qml
        TransformConfig.js
        TransformParameter.qml
        Transforms.qml
        Visualisation.qml
        Visualisations.qml
        VisualisationUtils.js
    SOURCES
        alert.h
        application.h application.cpp
        availableattributesmodel.h availableattributesmodel.cpp
        availabletransformsmodel.h availabletransformsmodel.cpp
        changelog.h changelog.cpp
        crashtype.h
        document.h document.cpp
        editattributetablemodel.h editattributetablemodel.cpp
        enrichmentheatmap.h enrichmentheatmap.cpp
        findtypes.h
        graphdisplay.h graphdisplay.cpp
        graphrenderertypes.h
        importattributeskeydetection.h importattributeskeydetection.cpp
        limitconstants.h
        preferences.h preferences.cpp
    RESOURCES
        tracking-background.png
)

target_link_libraries(main_qml
    PRIVATE Qt6::Quick Qt6::Concurrent Qt6::Widgets
)
