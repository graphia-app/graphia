import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Item
{
    id: root

    default property var content
    readonly property int _padding: 10
    property alias color: backRectangle.color

    property var title
    property var target
    property int alignment: Qt.AlignLeft
    property bool displayNext: false
    property bool displayClose: false
    property bool hoverEnabled: false
    property Item _mouseCapture

    height: backRectangle.height
    width: backRectangle.width

    signal nextClicked()
    signal closeClicked()
    signal skipClicked()

    visible: false
    opacity: 0.0
    z: 99

    onContentChanged: { content.parent = containerLayout; }
    onTargetChanged:
    {
        unlinkTarget();
        linkToTarget()
    }

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
                positionBubble();

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
        width: fullHubbleLayout.width + _padding
        height: fullHubbleLayout.height + _padding
        radius: 3

        ColumnLayout
        {
            id: fullHubbleLayout
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
                visible: displayNext || displayClose
                id: nextSkipButtons
                Text
                {
                    visible: displayNext
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
                    visible: displayNext
                    id: nextButton
                    text: qsTr("Next")
                    onClicked:
                    {
                        nextClicked();
                    }
                }
                Button
                {
                    id: closeButton
                    visible: displayClose
                    text: qsTr("Close")
                    onClicked:
                    {
                        closeClicked();
                    }
                }
            }
        }
    }

    Component.onCompleted:
    {
        linkToTarget();
    }

    function unlinkTarget()
    {
        if(_mouseCapture !== null)
        {
            for(var i = _mouseCapture.children.length; i >= 0; i--)
            {
                _mouseCapture.children[i].parent = _mouseCapture.parent;
            }
            _mouseCapture.parent = null;
        }
    }

    function linkToTarget()
    {
        if(target !== undefined && target !== null)
        {
            if(hoverEnabled)
            {
                // Use targets hoverChanged signal
                if(target.hoveredChanged !== undefined)
                {
                    target.hoveredChanged.connect(onHover);
                }
                else
                {
                    // If the target doesn't have a hover signal, we'll
                    // shim it with a HoverMousePassthrough item
                    if(_mouseCapture === undefined || _mouseCapture === null)
                    {
                        var component = Qt.createComponent("HoverMousePassthrough.qml");
                        _mouseCapture = component.createObject(target.parent);
                    }
                    if(target.parent !== _mouseCapture)
                    {
                        _mouseCapture.width = Qt.binding(function() { return target.width; });
                        _mouseCapture.height = Qt.binding(function() { return target.height; });

                        _mouseCapture.parent = target.parent;
                        target.parent = _mouseCapture;

                        _mouseCapture.hoveredChanged.connect(onHover);
                    }
                }
            }
            else
            {
                root.parent = mainWindow.toolBar;
                positionBubble();
            }
        }
    }

    function onHover()
    {
        var hoverTarget = _mouseCapture !== null ? _mouseCapture : target;
        if(hoverTarget.hovered)
        {
            hoverTimer.start();
            root.parent = mainWindow.toolBar;
            positionBubble();
        }
        else
        {
            hoverTimer.stop();
            root.visible = false;
        }
    }

    function positionBubble()
    {
        var point = {};

        switch(alignment)
        {
        case Qt.AlignLeft:
            point = target.mapToItem(parent, 0, target.height * 0.5);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y - (childrenRect.height * 0.5);
            break;
        case Qt.AlignRight:
            point = target.mapToItem(parent, target.width, target.height * 0.5);
            root.x = point.x + _padding
            root.y = point.y - (childrenRect.height * 0.5);
            break;
        case Qt.AlignTop:
            point = target.mapToItem(parent, target.width * 0.5, 0);
            root.x = point.x - (childrenRect.width * 0.5);
            root.y = point.y - childrenRect.height - _padding;
            break;
        case Qt.AlignBottom:
            point = target.mapToItem(parent, target.width * 0.5, target.height);
            root.x = point.x - (childrenRect.width * 0.5);
            root.y = point.y + _padding;
            break;
        case Qt.AlignLeft | Qt.AlignTop:
            point = target.mapToItem(parent, 0, 0);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y - childrenRect.height - _padding;
            break;
        case Qt.AlignRight | Qt.AlignTop:
            point = target.mapToItem(parent, target.width, 0);
            root.x = point.x + _padding
            root.y = point.y - _padding;
            break;
        case Qt.AlignLeft | Qt.AlignBottom:
            point = target.mapToItem(parent, 0, target.height);
            root.x = point.x - childrenRect.width - _padding;
            root.y = point.y + _padding;
            break;
        case Qt.AlignRight | Qt.AlignBottom:
            point = target.mapToItem(parent, target.width, target.height);
            root.x = point.x + _padding;
            root.y = point.y + _padding;
            break;
        }
    }
}
