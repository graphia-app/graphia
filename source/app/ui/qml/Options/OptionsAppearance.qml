import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants

import "../Controls"

Item
{
    Preferences
    {
        id: visuals;
        section: "visuals"

        property alias defaultNodeColor: nodeColorPickButton.color
        property alias defaultEdgeColor: edgeColorPickButton.color
        property alias multiElementColor: multiElementColorPickButton.color
        property alias backgroundColor: backgroundColorPickButton.color
        property alias highlightColor: highlightColorPickButton.color

        property double defaultNodeSize
        property alias defaultNodeSizeMinimumValue: nodeSizeSlider.minimumValue
        property alias defaultNodeSizeMaximumValue: nodeSizeSlider.maximumValue

        property double defaultEdgeSize
        property alias defaultEdgeSizeMinimumValue: edgeSizeSlider.minimumValue
        property alias defaultEdgeSizeMaximumValue: edgeSizeSlider.maximumValue

        property alias transitionTime: transitionTimeSlider.value
        property alias transitionTimeMinimumValue : transitionTimeSlider.minimumValue
        property alias transitionTimeMaximumValue : transitionTimeSlider.maximumValue

        property double minimumComponentRadius
        property alias minimumComponentRadiusMinimumValue : minimumComponentRadiusSlider.minimumValue
        property alias minimumComponentRadiusMaximumValue : minimumComponentRadiusSlider.maximumValue

        property string textFont
        property var textSize
        property int textAlignment
    }

    Component.onCompleted:
    {
        nodeSizeSlider.value = visuals.defaultNodeSize;
        edgeSizeSlider.value = visuals.defaultEdgeSize;
        minimumComponentRadiusSlider.value = visuals.minimumComponentRadius;

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

            visuals.defaultNodeSize = nodeSizeSlider.value;
            visuals.defaultEdgeSize = edgeSizeSlider.value;
            visuals.minimumComponentRadius = minimumComponentRadiusSlider.value;
        }
    }

    RowLayout
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        GridLayout
        {
            Layout.alignment: Qt.AlignTop|Qt.AlignLeft

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

            Label { text: qsTr("Selection") }
            ColorPickButton { id: highlightColorPickButton }

            Label
            {
                font.bold: true
                text: qsTr("Sizes")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Nodes") }
            Slider { id: nodeSizeSlider; onValueChanged: { delayedPreferences.update(); } }

            Label { text: qsTr("Edges") }
            Slider { id: edgeSizeSlider; onValueChanged: { delayedPreferences.update(); } }

            Label
            {
                font.bold: true
                text: qsTr("Miscellaneous")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Transition Time") }
            Slider { id: transitionTimeSlider }

            Label { text: qsTr("Minimum Component Radius") }
            Slider { id: minimumComponentRadiusSlider; onValueChanged: { delayedPreferences.update(); } }
        }

        GridLayout
        {
            Layout.alignment: Qt.AlignTop|Qt.AlignLeft

            columns: 2
            rowSpacing: Constants.spacing
            columnSpacing: Constants.spacing
            Label
            {
                font.bold: true
                text: qsTr("Text")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Font") }
            Button
            {
                text: visuals.textFont + " " + visuals.textSize + "pt";
                onClicked: { fontDialog.visible = true }
            }

            Label { text: qsTr("Alignment") }
            ComboBox
            {
                model:
                [
                    // Must stay synced with TextAlignment in graphrenderer.h
                    qsTr("Right"),
                    qsTr("Left"),
                    qsTr("Centre"),
                    qsTr("Top"),
                    qsTr("Bottom")
                ]
                currentIndex: visuals.textAlignment
                onCurrentIndexChanged: visuals.textAlignment = currentIndex;
            }
        }
    }

    FontDialog
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



