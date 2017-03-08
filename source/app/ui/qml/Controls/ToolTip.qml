import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

// This is a gigantic hack that hijacks a Button's tooltip property in order
// to provide generalised tooltips. It's only really suitable for inert items
// as the button is still clickable and will not pass input events to the
// underlying control.
Item
{
    id: root
    anchors.fill: item

    property Item item: parent
    property string text

    Button
    {
        anchors.fill: parent

        tooltip: root.text

        style: ButtonStyle
        {
            background: Rectangle
            {
                color: "transparent"
            }
        }
    }
}
