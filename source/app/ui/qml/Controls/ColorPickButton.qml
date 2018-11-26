import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQml 2.8
import QtQuick.Dialogs 1.2

import "../../../../shared/ui/qml/Utils.js" as Utils

Item
{
    id: root

    implicitWidth: 64
    implicitHeight: 24

    property string color: "#FF00FF" // Obvious default colour
    property string dialogTitle: qsTr("Select a Colour")

    ColorDialog
    {
        id: colorDialog
        title: root.dialogTitle
        onColorChanged: { root.color = color; }
    }

    Button
    {
        id: button
        width: root.width
        height: root.height

        style: ButtonStyle
        {
            label: Rectangle
            {
                color: button.enabled ? root.color : Utils.desaturate(root.color, 0.2)
                radius: 2
            }
        }

        onClicked:
        {
            colorDialog.color = root.color;
            colorDialog.open();
        }
    }
}
