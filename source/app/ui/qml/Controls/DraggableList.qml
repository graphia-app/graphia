import QtQuick 2.7
import QtQml.Models 2.2

import "../Constants.js" as Constants

Column
{
    id: root

    property Component component
    property var model
    property color heldColor
    property Item parentWhenDragging
    property int alignment

    signal orderChanged()

    Component
    {
        id: dragDelegate

        MouseArea
        {
            id: dragArea

            anchors.left: alignment === Qt.AlignLeft ? parent.left : undefined
            anchors.right: alignment === Qt.AlignRight ? parent.right : undefined

            property bool held: false

            drag.target: held ? content : undefined
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: { return repeater.itemAt(repeater.count - 1).y; }

            property int _dragStartIndex: -1

            onPressed: { timer.start(); }
            Timer
            {
                id: timer
                interval: 200
                onTriggered: { held = true; }
            }

            onReleased: { timer.stop(); held = false; }

            onClicked:
            {
                if(loader.item.onClicked !== undefined)
                    loader.item.onClicked();
            }

            onDoubleClicked:
            {
                if(loader.item.onDoubleClicked !== undefined)
                    loader.item.onDoubleClicked();
            }

            width: content.width
            height: content.height

            Rectangle
            {
                id: content

                anchors
                {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }

                width: loader.item.width + Constants.margin * 2
                height: loader.item.height + Constants.margin

                color: Qt.rgba(root.heldColor.r, root.heldColor.g, root.heldColor.b, dragArea.held ? 1.0 : 0.0)
                Behavior on color { ColorAnimation { duration: 100 } }

                radius: 2

                Drag.source: dragArea
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2

                // http://stackoverflow.com/a/24729837/2721809
                Drag.dragType: Drag.Automatic
                property bool dragActive: dragArea.drag.active
                onDragActiveChanged:
                {
                    if(dragActive)
                    {
                        Drag.start();
                        dragStarted();
                    }
                    else
                    {
                        Drag.drop();
                        dragFinished();
                    }
                }

                signal dragStarted()
                signal dragFinished()
                //

                onDragStarted: { dragArea._dragStartIndex = index; }

                states: State
                {
                    when: dragArea.held

                    ParentChange { target: content; parent: parentWhenDragging }
                    AnchorChanges
                    {
                        target: content
                        anchors { horizontalCenter: undefined; verticalCenter: undefined }
                    }
                }

                Loader
                {
                    id: loader
                    sourceComponent: root.component

                    anchors
                    {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }

                    onLoaded:
                    {
                        if(item.index !== undefined)
                            item.index = index;

                        if(item.value !== undefined)
                        {
                            item.value = modelData;
                            item.valueChanged.connect(function() { modelData = item.value; });
                        }
                    }
                }
            }

            DropArea
            {
                id: dropArea
                anchors
                {
                    top: parent.top
                    bottom: parent.bottom
                    left: alignment === Qt.AlignLeft ? parent.left : undefined
                    right: alignment === Qt.AlignRight ? parent.right : undefined
                }
                width: root.width

                onEntered:
                {
                    var sourcePinned = repeater.itemAt(drag.source.DelegateModel.itemsIndex).pinned;
                    var targetPinned = repeater.itemAt(dragArea.DelegateModel.itemsIndex).pinned;

                    // Must both be pinned or neither pinned
                    if(sourcePinned === targetPinned)
                    {
                        root.model.move(
                                drag.source.DelegateModel.itemsIndex,
                                dragArea.DelegateModel.itemsIndex);
                    }
                }

                onDropped:
                {
                    if(dragArea._dragStartIndex !== dragArea.DelegateModel.itemsIndex)
                        root.orderChanged();
                }
            }

            property bool pinned: loader.item.pinned !== undefined ? loader.item.pinned : false
        }
    }

    DelegateModel
    {
        id: delegateModel
        model: root.model
        delegate: dragDelegate
    }

    Repeater
    {
        id: repeater
        model: delegateModel
    }
}
