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
import QtQuick.Layouts

Item
{
    id: root

    property int orientation: Qt.Vertical

    width: orientation === Qt.Vertical ? 8 : 16
    height: orientation === Qt.Vertical ? 16 : 8

    Rectangle
    {
        width: root.orientation === Qt.Vertical ? 1 : root.width
        height: root.orientation === Qt.Vertical ? root.height : 1
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        color: "#22000000"
    }

    Rectangle
    {
        width: root.orientation === Qt.Vertical ? 1 : root.width
        height: root.orientation === Qt.Vertical ? root.height : 1
        anchors.horizontalCenterOffset: root.orientation === Qt.Vertical ? 1 : 0
        anchors.verticalCenterOffset: root.orientation === Qt.Vertical ? 0 : 1
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        color: "#33ffffff"
    }
}
