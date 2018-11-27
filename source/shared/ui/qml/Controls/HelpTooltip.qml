import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

import "../"

ToolButton
{
    id: root
    default property var content
    property int maximumToolTipWidth: 500
    property int maximumToolTipHeight: 300
    property string title: ""
    readonly property int _padding: 20
    readonly property int _offset: 10

    // If you use a custom element style you can't use iconName
    // so iconSource must be used
    iconSource: "qrc:/icons/Tango/16x16/apps/help-browser.png"

    width: 20
    height: 20

    onContentChanged:
    {
        // Parent to the container
        content.parent = containerLayout;
        content.anchors.fill = parent;
        // Set to fill layout to allow for text wrapping
        content.Layout.fillWidth = true;
    }

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
                tooltip.x = root.mapToGlobal(root.width, 0).x + _offset;
                tooltip.y = root.mapToGlobal(0, 0).y - (wrapperLayout.implicitHeight * 0.5);

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
        height: backRectangle.height
        width: backRectangle.width
        // Magic flags: No shadows, transparent, no focus snatching, no border
        flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup
        color: "#00000000"
        opacity: 0
        x: 0
        y: 0

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
            color: Qt.rgba(0.96, 0.96, 0.96, 0.96)
            border.width: 1
            border.color: sysPalette.dark
            width: wrapperLayout.width + _padding
            height:  wrapperLayout.height + _padding
            radius: 3
        }

        ColumnLayout
        {
            id: wrapperLayout
            anchors.centerIn: backRectangle
            onWidthChanged:
            {
                // Clip widths if we have to. This allows wrapping
                if(width > maximumToolTipWidth)
                    width = maximumToolTipWidth;
            }
            width: containerLayout.width

            Text
            {
                text: root.title
                font.pointSize: FontPointSize.h2
            }
            ColumnLayout
            {
                id: containerLayout
            }
        }
    }
}
