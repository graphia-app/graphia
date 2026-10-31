/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQml.Models

import Graphia.Utils

Item
{
    id: root

    // It is assumed that the component is always the same height
    property Component component
    property var model
    property color heldColor
    property int alignment
    property int count: repeater.count

    signal itemMoved(int from, int to)

    component DragArea: MouseArea
    {
        property Item item
        property int index
        property real hotspotY
        property bool pinned
    }

    implicitWidth: column.width
    implicitHeight: column.height

    function itemAt(index) { return (repeater.itemAt(index) as DragArea).item; }

    Component
    {
        id: dragDelegate

        DragArea
        {
            id: dragArea

            required property var modelData

            anchors.left: root.alignment === Qt.AlignLeft ? column.left : undefined
            anchors.right: root.alignment === Qt.AlignRight ? column.right : undefined

            acceptedButtons: Qt.AllButtons

            drag.target: held ? content : undefined
            drag.axis: Drag.YAxis
            drag.minimumY: root.y
            drag.maximumY: drag.minimumY + ((repeater.count - 1) * content.height)

            index: DelegateModel.itemsIndex
            item: loader.item as Item
            hotspotY: content.y + (content.height * 0.5)

            property bool held
            onPressAndHold: (mouse) => { held = true; }
            onReleased: (mouse) => { held = false; }

            property int _clickX
            property int _clickY

            onPositionChanged: (mouse) =>
            {
                let manhattan = Math.abs(_clickX - mouse.x) + Math.abs(_clickY - mouse.y);

                // If the mouse is significantly moved after a click, initiate a drag
                if(manhattan > 3)
                    held = true;

                if(!column.dragItem)
                    return;

                let newDropIndex = Math.floor(column.dragItem.hotspotY / content.height);
                if(newDropIndex < 0)
                    return;

                let dragItemPinned = column.dragItem.pinned;
                let dropItemPinned = (repeater.itemAt(newDropIndex) as DragArea).pinned;

                // Must both be pinned or neither pinned
                if(dragItemPinned === dropItemPinned)
                    column.dropIndex = newDropIndex;
            }

            onClicked: (mouse) =>
            {
                _clickX = mouse.x;
                _clickY = mouse.y;

                // Pass click events on to the child item(s)
                forwardEventToItem(mouse, "clicked");
            }

            onDoubleClicked: (mouse) => { forwardEventToItem(mouse, "doubleClicked"); }

            function forwardEventToItem(event, eventType)
            {
                let receivers = [];

                function recurse(item)
                {
                    if(typeof(item[eventType]) === "function")
                        receivers.push(item);

                    for(const child of item.children)
                    {
                        if(!child.visible || !child.enabled)
                            continue;

                        let p = mapToItem(child, event.x, event.y);

                        if(child.contains(p))
                            recurse(child);
                    }
                }

                recurse(loader.item);

                while(receivers.length > 0)
                {
                    let item = receivers[receivers.length - 1];

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

                width: dragArea.item ? dragArea.item.width + Constants.margin * 2 : 0
                height: dragArea.item ? dragArea.item.height + Constants.margin : 0

                color: Qt.rgba(root.heldColor.r, root.heldColor.g, root.heldColor.b, dragArea.held ? 1.0 : 0.0)
                Behavior on color { ColorAnimation { duration: 100 } }

                radius: 2

                Drag.source: dragArea
                Drag.keys: ["DraggableList"]

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

                onDragStarted:
                {
                    column.dragItem = repeater.itemAt(dragArea.index);
                    column.dragFromIndex = dragArea.index;
                }
                onDragFinished:
                {
                    column.dragItem = null;
                    column.dragFromIndex = column.dropIndex = -1;
                }
                //

                states: State
                {
                    when: dragArea.held

                    ParentChange { target: content; parent: root }
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
                        if(item.index !== undefined) // qmllint disable missing-property
                            item.index = dragArea.index; // qmllint disable missing-property

                        if(item.value !== undefined) // qmllint disable missing-property
                            item.value = dragArea.modelData; // qmllint disable missing-property
                    }
                }
            }

            pinned: loader.item.pinned ?? false // qmllint disable missing-property
        }
    }

    DelegateModel
    {
        id: delegateModel
        model: root.model
        delegate: dragDelegate
    }

    Column
    {
        id: column

        property int dragFromIndex: -1
        property DragArea dragItem: null
        property int dropIndex: -1

        Repeater
        {
            id: repeater
            model: delegateModel
        }

        onDropIndexChanged:
        {
            if(dropIndex < 0)
                return;

            // Do the move in the DelegateModel, so there is a visual change, but leave
            // the underlying model alone for now
            delegateModel.items.move(dragItem.index, dropIndex, 1);
        }

        move: Transition { SmoothedAnimation { properties: "y"; duration: 150 } }
    }

    DropArea
    {
        anchors.fill: column

        keys: ["DraggableList"]

        onDropped: (drop) =>
        {
            if(column.dragFromIndex === column.dropIndex)
                return;

            // Request that the underlying model performs a move
            root.itemMoved(column.dragFromIndex, column.dropIndex);
        }
    }
}
