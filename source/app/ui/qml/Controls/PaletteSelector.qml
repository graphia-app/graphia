import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import com.kajeka 1.0
import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

Window
{
    id: root

    property string configuration
    property string _selectedConfiguration
    property string _initialConfiguration

    // The window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    property var stringValues: []

    signal accepted()
    signal rejected()

    title: qsTr("Edit Palette")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 640
    minimumHeight: 480

    SystemPalette
    {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    ExclusiveGroup { id: selectedGroup }

    onVisibleChanged:
    {
        // When the window is first shown
        if(visible)
        {
            root._initialConfiguration = root._selectedConfiguration = root.configuration;
            paletteEditor.setup(root.configuration);
        }
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string savedPalettes
        property string defaultPalette
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            ColumnLayout
            {
                ScrollView
                {
                    id: paletteListScrollView

                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    frameVisible: true
                    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                    contentItem: Column
                    {
                        id: palettePresets

                        width: paletteListScrollView.viewport.width

                        spacing: 2

                        function initialise()
                        {
                            var savedPalettes = savedPalettesFromPreferences();

                            paletteListModel.clear();
                            for(var i = 0; i < savedPalettes.length; i++)
                                paletteListModel.append({"paletteConfiguration" : JSON.stringify(savedPalettes[i])});
                        }

                        function remove(index)
                        {
                            if(index < 0)
                                return;

                            var savedPalettes = savedPalettesFromPreferences();
                            savedPalettes.splice(index, 1);
                            visuals.savedPalettes = JSON.stringify(savedPalettes);

                            paletteListModel.remove(index);
                        }

                        function add(configuration)
                        {
                            var savedPalettes = savedPalettesFromPreferences();
                            savedPalettes.unshift(JSON.parse(configuration));
                            visuals.savedPalettes = JSON.stringify(savedPalettes);

                            paletteListModel.insert(0, {"paletteConfiguration" : configuration});
                        }

                        property int selectedIndex:
                        {
                            var savedPalettes = savedPalettesFromPreferences();

                            for(var i = 0; i < savedPalettes.length; i++)
                            {
                                if(root.comparePalettes(paletteEditor.configuration, savedPalettes[i]))
                                    return i;
                            }

                            return -1;
                        }

                        Repeater
                        {
                            model: ListModel { id: paletteListModel }

                            delegate: Rectangle
                            {
                                id: highlightMarker

                                property bool checked:
                                {
                                    return index === palettePresets.selectedIndex;
                                }

                                color: checked ? systemPalette.highlight : "transparent";
                                width: palettePresets.width
                                height: paletteKey.height + Constants.padding

                                MouseArea
                                {
                                    anchors.fill: parent

                                    onClicked:
                                    {
                                        root._selectedConfiguration = paletteKey.configuration;
                                        paletteEditor.setup(root._selectedConfiguration);
                                    }
                                }

                                // Shim for padding and clipping
                                RowLayout
                                {
                                    anchors.centerIn: parent

                                    spacing: 0

                                    width: highlightMarker.width - Constants.padding

                                    Text
                                    {
                                        Layout.preferredWidth: 16
                                        text:
                                        {
                                            return (root.comparePalettes(visuals.defaultPalette,
                                                paletteKey.configuration)) ? qsTr("✔") : "";
                                        }
                                    }

                                    PaletteKey
                                    {
                                        id: paletteKey

                                        implicitWidth: 0
                                        Layout.fillWidth: true

                                        keyHeight: 20
                                        separateKeys: false
                                        hoverEnabled: false

                                        Component.onCompleted:
                                        {
                                            paletteKey.configuration = paletteConfiguration;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Button
                {
                    implicitWidth: paletteListScrollView.width

                    enabled: palettePresets.selectedIndex < 0
                    text: qsTr("Save As New Preset")

                    onClicked:
                    {
                        palettePresets.add(paletteEditor.configuration);
                    }
                }

                Button
                {
                    implicitWidth: paletteListScrollView.width

                    enabled: palettePresets.selectedIndex >= 0
                    text: qsTr("Delete Preset")

                    MessageDialog
                    {
                        id: deleteDialog
                        visible: false
                        title: qsTr("Delete")
                        text: qsTr("Are you sure you want to delete this palette preset?")

                        icon: StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.No
                        onYes:
                        {
                            var defaultDeleted = comparePalettes(visuals.defaultPalette,
                                paletteEditor.configuration);

                            palettePresets.remove(palettePresets.selectedIndex);

                            if(defaultDeleted)
                            {
                                var savedPalettes = savedPalettesFromPreferences();
                                if(savedPalettes.length > 0)
                                    visuals.defaultPalette = JSON.stringify(savedPalettes[0]);
                            }
                        }
                    }

                    onClicked:
                    {
                        deleteDialog.visible = true;
                    }
                }

                Button
                {
                    implicitWidth: paletteListScrollView.width

                    enabled:
                    {
                        return !comparePalettes(visuals.defaultPalette,
                            paletteEditor.configuration);
                    }

                    text: qsTr("Set As Default")

                    onClicked:
                    {
                        // Add a preset too, if it doesn't exist already
                        if(palettePresets.selectedIndex < 0)
                            palettePresets.add(paletteEditor.configuration);

                        visuals.defaultPalette = paletteEditor.configuration;
                    }
                }
            }

            ScrollView
            {
                id: paletteEditorScrollview

                Layout.fillWidth: true
                Layout.fillHeight: true

                frameVisible: true
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                contentItem: PaletteEditor
                {
                    id: paletteEditor

                    width: paletteEditorScrollview.viewport.width

                    stringValues: root.stringValues

                    function scrollToItem(item)
                    {
                        var itemPosition = item.mapToItem(paletteEditorScrollview.contentItem, 0, 0);
                        var newContentY = (itemPosition.y + item.height) -
                            paletteEditorScrollview.viewport.height + Constants.margin;

                        if(newContentY > paletteEditorScrollview.flickableItem.contentY)
                            paletteEditorScrollview.flickableItem.contentY = newContentY;
                    }

                    onAutoColorAdded: { scrollToItem(item); }
                    onFixedColorAdded: { scrollToItem(item); }
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("OK")
                onClicked:
                {
                    root.configuration = paletteEditor.configuration;

                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked:
                {
                    if(root._initialConfiguration !== null && root._initialConfiguration !== "")
                        root.configuration = root._initialConfiguration;

                    rejected();
                    root.close();
                }
            }
        }
    }

    Component.onCompleted:
    {
        palettePresets.initialise();
    }

    function savedPalettesFromPreferences()
    {
        var savedPalettes;

        try
        {
            savedPalettes = JSON.parse(visuals.savedPalettes);
        }
        catch(e)
        {
            savedPalettes = [];
        }

        return savedPalettes;
    }

    function comparePalettes(paletteA, paletteB)
    {
        if(typeof(paletteA) === "string")
        {
            if(paletteA.length === 0)
                return false;

            paletteA = JSON.parse(paletteA);
        }

        if(typeof(paletteB) === "string")
        {
            if(paletteB.length === 0)
                return false;

            paletteB = JSON.parse(paletteB);
        }

        if((paletteA.baseColors !== undefined) && (paletteB.baseColors !== undefined))
        {
            if(paletteA.baseColors.length !== paletteB.baseColors.length)
                return false;
        }
        else if((paletteA.baseColors !== undefined) !== (paletteB.baseColors !== undefined))
            return false;

        for(var i = 0; i < paletteA.baseColors.length; i++)
        {
            if(!Qt.colorEqual(paletteA.baseColors[i], paletteB.baseColors[i]))
                return false;
        }

        if((paletteA.fixedColors !== undefined) && (paletteB.fixedColors !== undefined))
        {
            var aValues = Object.getOwnPropertyNames(paletteA.fixedColors);
            var bValues = Object.getOwnPropertyNames(paletteB.fixedColors);

            if(aValues.length !== bValues.length)
                return false;

            for(i = 0; i < aValues.length; i++)
            {
                var value = aValues[i];

                if(!Qt.colorEqual(paletteA.fixedColors[value], paletteA.fixedColors[value]))
                    return false;
            }
        }
        else if((paletteA.fixedColors !== undefined) !== (paletteB.fixedColors !== undefined))
            return false;

        if((paletteA.otherColor !== undefined) && (paletteB.otherColor !== undefined))
        {
            if(!Qt.colorEqual(paletteA.otherColor, paletteB.otherColor))
                return false;
        }
        else if((paletteA.otherColor !== undefined) !== (paletteB.otherColor !== undefined))
            return false;

        return true;
    }
}
