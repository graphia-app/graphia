import QtQuick 2.7
import QtQuick.Layouts 1.3

Item
{
    width: 8
    height: 16
    Layout.margins: 6

    Rectangle
    {
        width: 1
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#22000000"
    }

    Rectangle
    {
        width: 1
        height: parent.height
        anchors.horizontalCenterOffset: 1
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#33ffffff"
    }
}
