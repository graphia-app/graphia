import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.5

ToolButton
{
    id: root
    default property var content
    property int maximumToolTipWidth: 500
    property int maximumToolTipHeight: 300
    readonly property int _padding: 10
    iconName: "help-browser"

    onContentChanged: { content.parent = containerLayout; }

    Timer
    {
        id: hoverTimer
        interval: 500
        onTriggered: tooltip.visible = true
    }

    MouseArea
    {
        anchors.fill: parent
        hoverEnabled: true
        onHoveredChanged:
        {
            if(containsMouse)
            {
                tooltip.x = root.mapToGlobal(root.width, 0).x + _padding
                tooltip.y = root.mapToGlobal(0, 0).y
                hoverTimer.start();
            }
            else
            {
                tooltip.visible = false;
                hoverTimer.stop();
            }
        }
    }

    Window
    {
        id: tooltip
        width: Math.min(containerLayout.implicitWidth, maximumToolTipWidth) + _padding
        height: Math.min(containerLayout.implicitHeight, maximumToolTipHeight) + _padding
        // Magic flags: No shadows, transparent, no focus snatching, no border
        flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup
        color: "#00000000"
        opacity: 0

        onVisibilityChanged:
        {
            tooltip.opacity = visible ? 1.0 : 0
        }

        Behavior on opacity
        {
            PropertyAnimation
            {
                duration: 100
            }
        }

        Rectangle
        {
            id: backRectangle
            color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
            width: Math.min(containerLayout.implicitWidth, maximumToolTipWidth) + _padding
            height: Math.min(containerLayout.implicitHeight, maximumToolTipHeight) + _padding
            radius: 10
        }

        ColumnLayout
        {
            anchors.centerIn: backRectangle
            id: containerLayout
            anchors.fill: parent
            anchors.margins: 5
        }
    }
}
