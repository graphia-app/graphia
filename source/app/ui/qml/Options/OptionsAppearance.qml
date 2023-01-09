/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts

import Qt.labs.platform as Labs

import app.graphia
import app.graphia.Controls
import app.graphia.Shared

Item
{
    LimitConstants { id: limitConstants }

    Preferences
    {
        id: visuals
        section: "visuals"

        property alias defaultNodeColor: nodeColorPickButton.color
        property alias defaultEdgeColor: edgeColorPickButton.color
        property alias multiElementColor: multiElementColorPickButton.color
        property alias backgroundColor: backgroundColorPickButton.color
        property alias highlightColor: highlightColorPickButton.color

        property double defaultNormalNodeSize
        property double defaultNormalEdgeSize
        property alias transitionTime: transitionTimeSlider.value
        property double minimumComponentRadius

        property string textFont
        property var textSize
        property alias textAlignment: textAlignmentCombobox.currentIndex
    }

    Preferences
    {
        id: system
        section: "system"

        property string uiTheme
    }

    Component.onCompleted:
    {
        nodeSizeSlider.value = visuals.defaultNormalNodeSize;
        edgeSizeSlider.value = visuals.defaultNormalEdgeSize;
        minimumComponentRadiusSlider.value = visuals.minimumComponentRadius;

        let themeIndex = userInterfaceComboBox.model.indexOf(system.uiTheme);
        userInterfaceComboBox.currentIndex = themeIndex >= 0 ? themeIndex : 0;

        delayedPreferences.enabled = true;
    }

    Timer
    {
        id: delayedPreferences

        property bool enabled: false

        interval: 250
        repeat: false

        function update()
        {
            if(!enabled)
                return;

            restart();
        }

        onTriggered:
        {
            if(!enabled)
                return;

            visuals.defaultNormalNodeSize = nodeSizeSlider.value;
            visuals.defaultNormalEdgeSize = edgeSizeSlider.value;
            visuals.minimumComponentRadius = minimumComponentRadiusSlider.value;
        }
    }

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridLayout
            {
                columns: 2
                rowSpacing: Constants.spacing
                columnSpacing: Constants.spacing

                Label
                {
                    Layout.columnSpan: 2

                    font.bold: true
                    text: qsTr("Default Colours")
                }

                Label { text: qsTr("Nodes") }
                ColorPickButton { id: nodeColorPickButton }

                Label { text: qsTr("Edges") }
                ColorPickButton { id: edgeColorPickButton }

                Label { text: qsTr("Multi Elements") }
                ColorPickButton { id: multiElementColorPickButton }

                Label { text: qsTr("Background") }
                ColorPickButton { id: backgroundColorPickButton }

                Label { text: qsTr("Selection") }
                ColorPickButton { id: highlightColorPickButton }

                Label
                {
                    Layout.columnSpan: 2
                    Layout.topMargin: Constants.margin * 2

                    font.bold: true
                    text: qsTr("Default Sizes")
                }

                Label { text: qsTr("Nodes") }
                Slider
                {
                    id: nodeSizeSlider
                    from: 0.0
                    to: 1.0

                    onValueChanged: { delayedPreferences.update(); }
                }

                Label { text: qsTr("Edges") }
                Slider
                {
                    id: edgeSizeSlider
                    from: 0.0
                    to: 1.0

                    onValueChanged: { delayedPreferences.update(); }
                }
            }
        }

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridLayout
            {
                columns: 2
                rowSpacing: Constants.spacing
                columnSpacing: Constants.spacing

                Label
                {
                    Layout.columnSpan: 2

                    font.bold: true
                    text: qsTr("Text")
                }

                Label { text: qsTr("Font") }
                Button
                {
                    text: visuals.textFont + " " + visuals.textSize + "pt";
                    onClicked: function(mouse) { fontDialog.visible = true }
                }

                Label { text: qsTr("Alignment") }
                ComboBox
                {
                    id: textAlignmentCombobox

                    model:
                    [
                        // Must stay synced with TextAlignment in graphrenderer.h
                        qsTr("Right"),
                        qsTr("Left"),
                        qsTr("Centre"),
                        qsTr("Top"),
                        qsTr("Bottom")
                    ]
                }

                Label
                {
                    Layout.columnSpan: 2
                    Layout.topMargin: Constants.margin * 2

                    font.bold: true
                    text: qsTr("Miscellaneous")
                }

                Label { text: qsTr("Transition Time") }
                Slider
                {
                    id: transitionTimeSlider
                    from: limitConstants.minimumTransitionTime
                    to: limitConstants.maximumTransitionTime
                }

                Label { text: qsTr("Component Radius") }
                Slider
                {
                    id: minimumComponentRadiusSlider
                    from: limitConstants.minimumMinimumComponentRadius
                    to: limitConstants.maximumMinimumComponentRadius

                    onValueChanged: { delayedPreferences.update(); }
                }

                Label
                {
                    Layout.topMargin: Constants.margin * 2
                    font.bold: true
                    text: qsTr("System")
                }


                Label
                {
                    Layout.topMargin: Constants.margin * 2
                    font.italic: true
                    text: qsTr("(Restart Required)")
                }

                Label { text: qsTr("User Interface Theme") }
                ComboBox
                {
                    id: userInterfaceComboBox
                    model: ["Default", "Fusion"]

                    onCurrentTextChanged: { system.uiTheme = currentText; }
                }
            }
        }
    }

    Labs.FontDialog
    {
        id: fontDialog
        title: qsTr("Please choose a font")
        currentFont: Qt.font({ family: visuals.textFont, pointSize: visuals.textSize, weight: Font.Normal })
        font: Qt.font({ family: visuals.textFont, pointSize: visuals.textSize, weight: Font.Normal })

        onAccepted:
        {
            visuals.textFont = fontDialog.font.family;
            visuals.textSize = fontDialog.font.pointSize;
        }
    }
}



