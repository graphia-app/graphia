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
    property int count: repeater.count

    signal itemMoved(int from, int to)

    Component
    {
        id: dragDelegate

        MouseArea
        {
            id: dragArea

            anchors.left: alignment === Qt.AlignLeft ? parent.left : undefined
            anchors.right: alignment === Qt.AlignRight ? parent.right : undefined

            drag.target: held ? content : undefined
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: { return repeater.itemAt(repeater.count - 1).y; }

            property int _dragStartIndex: -1

            property bool held
            onPressAndHold: held = true;
            onReleased: held = false;

            property int _clickX
            property int _clickY

            // If the mouse is significantly moved after a click, initiate a drag
            onPositionChanged:
            {
                var manhattan = Math.abs(_clickX - mouse.x) + Math.abs(_clickY - mouse.y);

                if(manhattan > 3)
                    held = true;
            }

            onClicked:
            {
                _clickX = mouse.x;
                _clickY = mouse.y;

                // Pass click events on to the child item(s)
                forwardEventToItem(mouse, "clicked");
            }

            onDoubleClicked: { forwardEventToItem(mouse, "doubleClicked"); }

            function forwardEventToItem(event, eventType)
            {
                var receivers = [];

                function recurse(item)
                {
                    if(typeof(item[eventType]) === "function")
                        receivers.push(item);

                    for(var i = 0; i < item.children.length; i++)
                    {
                        var child = item.children[i];

                        if(!child.visible || !child.enabled)
                            continue;

                        var p = mapToItem(child, event.x, event.y);

                        if(child.contains(p))
                            recurse(child);
                    }
                }

                recurse(loader.item);

                while(receivers.length > 0)
                {
                    var item = receivers[receivers.length - 1];

                    item[eventType](event);
                    if(event.accepted)
                        break;

                    receivers.pop();
                }
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
                onDragFinished: { dragArea._dragStartIndex = -1; }

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
                        // Do the move in the DelegateModel, so there is a visual change, but leave
                        // the underlying model alone for now
                        delegateModel.items.move(
                                drag.source.DelegateModel.itemsIndex,
                                dragArea.DelegateModel.itemsIndex, 1);
                    }
                }

                onDropped:
                {
                    if(dragArea._dragStartIndex !== dragArea.DelegateModel.itemsIndex)
                    {
                        // Request that the underlying model performs a move
                        root.itemMoved(dragArea._dragStartIndex,
                                       dragArea.DelegateModel.itemsIndex);
                    }
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
