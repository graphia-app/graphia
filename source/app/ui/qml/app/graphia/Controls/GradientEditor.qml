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
import QtQuick.Layouts
import QtQuick.Dialogs

import Qt.labs.platform as Labs

import app.graphia.Shared

Item
{
    id: root
    implicitHeight: picker.visible ? 60 + picker.height : 60

    property alias configuration: gradientKey.configuration

    signal clicked()

    property var _markers : []

    function setup(configuration)
    {
        let gradient = JSON.parse(configuration);

        let markers = [];

        for(let value in gradient)
            markers.push({value: value, color: gradient[value]});

        markerRepeater.model = root._markers = markers;
    }

    function addMarker(value, color)
    {
        let markers = root._markers;

        let newMarker = {};
        newMarker.value = value;
        newMarker.color = color.toString();
        markers.push(newMarker);

        markerRepeater.model = root._markers = markers;
    }

    function removeMarker(index)
    {
        let markers = root._markers;
        markers.splice(index, 1);

        markerRepeater.model = root._markers = markers;
    }

    function alterMarker(index, value, color)
    {
        let markers = root._markers;
        markers.splice(index, 1);

        addMarker(value, color);
    }

    function invert()
    {
        let markers = [];

        for(let i = 0; i < root._markers.length; i++)
        {
            let newValue = 1.0 - root._markers[i].value;
            let oldColor = root._markers[i].color;
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
                let o = {};

                for(let i = 0; i < _markers.length; i++)
                {
                    let marker = _markers[i];

                    let normalisedPos = marker.value;
                    let color = marker.color;

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
                onClicked: function(mouse)
                {
                    let leftItem = null;
                    let rightItem = null;
                    let closestMarker = null;
                    let snapValue = parseFloat(mouseX / width).toFixed(2);
                    let findValue = picker.valueToMarkerPosition(snapValue);
                    let diff = Number.MAX_VALUE;

                    // Find suitable left + right marker (also find the closest marker)
                    for(let i = 0; i < markerRepeater.count; i++)
                    {
                        let modelObj = markerRepeater.itemAt(i);
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

                    let leftPos = picker.markerToValue(leftItem);
                    let rightPos = picker.markerToValue(rightItem);
                    let clickPos = parseFloat(snapValue);
                    let blend = (clickPos - leftPos) / (rightPos - leftPos);
                    let oneMinusBlend = 1.0 - blend;

                    let mixcolor = Qt.rgba((leftItem.color.r * oneMinusBlend) + (rightItem.color.r * blend),
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
                        property color borderColor: palette.dark

                        property bool highlighted: picker.selected === marker

                        width: picker._markerWidth
                        height: parent.height

                        onPaint: function(rect)
                        {
                            let ctx = getContext("2d");
                            ctx.save();
                            ctx.clearRect(0, 0, picker._markerWidth, height);

                            ctx.fillStyle = color;
                            ctx.lineWidth = 1
                            ctx.strokeStyle = borderColor;
                            if(highlighted)
                                ctx.strokeStyle = palette.highlight;

                            // Rectangle
                            ctx.beginPath();
                            ctx.moveTo(0, (height * 0.6) + 1);
                            ctx.lineTo(0, height);
                            ctx.lineTo(picker._markerWidth, height);
                            ctx.lineTo(picker._markerWidth, (height * 0.6) + 1);
                            ctx.fill();
                            ctx.stroke();
                            ctx.restore();

                            ctx.fillStyle = palette.mid;
                            if(highlighted)
                                ctx.fillStyle = palette.light;

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

                        onReleased: function(mouse)
                        {
                            // Snap X Positions to nearest 100th
                            marker.x = parseFloat(marker.x / (picker.width -
                                picker._markerWidth)).toFixed(2) *
                                (picker.width - picker._markerWidth);

                            toolTip.visible = false;

                            root.alterMarker(index, picker.markerToValue(marker), modelData.color);
                        }

                        onDoubleClicked: function(mouse)
                        {
                            if(mouse.button & Qt.LeftButton)
                            {
                                picker.selected = marker;
                                colorDialog.color = picker.selected.color;
                                colorDialog.visible = true;
                                toolTip.visible = false;
                            }
                        }

                        onPressed: function(mouse)
                        {
                            if(mouse.button & Qt.LeftButton)
                            {
                                picker.selected = marker;
                                toolTip.parent = marker;
                                toolTip.text = picker.markerToFormattedValue(marker);
                                toolTip.visible = true;
                            }
                        }

                        onClicked: function(mouse)
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

        color: palette.light
        border.color: palette.midlight
        border.width: 1

        property alias text: toolTipText.text

        Text
        {
            anchors.horizontalCenter: parent.horizontalCenter
            id: toolTipText
            color: palette.text
        }
    }

    Labs.ColorDialog
    {
        id: colorDialog
        title: qsTr("Select a Colour")
        onAccepted:
        {
            picker.selected.setColor(colorDialog.color);
        }
    }
}
