import QtQuick 2.7

Rectangle
{
    property Item item: parent
    property string backgroundColor: "transparent"
    property string borderColor: "blue"

    color: backgroundColor
    border
    {
        width: 1
        color: borderColor
    }

    anchors.fill: item
}

