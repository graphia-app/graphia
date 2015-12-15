import QtQuick 2.5

Rectangle
{
    property Item item
    property string backgroundColor: "transparent"
    property string borderColor: "blue"

    color: backgroundColor
    border
    {
        width: 1
        color: borderColor
    }

    x: item.x
    y: item.y
    width: item.width
    height: item.height
}

