import QtQuick 2.7
import QtQuick.Controls 1.4

Item
{
    property real radius: height * 0.1
    property color color: "white"
    property color hoverColor: color

    property color _displayColor: mouseArea.containsMouse ? hoverColor : color

    property real _barToSpaceRatio: 1.0
    property real _d: 2.0 + 3.0 * _barToSpaceRatio
    property real _barHeight: (_barToSpaceRatio * height) / _d
    property real _spaceHeight: height / _d

    property Menu menu: null

    Rectangle
    {
        x: 0
        y: 0
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    Rectangle
    {
        x: 0
        y: _barHeight + _spaceHeight
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    Rectangle
    {
        x: 0
        y: parent.height - _barHeight
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    MouseArea
    {
        id: mouseArea

        hoverEnabled: true
        anchors.fill: parent
        onClicked:
        {
            if(menu)
                menu.__popup(parent.mapToItem(null, 0, parent.height + 4/*padding*/, 0, 0), 0);
        }
    }
}
