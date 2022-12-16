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
    property bool applied: false

    // The window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    signal accepted()
    signal rejected()
    signal applyClicked(bool alreadyApplied)

    title: qsTr("Edit Gradient")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 640
    minimumHeight: 320

    function open(configuration)
    {
        root.applied = false;
        gradientEditor.setup(configuration);
        show();
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string savedGradients
        property string defaultGradient
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
                    id: gradientListScrollView

                    implicitWidth: 160 // Instead of Layout.preferredWidth (QTBUG-109471)
                    Layout.fillHeight: true

                    Column
                    {
                        id: gradientPresets

                        width: gradientListScrollView.width

                        spacing: 2

                        function initialise()
                        {
                            let savedGradients = savedGradientsFromPreferences();

                            gradientListModel.clear();
                            for(let i = 0; i < savedGradients.length; i++)
                                gradientListModel.append({"gradientConfiguration" : JSON.stringify(savedGradients[i])});
                        }

                        function remove(index)
                        {
                            if(index < 0)
                                return;

                            let savedGradients = savedGradientsFromPreferences();
                            savedGradients.splice(index, 1);
                            visuals.savedGradients = JSON.stringify(savedGradients);

                            gradientListModel.remove(index);
                        }

                        function add(configuration)
                        {
                            let savedGradients = savedGradientsFromPreferences();
                            savedGradients.unshift(JSON.parse(configuration));
                            visuals.savedGradients = JSON.stringify(savedGradients);

                            gradientListModel.insert(0, {"gradientConfiguration" : configuration});
                        }

                        property int selectedIndex:
                        {
                            let savedGradients = savedGradientsFromPreferences();

                            for(let i = 0; i < savedGradients.length; i++)
                            {
                                if(root.compareGradients(gradientEditor.configuration, savedGradients[i]))
                                    return i;
                            }

                            return -1;
                        }

                        Repeater
                        {
                            model: ListModel { id: gradientListModel }

                            delegate: Rectangle
                            {
                                id: highlightMarker

                                property bool checked:
                                {
                                    return index === gradientPresets.selectedIndex;
                                }

                                color: checked ? palette.highlight : "transparent";
                                width: gradientPresets.width
                                height: gradientKey.height + Constants.padding

                                MouseArea
                                {
                                    anchors.fill: parent

                                    onClicked: function(mouse)
                                    {
                                        root.configuration = gradientKey.configuration;
                                        gradientEditor.setup(gradientKey.configuration);
                                    }
                                }

                                // Shim for padding and clipping
                                RowLayout
                                {
                                    anchors.centerIn: parent

                                    spacing: 0

                                    width: highlightMarker.width - (Constants.padding + gradientListScrollView.scrollBarWidth)

                                    Text
                                    {
                                        Layout.preferredWidth: 16
                                        text:
                                        {
                                            return (root.compareGradients(visuals.defaultGradient,
                                                gradientKey.configuration)) ? qsTr("✔") : "";
                                        }
                                    }

                                    GradientKey
                                    {
                                        id: gradientKey

                                        implicitWidth: 0
                                        Layout.fillWidth: true
                                        Layout.rightMargin: Constants.margin

                                        keyHeight: 20
                                        hoverEnabled: false
                                        showLabels: false

                                        Component.onCompleted:
                                        {
                                            gradientKey.configuration = gradientConfiguration;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Button
                {
                    implicitWidth: gradientListScrollView.width

                    enabled: gradientPresets.selectedIndex < 0
                    text: qsTr("Save As New Preset")

                    onClicked: function(mouse)
                    {
                        gradientPresets.add(gradientEditor.configuration);
                    }
                }

                Button
                {
                    implicitWidth: gradientListScrollView.width

                    enabled: gradientPresets.selectedIndex >= 0
                    text: qsTr("Delete Preset")

                    Labs.MessageDialog
                    {
                        id: deleteDialog
                        visible: false
                        title: qsTr("Delete")
                        text: qsTr("Are you sure you want to delete this gradient preset?")
                        buttons: Labs.MessageDialog.Yes | Labs.MessageDialog.No
                        modality: Qt.ApplicationModal

                        onYesClicked:
                        {
                            let defaultDeleted = compareGradients(visuals.defaultGradient,
                                gradientEditor.configuration);

                            gradientPresets.remove(gradientPresets.selectedIndex);

                            if(defaultDeleted)
                            {
                                let savedGradients = savedGradientsFromPreferences();
                                if(savedGradients.length > 0)
                                    visuals.defaultGradient = JSON.stringify(savedGradients[0]);
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
                    implicitWidth: gradientListScrollView.width

                    enabled:
                    {
                        return !compareGradients(visuals.defaultGradient,
                            gradientEditor.configuration);
                    }

                    text: qsTr("Set As Default")

                    onClicked: function(mouse)
                    {
                        // Add a preset too, if it doesn't exist already
                        if(gradientPresets.selectedIndex < 0)
                            gradientPresets.add(gradientEditor.configuration);

                        visuals.defaultGradient = gradientEditor.configuration;
                    }
                }
            }

            ColumnLayout
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: Constants.margin

                RowLayout
                {
                    Layout.fillWidth: true

                    Text
                    {
                        Layout.fillWidth: true

                        wrapMode: Text.WordWrap
                        text: qsTr("Click on the bar to add a marker.<br>" +
                                   "Double click a marker to change its color.<br>" +
                                   "Right click a marker to remove it.")
                    }

                    Button
                    {
                        text: "Invert"
                        onClicked: function(mouse) { gradientEditor.invert(); }
                    }
                }

                GradientEditor
                {
                    id: gradientEditor

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    onConfigurationChanged:
                    {
                        root.configuration = gradientEditor.configuration;
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
        gradientPresets.initialise();
    }

    function savedGradientsFromPreferences()
    {
        let savedGradients;

        try
        {
            savedGradients = JSON.parse(visuals.savedGradients);
        }
        catch(e)
        {
            savedGradients = [];
        }

        return savedGradients;
    }

    function compareGradients(gradientA, gradientB)
    {
        if(typeof(gradientA) === "string")
        {
            if(gradientA.length === 0)
                return false;

            gradientA = JSON.parse(gradientA);
        }

        if(typeof(gradientB) === "string")
        {
            if(gradientB.length === 0)
                return false;

            gradientB = JSON.parse(gradientB);
        }

        if(typeof(gradientA) === "undefined" || typeof(gradientB) === "undefined")
            return false;

        let aValues = Object.getOwnPropertyNames(gradientA);
        let bValues = Object.getOwnPropertyNames(gradientB);

        if(aValues.length !== bValues.length)
            return false;

        for(let i = 0; i < aValues.length; i++)
        {
            let aValue = aValues[i];
            let bValue = bValues[i];

            if(aValue !== bValue)
                return false;

            if(!Qt.colorEqual(gradientA[aValue], gradientB[bValue]))
                return false;
        }

        return true;
    }
}
