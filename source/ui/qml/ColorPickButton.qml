import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQml 2.2
import QtQuick.Dialogs 1.2

Item
{
    id: root
    width: button.width
    height: button.height
    implicitWidth: button.implicitWidth
    implicitHeight: button.implicitHeight

    property string color: "#FF00FF" // Obvious default colour
    property string dialogTitle: qsTr("Select a Colour")

    ColorDialog
    {
        id: colorDialog
        title: root.dialogTitle
        onColorChanged: root.color = color;
    }

    Button
    {
        id: button
        width: 40

        style: ButtonStyle
        {
            label: Rectangle
            {
                color: root.color
            }
        }

        onClicked:
        {
            colorDialog.color = root.color;
            colorDialog.open();
        }
    }
}
