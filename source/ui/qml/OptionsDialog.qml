import QtQuick 2.5
import QtQuick.Window 2.2

Window
{
    flags: Qt.Window|Qt.Dialog

    Rectangle
    {
        anchors.fill: parent

        Text
        {
            anchors.centerIn: parent
            text: "Options Dialog"
        }
    }
}

