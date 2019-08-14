import QtQuick 2.7
import QtQuick.Controls.Private 1.0

// This is a gigantic hack that delves into QtQuick.Controls.Private to
// provide the ability to add passive tooltips to things that don't otherwise
// have them (like Images). Note that this prevents the underlying Item's hover
// functionality from working, so may break things in exciting ways.
Item
{
    id: root
    anchors.fill: item

    property Item item: parent
    property string text

    MouseArea
    {
        id: mouseArea

        anchors.fill: parent

        // Pass any clicks through
        propagateComposedEvents: true
        onPressed: { mouse.accepted = false; }

        hoverEnabled: true

        onExited: { Tooltip.hideText(); }
        onCanceled: { Tooltip.hideText(); }
    }

    Timer
    {
        interval: 1000
        running: mouseArea.containsMouse && !mouseArea.pressed &&
            root.text.length > 0

        onTriggered:
        {
            Tooltip.showText(root, Qt.point(mouseArea.mouseX,
                mouseArea.mouseY), root.text);
        }
    }
}
