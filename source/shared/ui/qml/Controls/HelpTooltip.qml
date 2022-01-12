/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

import app.graphia 1.0

import "../"

NamedIcon
{
    id: root
    default property var content
    property string title: ""

    readonly property int _maxWidth: 500
    readonly property int _padding: 20
    readonly property int _offset: 10

    iconName: "help-browser"

    width: 16
    height: 16

    onContentChanged:
    {
        // Parent to the container
        content.parent = containerLayout;

        // Set to fill layout to allow for text wrapping
        content.Layout.fillWidth = true;
    }

    Timer
    {
        id: hoverTimer
        interval: 500
        onTriggered: { tooltip.visible = true; }
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

        width: wrapperLayout.width + _padding
        height: wrapperLayout.height + _padding

        // Magic flags: No shadows, transparent, no focus snatching, no border
        flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup
        color: "#00000000"

        opacity: visible ? 1.0 : 0.0
        Behavior on opacity { PropertyAnimation { duration: 100 } }

        SystemPalette { id: sysPalette }

        Rectangle
        {
            anchors.fill: parent
            color: Qt.rgba(0.96, 0.96, 0.96, 0.96)
            border.width: 1
            border.color: sysPalette.dark
            radius: 3
        }

        ColumnLayout
        {
            id: wrapperLayout
            anchors.centerIn: parent

            Text
            {
                text: root.title
                font.pointSize: FontPointSize.h2
            }

            ColumnLayout
            {
                id: containerLayout
                Layout.maximumWidth: root._maxWidth
            }
        }
    }
}
