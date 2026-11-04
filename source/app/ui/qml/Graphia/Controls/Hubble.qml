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

    default property Item content
    readonly property int _padding: 10
    property alias color: backRectangle.color

    property string title: ""
    property Item target
    property ApplicationWindow mainWindow: null

    property int alignment: Qt.AlignLeft | Qt.AlignTop
    property int edges: Qt.LeftEdge | Qt.TopEdge

    property bool displayPrevious: false
    property bool displayNext: false
    property bool displayClose: false
    property bool tooltipMode: false

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
    onTargetChanged: { linkToTarget(); }

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
        onTriggered: { root.visible = true; }
    }

    HoverHandler
    {
        parent: root.tooltipMode && root.target ? root.target : root
        enabled: parent !== root

        onHoveredChanged:
        {
            if(hovered)
            {
                hoverTimer.start();
                root.parent = root.mainWindow.header;
                root.positionBubble();
            }
            else
            {
                hoverTimer.stop();
                root.visible = false;
            }
        }
    }

    Rectangle
    {
        id: backRectangle
        color: Qt.rgba(0.96, 0.96, 0.96, 0.9)
        width: mainLayout.width + root._padding
        height: mainLayout.height + root._padding
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
                text: root.title
                font.pointSize: FontPointSize.h2
            }
        }

        RowLayout
        {
            Layout.preferredWidth: containerLayout.width

            visible: root.displayPrevious || root.displayNext || root.displayClose

            Text
            {
                visible: !root.displayClose
                text: qsTr("Skip")
                font.underline: true
                MouseArea
                {
                    cursorShape: Qt.PointingHandCursor
                    anchors.fill: parent
                    onClicked: (mouse) => { root.skipClicked(); }
                }
            }

            Item { Layout.fillWidth: true }

            Button
            {
                visible: root.displayPrevious
                text: qsTr("Previous")
                onClicked: { root.previousClicked(); }
            }

            Button
            {
                visible: root.displayNext
                text: qsTr("Next")
                onClicked: { root.nextClicked(); }
            }

            Button
            {
                visible: root.displayClose
                text: qsTr("Close")
                onClicked: { root.closeClicked(); }
            }
        }
    }

    Component.onCompleted:
    {
        linkToTarget();
    }

    function linkToTarget()
    {
        if(target !== null && !tooltipMode)
        {
            root.parent = root.mainWindow.header;
            positionBubble();
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
