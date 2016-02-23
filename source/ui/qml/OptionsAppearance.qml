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
        property alias nodeSizeMinimumValue: nodeSizeSlider.minimumValue
        property alias nodeSizeMaximumValue: nodeSizeSlider.maximumValue

        property alias edgeSize: edgeSizeSlider.value
        property alias edgeSizeMinimumValue: edgeSizeSlider.minimumValue
        property alias edgeSizeMaximumValue: edgeSizeSlider.maximumValue

        property alias transitionTime: transitionTimeSlider.value
        property alias transitionTimeMinimumValue : transitionTimeSlider.minimumValue
        property alias transitionTimeMaximumValue : transitionTimeSlider.maximumValue

        property alias minimumComponentRadius: minimumComponentRadiusSlider.value
        property alias minimumComponentRadiusMinimumValue : minimumComponentRadiusSlider.minimumValue
        property alias minimumComponentRadiusMaximumValue : minimumComponentRadiusSlider.maximumValue
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
            Slider { id: nodeSizeSlider }

            Label { text: qsTr("Edges") }
            Slider { id: edgeSizeSlider }

            Label
            {
                font.bold: true
                text: qsTr("Miscellaneous")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Transition Time") }
            Slider { id: transitionTimeSlider }

            Label { text: qsTr("Minimum Component Radius") }
            Slider { id: minimumComponentRadiusSlider }
        }
    }
}

