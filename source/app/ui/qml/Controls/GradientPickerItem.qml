import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import ".."
import "../Constants.js" as Constants
import "../Utils.js" as Utils

import "../Controls/"

Item
{
    id: root
    height: picker.visible ? 60 + picker.height : 60
    implicitHeight: height

    property alias expanded: picker.visible
    property alias checked: picker.visible
    property alias currentGradient: gradientKey.configuration
    property bool showTooltip: true
    property var deleteFunction: null
    property bool selected: false
    property bool expandable: true
    property bool initialising: false
    property ExclusiveGroup exclusiveGroup: null

    signal gradientChanged()
    signal clicked()

    function initialise(gradientString)
    {
        initialising = true;
        gradientKey.configuration = gradientString;
        markerRepeater.loadGradientString(gradientString);
        initialising = false;
    }

    onExclusiveGroupChanged:
    {
        if(exclusiveGroup)
            exclusiveGroup.bindCheckable(root);
    }

    SystemPalette
    {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    Column
    {
        id: layout

        anchors.left: root.left
        anchors.right: root.right
        anchors.top: root.top

        spacing: 0

        GradientKey
        {
            width: root.width
            id: gradientKey
            implicitHeight: picker.visible ? root.height - picker.height : root.height

            _padding: picker.markerWidth
            _paddingTopBottom: 0

            configuration: ""

            showLabels: false
            hoverEnabled: !expanded
            hoverColor: systemPalette.highlight
            onClicked: { root.clicked(); }

            Column
            {
                id: btnHolder
                z: 2
                opacity: togglePickerButton.hovered ||
                         deleteButton.hovered ||
                         pickerMouseArea.containsMouse ||
                         expanded ? 1.0 : 0.0
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                Behavior on opacity
                {
                    PropertyAnimation { properties: "opacity"; }
                }
                ToolButton
                {
                    id: togglePickerButton
                    visible: expandable
                    iconName: "view-fullscreen"
                    onClicked:
                    {
                        gradientItem.expanded = !gradientItem.expanded;
                        root.clicked()
                    }
                }
                ToolButton
                {
                    id: deleteButton
                    visible: deleteFunction !== null
                    iconName: "edit-delete"
                    onClicked:
                    {
                        deleteDialog.visible = true;
                    }
                }
            }

            MouseArea
            {
                id: pickerMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: { root.clicked(); }
            }
        }

        Item
        {
            id: picker
            property var selected
            property int markerWidth: 20
            property int finalHeight: 20
            width: root.width
            height: 20

            visible: false

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
                width: parent.width - parent.markerWidth
            }

            MouseArea
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height
                width: parent.width - (picker.markerWidth)
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

                    var mixcolor = Qt.rgba((leftItem.color.r + rightItem.color.r) * 0.5,
                                           (leftItem.color.g + rightItem.color.g) * 0.5,
                                           (leftItem.color.b + rightItem.color.b) * 0.5,
                                           1.0);
                    var newItem = {};
                    newItem.gradientValue = parseFloat(snapValue);
                    newItem.gradientColor = mixcolor.toString();
                    markerRepeater.model.append(newItem);
                    markerRepeater.generateGradientString();
                }
            }

            onSelectedChanged:
            {
                if(selected !== undefined && selected !== null)
                {
                    // Select new one
                    selected.highlighted = true;
                }
            }

            Repeater
            {
                id: markerRepeater

                function generateGradientString()
                {
                    var currentModel = new Object();
                    // Regenerate Model
                    for(var i = 0; i < markerRepeater.count; i++)
                    {
                        var modelObj = markerRepeater.itemAt(i);
                        // Can happen on initialisation
                        if(modelObj === null)
                            continue;
                        var normalisedPos = picker.markerToValue(modelObj);
                        var modelColor = modelObj.color.toString();

                        currentModel[normalisedPos] = modelColor;
                    }
                    gradientKey.configuration = JSON.stringify(currentModel);
                }

                function loadGradientString(gradientString)
                {
                    var gradientModel = JSON.parse(gradientString);
                    markerRepeater.model.clear();

                    // Regenerate Model
                    for(var value in gradientModel)
                    {
                        markerRepeater.model.insert(markerRepeater.count,
                                                    {gradientValue: parseFloat(value), gradientColor: gradientModel[value]});
                    }
                }

                model: ListModel{}
                delegate: Item
                {
                    id: markerDelegate
                    x: picker.valueToMarkerPosition(gradientValue)
                    y: 0
                    width: picker.markerWidth
                    height: picker.height
                    color: "white"

                    property alias color: canvas.color
                    property alias borderColor: canvas.borderColor
                    property alias highlighted: canvas.highlighted

                    property var modelId: index

                    Canvas
                    {
                        id: canvas
                        property color color: gradientColor
                        property color borderColor: systemPalette.dark
                        property bool highlighted: false
                        width: picker.markerWidth
                        height: parent.height
                        property bool _firstDraw: true
                        onPaint:
                        {
                            var ctx = getContext("2d");
                            ctx.save();
                            ctx.clearRect(0, 0, picker.markerWidth, height);

                            ctx.fillStyle = color;
                            ctx.lineWidth = 1
                            ctx.strokeStyle = borderColor;
                            if(highlighted)
                                ctx.strokeStyle = systemPalette.highlight;

                            // Rectangle
                            ctx.beginPath();
                            ctx.moveTo(0, (height * 0.6) + 1);
                            ctx.lineTo(0, height);
                            ctx.lineTo(picker.markerWidth, height );
                            ctx.lineTo(picker.markerWidth, (height * 0.6) + 1);
                            ctx.fill();
                            ctx.stroke();
                            ctx.restore();

                            ctx.fillStyle = systemPalette.mid;
                            if(highlighted)
                                ctx.fillStyle = systemPalette.light;

                            // Triangle cap
                            ctx.beginPath();
                            ctx.moveTo(0, height * 0.6);
                            ctx.lineTo(picker.markerWidth / 2, 0);
                            ctx.lineTo(picker.markerWidth, height * 0.6);
                            ctx.lineTo(0, height * 0.6);
                            ctx.fill();
                            ctx.stroke();

                            ctx.restore();
                        }
                        onColorChanged: { requestPaint(); }
                        onBorderColorChanged: { requestPaint(); }
                        onHighlightedChanged: { requestPaint(); }
                    }

                    onXChanged: { toolTip.text = picker.markerToFormattedValue(markerDelegate); }

                    MouseArea
                    {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.threshold: 0
                        drag.minimumX: 0
                        drag.maximumX: picker.width - picker.markerWidth
                        onReleased:
                        {
                            // Snap X Positions to nearest 100th
                            parent.x = parseFloat(parent.x / (picker.width - picker.markerWidth)).toFixed(2)
                                * (picker.width - picker.markerWidth);

                            picker.selected = parent;

                            if(root.showTooltip)
                                toolTip.visible = false;

                            markerRepeater.generateGradientString();
                        }
                        onDoubleClicked:
                        {
                            if(mouse.button & Qt.LeftButton)
                            {
                                picker.selected = parent;
                                colorDialog.color = picker.selected.color;
                                colorDialog.visible = true;

                                if(root.showTooltip)
                                    toolTip.visible = false;
                            }
                        }
                        onPressed:
                        {
                            if(mouse.button & Qt.RightButton)
                            {
                                if(markerRepeater.model.count > 2)
                                    markerRepeater.model.remove(parent.modelId, 1);
                            }
                            else if(root.showTooltip && mouse.button & Qt.LeftButton)
                            {
                                toolTip.parent = markerDelegate;
                                toolTip.text = picker.markerToFormattedValue(markerDelegate);
                                toolTip.visible = true;
                            }
                        }
                        onClicked:
                        {
                            if(mouse.button & Qt.RightButton)
                            {
                                if(markerRepeater.model.count > 2)
                                    markerRepeater.model.remove(parent.modelId, 1);
                            }
                        }
                    }
                }
                onItemAdded:
                {
                    picker.selected = item;
                }
                onItemRemoved:
                {
                    if(!initialising)
                        markerRepeater.generateGradientString();
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
        height: _text.height
        z: 10
        visible: false
        color: systemPalette.light
        border.color: systemPalette.midlight
        border.width: 1
        property alias text: _text.text
        Text
        {
            anchors.horizontalCenter: parent.horizontalCenter
            id: _text
            color: systemPalette.text
            text: ""
        }
    }

    ColorDialog
    {
        id: colorDialog
        title: qsTr("Please choose a colour")
        onAccepted:
        {
            picker.selected.color = colorDialog.color;
            markerRepeater.generateGradientString();
        }
    }

    MessageDialog
    {
        id: deleteDialog
        visible: false
        title: qsTr("Delete Gradient?")
        text: qsTr("Are you sure you want to delete this gradient?")

        icon: StandardIcon.Warning
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: deleteFunction()
    }
}
