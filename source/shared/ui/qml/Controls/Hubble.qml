import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item
{
    id: root

    default property var content
    property var _mouseArea
    property var _padding: 10

    property var title
    property var target
    property int alignment: Qt.AlignLeft

    visible: false
    opacity: 0.0
    z: 99

    onContentChanged: { content.parent = containerLayout }

    Behavior on opacity
    {
        PropertyAnimation
        {
            duration: 100
        }
    }

    onVisibleChanged:
    {
        if(visible)
            opacity = 1.0;
        else
            opacity = 0.0;
    }

    Timer
    {
        id: hoverTimer
        interval: 500
        onTriggered: root.visible = true
    }

    Rectangle
    {
        color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
        width: containerLayout.width + _padding
        height: containerLayout.height + _padding
        radius: 3

        ColumnLayout
        {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            id: containerLayout

            Text
            {
                text: title
                font.pointSize: 15
            }
        }
    }

    Component.onCompleted:
    {
        if(target === undefined)
            target = parent;

        if(target)
        {
            var component = Qt.createComponent("HelpMouseArea.qml");
            if (target.hoveredChanged === undefined)
                return;
            target.hoveredChanged.connect(function()
            {
                if(target.hovered)
                {
                    hoverTimer.start();
                    root.parent = mainWindow.toolBar;
                    positionBubble();
                }
                else
                {
                    hoverTimer.stop()
                    root.visible = false
                }
            });
        }
    }

    function positionBubble()
    {
        switch(alignment)
        {
        case Qt.AlignLeft:
            var point = target.mapToItem(parent, 0, target.height * 0.5);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y - (childrenRect.height * 0.5);
            break;
        case Qt.AlignRight:
            var point = target.mapToItem(parent, target.width, target.height * 0.5);
            root.x = point.x + _padding
            root.y = point.y - (childrenRect.height * 0.5);
            break;
        case Qt.AlignTop:
            var point = target.mapToItem(parent, 0, target.width * 0.5);
            root.x = target.width * 0.5
            root.y = - child.height - _padding
            break;
        case Qt.AlignBottom:
            root.x = target.width * 0.5
            root.y = target.height + _padding
            break;
        case Qt.AlignLeft | Qt.AlignTop:
            var point = target.mapToItem(parent, 0, 0);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y - childrenRect.height - _padding;
            break;
        case Qt.AlignRight | Qt.AlignTop:
            var point = target.mapToItem(parent, target.width, 0);
            root.x = point.x + _padding
            root.y = point.y - _padding;
            break;
        case Qt.AlignLeft | Qt.AlignBottom:
            var point = target.mapToItem(parent, 0, target.height);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y + _padding;
            break;
        case Qt.AlignRight | Qt.AlignBottom:
            var point = target.mapToItem(parent, target.width, target.height);
            root.x = point.x + _padding;
            root.y = point.y + _padding;
            break;
        }
    }
}
