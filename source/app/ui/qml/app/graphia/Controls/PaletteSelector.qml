/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Qt.labs.platform as Labs

import app.graphia
import app.graphia.Shared

Window
{
    id: root

    property string configuration
    property var stringValues: []
    property bool applied: false

    // The window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    signal accepted()
    signal rejected()
    signal applyClicked(bool alreadyApplied)

    title: qsTr("Edit Palette")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 640
    minimumHeight: 480

    ButtonGroup { id: selectedGroup }

    function open(configuration, stringValues)
    {
        root.applied = false;
        root.stringValues = paletteEditor.stringValues = stringValues;
        paletteEditor.setup(configuration);
        show();
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
                FramedScrollView
                {
                    id: paletteListScrollView

                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    Column
                    {
                        id: palettePresets

                        width: paletteListScrollView.width

                        spacing: 2

                        function initialise()
                        {
                            let savedPalettes = savedPalettesFromPreferences();

                            paletteListModel.clear();
                            for(let i = 0; i < savedPalettes.length; i++)
                                paletteListModel.append({"paletteConfiguration" : JSON.stringify(savedPalettes[i])});
                        }

                        function remove(index)
                        {
                            if(index < 0)
                                return;

                            let savedPalettes = savedPalettesFromPreferences();
                            savedPalettes.splice(index, 1);
                            visuals.savedPalettes = JSON.stringify(savedPalettes);

                            paletteListModel.remove(index);
                        }

                        function add(configuration)
                        {
                            let savedPalettes = savedPalettesFromPreferences();
                            savedPalettes.unshift(JSON.parse(configuration));
                            visuals.savedPalettes = JSON.stringify(savedPalettes);

                            paletteListModel.insert(0, {"paletteConfiguration" : configuration});
                        }

                        property int selectedIndex:
                        {
                            let savedPalettes = savedPalettesFromPreferences();

                            for(let i = 0; i < savedPalettes.length; i++)
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

                                color: checked ? palette.highlight : "transparent";
                                width: palettePresets.width
                                height: paletteKey.height + Constants.padding

                                MouseArea
                                {
                                    anchors.fill: parent

                                    onClicked: function(mouse)
                                    {
                                        root.configuration = paletteKey.configuration;
                                        paletteEditor.setup(paletteKey.configuration);
                                    }
                                }

                                // Shim for padding and clipping
                                RowLayout
                                {
                                    anchors.centerIn: parent

                                    spacing: 0

                                    width: highlightMarker.width - (Constants.padding + paletteListScrollView.scrollBarWidth)

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
                                        Layout.rightMargin: Constants.margin

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

                    onClicked: function(mouse)
                    {
                        palettePresets.add(paletteEditor.configuration);
                    }
                }

                Button
                {
                    implicitWidth: paletteListScrollView.width

                    enabled: palettePresets.selectedIndex >= 0
                    text: qsTr("Delete Preset")

                    Labs.MessageDialog
                    {
                        id: deleteDialog
                        visible: false
                        title: qsTr("Delete")
                        text: qsTr("Are you sure you want to delete this palette preset?")
                        buttons: Labs.MessageDialog.Yes | Labs.MessageDialog.No

                        onYesClicked:
                        {
                            let defaultDeleted = comparePalettes(visuals.defaultPalette,
                                paletteEditor.configuration);

                            palettePresets.remove(palettePresets.selectedIndex);

                            if(defaultDeleted)
                            {
                                let savedPalettes = savedPalettesFromPreferences();
                                if(savedPalettes.length > 0)
                                    visuals.defaultPalette = JSON.stringify(savedPalettes[0]);
                            }
                        }
                    }

                    onClicked: function(mouse)
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

                    onClicked: function(mouse)
                    {
                        // Add a preset too, if it doesn't exist already
                        if(palettePresets.selectedIndex < 0)
                            palettePresets.add(paletteEditor.configuration);

                        visuals.defaultPalette = paletteEditor.configuration;
                    }
                }
            }

            FramedScrollView
            {
                id: paletteEditorScrollview

                Layout.fillWidth: true
                Layout.fillHeight: true

                PaletteEditor
                {
                    id: paletteEditor

                    width: paletteEditorScrollview.width - paletteEditorScrollview.staticScrollBarWidth

                    function scrollToItem(item)
                    {
                        let itemPosition = item.mapToItem(paletteEditor, 0, 0);
                        let newContentY = (itemPosition.y + item.height) -
                            paletteEditorScrollview.height + Constants.margin;

                        if(newContentY > paletteEditorScrollview.contentItem.contentY)
                            paletteEditorScrollview.contentItem.contentY = newContentY;
                    }

                    onItemAdded: function(item) { scrollToItem(item); }

                    onConfigurationChanged:
                    {
                        root.configuration = paletteEditor.configuration;
                    }
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
                onClicked: function(mouse)
                {
                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked: function(mouse)
                {
                    rejected();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Apply")
                onClicked: function(mouse)
                {
                    applyClicked(root.applied);
                    root.applied = true;
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
        let savedPalettes;

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

        if((paletteA.autoColors !== undefined) && (paletteB.autoColors !== undefined))
        {
            if(paletteA.autoColors.length !== paletteB.autoColors.length)
                return false;
        }
        else if((paletteA.autoColors !== undefined) !== (paletteB.autoColors !== undefined))
            return false;

        for(let i = 0; i < paletteA.autoColors.length; i++)
        {
            if(!Qt.colorEqual(paletteA.autoColors[i], paletteB.autoColors[i]))
                return false;
        }

        if((paletteA.fixedColors !== undefined) && (paletteB.fixedColors !== undefined))
        {
            let aValues = Object.getOwnPropertyNames(paletteA.fixedColors);
            let bValues = Object.getOwnPropertyNames(paletteB.fixedColors);

            if(aValues.length !== bValues.length)
                return false;

            for(let i = 0; i < aValues.length; i++)
            {
                let aValue = aValues[i];
                let bValue = bValues[i];

                if(aValue !== bValue)
                    return false;

                if(!Qt.colorEqual(paletteA.fixedColors[aValue], paletteB.fixedColors[bValue]))
                    return false;
            }
        }
        else if((paletteA.fixedColors !== undefined) !== (paletteB.fixedColors !== undefined))
            return false;

        if((paletteA.defaultColor !== undefined) && (paletteB.defaultColor !== undefined))
        {
            if(!Qt.colorEqual(paletteA.defaultColor, paletteB.defaultColor))
                return false;
        }
        else if((paletteA.defaultColor !== undefined) !== (paletteB.defaultColor !== undefined))
            return false;

        return true;
    }
}
