import QtQuick 2.7
import QtQml.Models 2.2
import QtQuick.Controls 1.4

import "../Constants.js" as Constants

Item
{
    id: root
    width: transformsColumn.width
    height: transformsColumn.height

    property var model
    property var document

    property color enabledTextColor
    property color disabledTextColor
    property color heldColor

    enabled: document.idle
    model: document.transforms

    Component
    {
        id: dragDelegate

        MouseArea
        {
            id: dragArea

            anchors.right: parent.right

            property bool held: false

            drag.target: held ? content : undefined
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: { return transformsRepeater.itemAt(transformsRepeater.count - 1).y; }

            property int dragStartIndex

            onPressed: { timer.start(); }
            Timer
            {
                id: timer
                interval: 200
                onTriggered: { held = true; }
            }

            onReleased: { timer.stop(); held = false; }

            onDoubleClicked: { transform.toggleLock(); }

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

                width: transform.width + Constants.margin * 2
                height: transform.height + Constants.margin

                color: dragArea.held ? Qt.rgba(heldColor.r, heldColor.g, heldColor.b, 1.0) :
                                       Qt.rgba(heldColor.r, heldColor.g, heldColor.b, 0.0)
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

                onDragStarted: { dragArea.dragStartIndex = index; }

                states: State
                {
                    when: dragArea.held

                    ParentChange { target: content; parent: transformsColumn.parent }
                    AnchorChanges
                    {
                        target: content
                        anchors { horizontalCenter: undefined; verticalCenter: undefined }
                    }
                }

                Transform
                {
                    property var document: root.document

                    id: transform

                    anchors
                    {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }

                    Component.onCompleted:
                    {
                        enabledTextColor = Qt.binding(function() { return root.enabledTextColor; });
                        disabledTextColor = Qt.binding(function() { return root.disabledTextColor; });
                    }
                }
            }

            DropArea
            {
                id: dropArea
                anchors { fill: parent }

                onEntered:
                {
                    var sourcePinned = transformsRepeater.itemAt(drag.source.DelegateModel.itemsIndex).pinned;
                    var targetPinned = transformsRepeater.itemAt(dragArea.DelegateModel.itemsIndex).pinned;

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
                    if(dragArea.dragStartIndex !== dragArea.DelegateModel.itemsIndex)
                        document.updateGraphTransforms();
                }
            }

            property bool pinned: transform.pinned
        }
    }

    DelegateModel
    {
        id: delegateModel
        model: root.model
        delegate: dragDelegate
    }

    CreateTransformDialog
    {
        id: createTransformDialog

        document: root.document
    }

    Column
    {
        id: transformsColumn

        Repeater
        {
            id: transformsRepeater
            model: delegateModel
        }

        Item
        {
            // This is a bit of a hack to get margins around the add button:
            // As the button has no top anchor, anchors.topMargin doesn't
            // work, so instead we just pad out the button by the margin
            // size with a parent Item

            anchors.right: parent.right

            width: addButton.width + Constants.margin * 2
            height: addButton.height + Constants.margin * 2

            Button
            {
                id: addButton

                enabled: root.document.idle

                anchors
                {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }

                text: qsTr("Add Transform")
                onClicked: { createTransformDialog.show(); }
            }
        }
    }
}
