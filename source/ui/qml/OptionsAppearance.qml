import QtQuick 2.5
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import com.kajeka 1.0

import "Constants.js" as Constants

Item
{
    Preferences
    {
        section: "visualDefaults"

        property alias nodeColor: nodeColorPickButton.color
        property alias edgeColor: edgeColorPickButton.color
        property alias multiElementColor: multiElementColorPickButton.color
        property alias backgroundColor: backgroundColorPickButton.color

        property alias nodeSize: nodeSizeSlider.value
        property alias edgeSize: edgeSizeSlider.value

        property alias transitionTime: transitionTimeSlider.value
        property alias minimumComponentRadius: minimumComponentRadiusSlider.value
    }

    Column
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        GridLayout
        {
            columns: 2
            rowSpacing: Constants.spacing
            columnSpacing: Constants.spacing

            Label
            {
                font.bold: true
                text: qsTr("Colours")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Nodes") }
            ColorPickButton { id: nodeColorPickButton }

            Label { text: qsTr("Edges") }
            ColorPickButton { id: edgeColorPickButton }

            Label { text: qsTr("Multi Elements") }
            ColorPickButton { id: multiElementColorPickButton }

            Label { text: qsTr("Background") }
            ColorPickButton { id: backgroundColorPickButton }

            Label
            {
                font.bold: true
                text: qsTr("Sizes")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Nodes") }
            Slider
            {
                id: nodeSizeSlider
                minimumValue: 0.1
                maximumValue: 2.0
            }

            Label { text: qsTr("Edges") }
            Slider
            {
                id: edgeSizeSlider
                minimumValue: 0.05
                maximumValue: 2.0
            }

            Label
            {
                font.bold: true
                text: qsTr("Miscellaneous")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Transition Time") }
            Slider
            {
                id: transitionTimeSlider
                minimumValue: 0.1
                maximumValue: 5.0
            }

            Label { text: qsTr("Minimum Component Radius") }
            Slider
            {
                id: minimumComponentRadiusSlider
                minimumValue: 0.1
                maximumValue: 10.0
            }
        }

    }
}

