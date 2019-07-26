import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

// This is a gigantic hack that hijacks a Button's tooltip property in order
// to provide generalised tooltips
Item
{
    id: root
    anchors.fill: item

    property Item item: parent
    property string text

    Button
    {
        id: tooltipButton
        anchors.fill: parent

        tooltip: root.text

        style: ButtonStyle
        {
            background: Rectangle
            {
                color: "transparent"
            }
        }

        // Pass any mouse events on to any underlying MouseAreas
        Component.onCompleted:
        {
            tooltipButton.__behavior.propagateComposedEvents = true;
        }

        Connections
        {
            target: tooltipButton.__behavior
            onPressed: { mouse.accepted = false; }
        }
    }
}
