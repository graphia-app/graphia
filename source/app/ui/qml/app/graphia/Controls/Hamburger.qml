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

import QtQuick
import QtQuick.Controls

Item
{
    property real radius: height * 0.1
    property color color: "white"
    property color hoverColor: color
    property bool propogatePresses: false

    property color _displayColor: (mouseArea.containsMouse || menuDropped) ? hoverColor : color

    property real _barToSpaceRatio: 1.0
    property real _d: 2.0 + 3.0 * _barToSpaceRatio
    property real _barHeight: (_barToSpaceRatio * height) / _d
    property real _spaceHeight: height / _d

    property Menu menu: null

    property bool menuDropped: false
    Connections
    {
        target: menu

        function onAboutToShow() { menuDropped = true; }
        function onAboutToHide() { menuDropped = false; }
    }

    Rectangle
    {
        x: 0
        y: 0
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    Rectangle
    {
        x: 0
        y: _barHeight + _spaceHeight
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    Rectangle
    {
        x: 0
        y: parent.height - _barHeight
        width: parent.width
        height: _barHeight
        radius: parent.radius
        color: parent._displayColor
    }

    MouseArea
    {
        id: mouseArea

        hoverEnabled: true
        anchors.fill: parent
        onClicked: function(mouse)
        {
            if(menu)
                menu.popup(0, parent.height + 4/*padding*/);
        }

        onPressed: function(mouse) { mouse.accepted = !propogatePresses; }
    }
}
