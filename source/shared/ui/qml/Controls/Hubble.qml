import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Item
{
    id: root

    default property var content
    property var _mouseArea
    property int _padding: 10
    property alias color: backRectangle.color

    property var title
    property var target
    property int alignment: Qt.AlignLeft
    property bool displayButtons: false
    property bool hoverEnabled: false

    height: backRectangle.height
    width: backRectangle.width

    signal nextClicked()
    signal skipClicked()

    visible: false
    opacity: 0.0
    z: 99

    onContentChanged: { content.parent = containerLayout }
    onTargetChanged: { linkToTarget() }

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
        {
            if(target)
            {
                positionBubble();
            }

            opacity = 1.0;
        }
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
        id: backRectangle
        color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
        width: containerLayout.width + _padding
        height: containerLayout.height + nextSkipButtons.height + _padding
        radius: 3

        ColumnLayout
        {
            anchors.verticalCenter: backRectangle.verticalCenter
            anchors.horizontalCenter: backRectangle.horizontalCenter
            ColumnLayout
            {
                id: containerLayout

                Text
                {
                    text: title
                    font.pointSize: 15
                }
            }
            RowLayout
            {
                id: nextSkipButtons
                visible: displayButtons
                Text
                {
                    text: qsTr("Skip")
                    font.underline: true
                    MouseArea
                    {
                        cursorShape: Qt.PointingHandCursor
                        anchors.fill: parent
                        onClicked: skipClicked();
                    }
                }
                Rectangle
                {
                    Layout.fillWidth: true
                }
                Button
                {
                    id: nextButton
                    text: qsTr("Next")
                    onClicked:
                    {
                        nextClicked();
                    }
                }
            }
        }
    }

    Component.onCompleted:
    {
        linkToTarget();
    }

    function linkToTarget()
    {
        if(target)
        {
            if(hoverEnabled)
            {
                var component = Qt.createComponent("HelpMouseArea.qml");
                if(target.hoveredChanged === undefined)
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
            else
            {
                root.parent = mainWindow.toolBar;
                positionBubble();
            }
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
            var point = target.mapToItem(parent, target.width * 0.5, 0);
            root.x = point.x - (childrenRect.width * 0.5);
            root.y = point.y - (childrenRect.height) - _padding;
            break;
        case Qt.AlignBottom:
            var point = target.mapToItem(parent, target.width * 0.5, target.height);
            root.x = point.x - (childrenRect.width * 0.5);
            root.y = point.y + _padding;
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
