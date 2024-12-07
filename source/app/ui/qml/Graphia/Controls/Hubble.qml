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

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

Item
{
    id: root

    default property var content
    readonly property int _padding: 10
    property alias color: backRectangle.color

    property string title: ""
    property var target

    property int alignment: Qt.AlignLeft | Qt.AlignTop
    property int edges: Qt.LeftEdge | Qt.TopEdge

    property bool displayPrevious: false
    property bool displayNext: false
    property bool displayClose: false
    property bool tooltipMode: false
    property Item _mouseCapture

    height: backRectangle.height
    width: backRectangle.width

    signal previousClicked()
    signal nextClicked()
    signal closeClicked()
    signal skipClicked()

    visible: false

    Preferences
    {
        id: misc
        section: "misc"
        property bool disableHubbles
    }

    opacity: 0.0
    z: 99

    onContentChanged: { content.parent = containerLayout; }
    onTargetChanged:
    {
        unlinkFromTarget();
        linkToTarget();
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
        if(misc.disableHubbles && tooltipMode)
        {
            visible = false;
            return;
        }

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
        interval: 750
        onTriggered: root.visible = true
    }

    Rectangle
    {
        id: backRectangle
        color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
        width: mainLayout.width + _padding
        height: mainLayout.height + _padding
        radius: 3
    }

    ColumnLayout
    {
        id: mainLayout
        anchors.centerIn: backRectangle

        ColumnLayout
        {
            id: containerLayout

            Text
            {
                text: title
                font.pointSize: FontPointSize.h2
            }
        }

        RowLayout
        {
            Layout.preferredWidth: containerLayout.width

            visible: displayPrevious || displayNext || displayClose

            Text
            {
                visible: !displayClose
                text: qsTr("Skip")
                font.underline: true
                MouseArea
                {
                    cursorShape: Qt.PointingHandCursor
                    anchors.fill: parent
                    onClicked: function(mouse) { skipClicked(); }
                }
            }

            Item { Layout.fillWidth: true }

            Button
            {
                visible: displayPrevious
                text: qsTr("Previous")
                onClicked: function(mouse) { previousClicked(); }
            }

            Button
            {
                visible: displayNext
                text: qsTr("Next")
                onClicked: function(mouse) { nextClicked(); }
            }

            Button
            {
                visible: displayClose
                text: qsTr("Close")
                onClicked: function(mouse) { closeClicked(); }
            }
        }
    }

    Component.onCompleted:
    {
        linkToTarget();
    }

    function replaceInParent(replacee, replacer)
    {
        let parent = replacee.parent;

        if(parent === null || parent === undefined)
        {
            console.log("replacee has no parent, giving up");
            return;
        }

        let tail = [];
        for(let index = 0; index < parent.children.length; index++)
        {
            let child = parent.children[index];
            if(child === replacee)
            {
                // Make the replacee an orphan
                child.parent = null;

                // Move all the remaining children to an array
                while(index < parent.children.length)
                {
                    child = parent.children[index];
                    tail.push(child);
                    child.parent = null;
                }

                break;
            }
        }

        if(replacee.parent === null)
        {
            // Parent the replacer
            replacer.parent = parent;

            // Reattach the original children
            tail.forEach(function(child)
            {
                child.parent = parent;
            });
        }
    }

    function unlinkFromTarget()
    {
        // Remove the mouse capture shim
        // Heirarchy was parent->mouseCapture->target
        // Remove mouseCapture and reparent the child items to parent
        // Results in parent->item
        if(_mouseCapture !== null && _mouseCapture.children.length > 0)
        {
            if(_mouseCapture.children.length > 1)
                console.log("HoverMousePassthrough has more than one child; it shouldn't");

            replaceInParent(_mouseCapture, _mouseCapture.children[0]);
        }
    }

    Component
    {
        id: hoverMousePassthroughComponent
        HoverMousePassthrough {}
    }

    function linkToTarget()
    {
        if(target !== undefined && target !== null)
        {
            if(tooltipMode)
            {
                // Use target's hoverChanged signal
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
                        // root as parent is temporary, until it gets set for real below
                        _mouseCapture = hoverMousePassthroughComponent.createObject(root);
                    }

                    // Insert the mouse capture shim
                    // Hierarchy was parent->target
                    // Add mouseCapture below parent and reparent the child items to mousecapture
                    // results in: parent->mouseCapture->target
                    // This allows mouseCapture to access all mouse hover events!
                    if(target.parent !== _mouseCapture)
                    {
                        // When the target item's visibility is false, our shim should also be invisible,
                        // but we can't change its visible property because that would in turn explicity set the
                        // child's visible property and obviously we don't want that. Instead therefore, we fake
                        // visibility by making the shim's size very small. Unforunately, we can't use 0 for the
                        // size since it means "invalid" in the context of implicit size.
                        //
                        // FIXME: This is a bit of a crap solution really, because it means that:
                        //        a) the shim is never truly invisible, and at best will be a single pixel big
                        //        b) in the case of the item being in a layout, it will potentially add the
                        //           layout's spacing parameter to the sides of said single pixel when really
                        //           the entire thing should be occupying zero space
                        _mouseCapture.implicitWidth = Qt.binding(function()
                            { return target.visible ? target.implicitWidth : 1; });
                        _mouseCapture.implicitHeight = Qt.binding(function()
                            { return target.visible ? target.implicitHeight : 1; });

                        // If we can't see the target, disable the hubble
                        _mouseCapture.enabled = Qt.binding(() => target.visible);

                        replaceInParent(target, _mouseCapture);
                        target.parent = _mouseCapture;

                        _mouseCapture.hoveredChanged.connect(onHover);
                    }
                }
            }
            else
            {
                root.parent = mainWindow.header;
                positionBubble();
            }
        }
    }

    function onHover()
    {
        let hoverTarget = _mouseCapture !== null ? _mouseCapture : target;
        if(hoverTarget.hovered)
        {
            hoverTimer.start();
            root.parent = mainWindow.header;
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
        let point = {};

        if((alignment & Qt.AlignLeft) && (edges & Qt.LeftEdge))
            root.x = target.mapToItem(parent, 0.0, 0.0).x;
        else if((alignment & Qt.AlignLeft) && (edges & Qt.RightEdge))
            root.x = target.mapToItem(parent, target.width, 0.0).x + _padding;
        else if((alignment & Qt.AlignRight) && (edges & Qt.RightEdge))
            root.x = target.mapToItem(parent, target.width, 0.0).x - childrenRect.width;
        else if((alignment & Qt.AlignRight) && (edges & Qt.LeftEdge))
            root.x = target.mapToItem(parent, 0.0, 0.0).x - childrenRect.width - _padding;

        if((alignment & Qt.AlignTop) && (edges & Qt.TopEdge))
            root.y = target.mapToItem(parent, 0.0, 0.0).y;
        else if((alignment & Qt.AlignTop) && (edges & Qt.BottomEdge))
            root.y = target.mapToItem(parent, 0.0, 0.0).y - childrenRect.height - _padding;
        else if((alignment & Qt.AlignBottom) && (edges & Qt.BottomEdge))
            root.y = target.mapToItem(parent, 0.0, target.height).y - childrenRect.height;
        else if((alignment & Qt.AlignBottom) && (edges & Qt.TopEdge))
            root.y = target.mapToItem(parent, 0.0, target.height).y + _padding;

    }
}
