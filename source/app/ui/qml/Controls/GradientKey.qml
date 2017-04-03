import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../Utils.js" as Utils

Item
{
    id: root

    implicitWidth: layout.implicitWidth + _padding
    implicitHeight: minimumLabel.implicitHeight + _padding

    property int _padding: 2 * 4

    property int keyWidth

    property double minimum
    property double maximum

    property int _decimalPoints: Utils.decimalPointsForRange(root.minimum, root.maximum)

    property color hoverColor
    property color textColor

    property bool showLabels: true
    property bool invert: false
    property bool propogatePresses: false

    Component
    {
        id: stopComponent
        GradientStop {}
    }

    function updateGradient()
    {
        if(configuration === undefined)
            return;

        var stops = [];

        for(var prop in configuration)
        {
            var color = configuration[prop];

            if(!root.enabled)
                color = Utils.desaturate(color);

            stops.push(stopComponent.createObject(rectangle.gradient,
                { "position": prop, "color": color }));
        }

        rectangle.gradient.stops = stops;
    }

    onEnabledChanged:
    {
        updateGradient();
    }

    property var configuration
    onConfigurationChanged:
    {
        updateGradient();
    }

    Rectangle
    {
        id: button

        anchors.centerIn: parent
        width: root.width
        height: root.height
        radius: 2
        color: (mouseArea.containsMouse) ? root.hoverColor : "transparent"
    }

    RowLayout
    {
        id: layout

        anchors.centerIn: parent

        width: root.width !== undefined ? root.width - _padding : undefined
        height: root.height !== undefined ? root.height - _padding : undefined

        Label
        {
            id: minimumLabel

            visible: root.showLabels
            text: Utils.roundToDp(root.minimum, _decimalPoints)
            color: root.textColor
        }

        Item
        {
            // The wrapper item is here to give the
            // rotated Rectangle something to fill
            id: item

            width: root.keyWidth !== 0 ? root.keyWidth : 0
            Layout.fillWidth: root.keyWidth === 0
            Layout.fillHeight: true

            Rectangle
            {
                id: rectangle

                anchors.centerIn: parent
                width: parent.height
                height: parent.width
                radius: 4

                border.width: 1
                border.color: root.textColor

                rotation: root.invert ? 90 : -90
                gradient: Gradient {}
            }
        }

        Label
        {
            id: maximumLabel

            visible: root.showLabels
            text: Utils.roundToDp(root.maximum, _decimalPoints)
            color: root.textColor
        }
    }

    MouseArea
    {
        id: mouseArea

        anchors.fill: root

        onClicked: root.clicked()
        onDoubleClicked: root.doubleClicked()

        hoverEnabled: true

        onPressed: { mouse.accepted = !propogatePresses; }
    }

    signal clicked()
    signal doubleClicked()
}
