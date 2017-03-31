import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../Utils.js" as Utils

Item
{
    id: root

    width: layout.width
    height: layout.height

    implicitWidth: width
    implicitHeight: height

    property alias keyWidth: item.width

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

    property var configuration
    onConfigurationChanged:
    {
        var stops = [];

        for(var prop in configuration)
        {
            stops.push(stopComponent.createObject(rectangle,
                { "position": prop, "color": configuration[prop] }));
        }

        rectangle.gradient.stops = stops;
    }

    Rectangle
    {
        id: button

        anchors.centerIn: parent
        width: root.width + 2 * 4/*padding*/
        height: root.height + 2 * 4/*padding*/
        implicitWidth: width
        implicitHeight: height
        radius: 2
        color: (mouseArea.containsMouse) ? root.hoverColor : "transparent"
    }

    RowLayout
    {
        id: layout

        Label
        {
            visible: root.showLabels
            text: Utils.roundToDp(root.minimum, _decimalPoints)
            color: root.textColor
        }

        Item
        {
            // The wrapper item is here to give the
            // rotated Rectangle something to fill
            id: item

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
