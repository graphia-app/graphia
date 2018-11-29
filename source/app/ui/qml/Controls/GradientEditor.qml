import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

Item
{
    id: root
    implicitHeight: picker.visible ? 60 + picker.height : 60

    property alias configuration: gradientKey.configuration

    signal clicked()

    SystemPalette
    {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    property var _markers : []

    function setup(configuration)
    {
        var gradient = JSON.parse(configuration);

        var markers = [];

        for(var value in gradient)
            markers.push({value: value, color: gradient[value]});

        markerRepeater.model = root._markers = markers;
    }

    function addMarker(value, color)
    {
        var markers = root._markers;

        var newMarker = {};
        newMarker.value = value;
        newMarker.color = Utils.colorToString(color);
        markers.push(newMarker);

        markerRepeater.model = root._markers = markers;
    }

    function removeMarker(index)
    {
        var markers = root._markers;
        markers.splice(index, 1);

        markerRepeater.model = root._markers = markers;
    }

    function alterMarker(index, value, color)
    {
        var markers = root._markers;
        markers.splice(index, 1);

        addMarker(value, color);
    }

    function invert()
    {
        var markers = [];

        for(var i = 0; i < root._markers.length; i++)
        {
            var newValue = 1.0 - root._markers[i].value;
            var oldColor = root._markers[i].color;
            markers.push({value:newValue , color: oldColor});
        }

        markerRepeater.model = root._markers = markers;
    }

    Column
    {
        id: layout

        anchors.fill: parent

        spacing: 0

        GradientKey
        {
            width: root.width
            id: gradientKey
            implicitHeight: picker.visible ? root.height - picker.height : root.height

            _padding: picker._markerWidth

            configuration:
            {
                var o = {};

                for(var i = 0; i < _markers.length; i++)
                {
                    var marker = _markers[i];

                    var normalisedPos = marker.value;
                    var color = marker.color;

                    o[normalisedPos] = color;
                }

                return JSON.stringify(o);
            }

            showLabels: false
            hoverEnabled: false
        }

        Item
        {
            id: picker

            property var selected
            property int _markerWidth: 20

            width: root.width
            height: 20

            function valueToMarkerPosition(value)
            {
                return (value * markerBar.width);
            }

            function markerToValue(item)
            {
                return (item.x / (markerBar.width));
            }

            function markerToFormattedValue(item)
            {
                return parseInt(parseFloat(picker.markerToValue(item))
                            .toFixed(2) * 100);
            }

            Rectangle
            {
                id: markerBar
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "lightgrey"
                height: parent.height / 5.0
                width: parent.width - parent._markerWidth
            }

            MouseArea
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height
                width: parent.width - picker._markerWidth
                onClicked:
                {
                    var leftItem = null;
                    var rightItem = null;
                    var closestMarker = null;
                    var snapValue = parseFloat(mouseX / width).toFixed(2);
                    var findValue = picker.valueToMarkerPosition(snapValue);
                    var diff = Number.MAX_VALUE;

                    // Find suitable left + right marker (also find the closest marker)
                    for(var i = 0; i < markerRepeater.count; i++)
                    {
                        var modelObj = markerRepeater.itemAt(i);
                        if(modelObj.x < findValue)
                        {
                            if(leftItem === null || modelObj.x > leftItem.x)
                                leftItem = modelObj;
                        }
                        else if(modelObj.x > findValue)
                        {
                            if(rightItem === null || modelObj.x < rightItem.x)
                                rightItem = modelObj;
                        }

                        if(Math.abs(modelObj.x - findValue) < diff)
                        {
                            closestMarker = modelObj;
                            diff = Math.abs(modelObj.x - findValue);
                        }
                    }

                    // If Left/Right item doesn't exist just use the closest one
                    if(leftItem === null)
                        leftItem = closestMarker;

                    if(rightItem === null)
                        rightItem = closestMarker;

                    var leftPos = picker.markerToValue(leftItem);
                    var rightPos = picker.markerToValue(rightItem);
                    var clickPos = parseFloat(snapValue);
                    var blend = (clickPos - leftPos) / (rightPos - leftPos);
                    var oneMinusBlend = 1.0 - blend;

                    var mixcolor = Qt.rgba((leftItem.color.r * oneMinusBlend) + (rightItem.color.r * blend),
                                           (leftItem.color.g * oneMinusBlend) + (rightItem.color.g * blend),
                                           (leftItem.color.b * oneMinusBlend) + (rightItem.color.b * blend),
                                           1.0);

                    root.addMarker(clickPos, mixcolor);
                }
            }

            Repeater
            {
                id: markerRepeater

                delegate: Item
                {
                    id: marker

                    x: picker.valueToMarkerPosition(modelData.value)
                    y: 0
                    width: picker._markerWidth
                    height: picker.height

                    color: "white"

                    property alias color: canvas.color

                    function setColor(color)
                    {
                        marker.color = color;
                        root.alterMarker(index, modelData.value, marker.color);
                    }

                    Canvas
                    {
                        id: canvas

                        property color color: modelData.color
                        property color borderColor: systemPalette.dark

                        property bool highlighted: picker.selected === marker

                        width: picker._markerWidth
                        height: parent.height

                        onPaint:
                        {
                            var ctx = getContext("2d");
                            ctx.save();
                            ctx.clearRect(0, 0, picker._markerWidth, height);

                            ctx.fillStyle = color;
                            ctx.lineWidth = 1
                            ctx.strokeStyle = borderColor;
                            if(highlighted)
                                ctx.strokeStyle = systemPalette.highlight;

                            // Rectangle
                            ctx.beginPath();
                            ctx.moveTo(0, (height * 0.6) + 1);
                            ctx.lineTo(0, height);
                            ctx.lineTo(picker._markerWidth, height );
                            ctx.lineTo(picker._markerWidth, (height * 0.6) + 1);
                            ctx.fill();
                            ctx.stroke();
                            ctx.restore();

                            ctx.fillStyle = systemPalette.mid;
                            if(highlighted)
                                ctx.fillStyle = systemPalette.light;

                            // Triangle cap
                            ctx.beginPath();
                            ctx.moveTo(0, height * 0.6);
                            ctx.lineTo(picker._markerWidth / 2, 0);
                            ctx.lineTo(picker._markerWidth, height * 0.6);
                            ctx.lineTo(0, height * 0.6);
                            ctx.fill();
                            ctx.stroke();

                            ctx.restore();
                        }
                        onColorChanged: { requestPaint(); }
                        onBorderColorChanged: { requestPaint(); }
                        onHighlightedChanged: { requestPaint(); }
                    }

                    onXChanged: { toolTip.text = picker.markerToFormattedValue(marker); }

                    MouseArea
                    {
                        anchors.fill: parent

                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.threshold: 0
                        drag.minimumX: 0
                        drag.maximumX: picker.width - picker._markerWidth

                        onReleased:
                        {
                            // Snap X Positions to nearest 100th
                            marker.x = parseFloat(marker.x / (picker.width -
                                picker._markerWidth)).toFixed(2) *
                                (picker.width - picker._markerWidth);

                            toolTip.visible = false;

                            root.alterMarker(index, picker.markerToValue(marker), modelData.color);
                        }

                        onDoubleClicked:
                        {
                            if(mouse.button & Qt.LeftButton)
                            {
                                picker.selected = marker;
                                colorDialog.color = picker.selected.color;
                                colorDialog.visible = true;
                                toolTip.visible = false;
                            }
                        }

                        onPressed:
                        {
                            if(mouse.button & Qt.LeftButton)
                            {
                                picker.selected = marker;
                                toolTip.parent = marker;
                                toolTip.text = picker.markerToFormattedValue(marker);
                                toolTip.visible = true;
                            }
                        }

                        onClicked:
                        {
                            if(mouse.button & Qt.RightButton && root._markers.length > 2)
                                root.removeMarker(index);
                        }
                    }
                }
            }
        }
    }

    Rectangle
    {
        id: toolTip

        anchors.horizontalCenter: parent !== null ? parent.horizontalCenter : undefined
        anchors.bottom: parent !== null ? parent.top : undefined
        anchors.margins: 5

        width: parent !== null ? parent.width + 8 : 0
        height: toolTipText.height

        visible: false

        color: systemPalette.light
        border.color: systemPalette.midlight
        border.width: 1

        property alias text: toolTipText.text

        Text
        {
            anchors.horizontalCenter: parent.horizontalCenter
            id: toolTipText
            color: systemPalette.text
        }
    }

    ColorDialog
    {
        id: colorDialog
        title: qsTr("Select a Colour")
        onAccepted:
        {
            picker.selected.setColor(colorDialog.color);
        }
    }
}
