import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

ToolButton
{
    id: root
    default property var content
    property int maximumToolTipWidth: 500
    property int maximumToolTipHeight: 300
    property string title: ""
    readonly property int _padding: 20
    readonly property int _offset: 10
    iconSource: "qrc:/icons/Tango/16x16/apps/help-browser.png"
    width: 20
    height: 20

    onContentChanged: { content.parent = containerLayout; }

    style: ButtonStyle
    {
        background: Rectangle {}
    }

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
                tooltip.x = root.mapToGlobal(root.width, 0).x + _offset
                tooltip.y = root.mapToGlobal(0, 0).y - (tooltip.height * 0.5)
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
        width: Math.min(wrapperLayout.implicitWidth, maximumToolTipWidth) + _padding
        height: Math.min(wrapperLayout.implicitHeight, maximumToolTipHeight) + _padding
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

        SystemPalette
        {
            id: sysPalette
        }

        Rectangle
        {
            id: backRectangle
            color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
            border.width: 1
            border.color: sysPalette.dark
            width: Math.min(wrapperLayout.implicitWidth, maximumToolTipWidth) + _padding
            height: Math.min(wrapperLayout.implicitHeight, maximumToolTipHeight) + _padding
            radius: 3
        }

        ColumnLayout
        {
            id: wrapperLayout
            anchors.centerIn: backRectangle
            anchors.fill: parent
            anchors.margins: _padding * 0.5
            Text
            {
                text: root.title
                font.pointSize: 12
            }
            ColumnLayout
            {
                id: containerLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
