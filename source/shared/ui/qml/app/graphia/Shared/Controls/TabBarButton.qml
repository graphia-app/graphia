/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

TabButton
{
    id: root

    property bool showCloseButton: false

    width: implicitWidth
    height: implicitHeight

    leftPadding: 8
    rightPadding: showCloseButton ? 4 : 8
    topPadding: 4
    bottomPadding: 4

    contentItem: Row
    {
        spacing: 4

        Label
        {
            anchors.verticalCenter: parent.verticalCenter

            text: root.text
            font: root.font
            color: root.palette.buttonText
        }

        FloatingButton
        {
            visible: root.showCloseButton

            anchors.verticalCenter: parent.verticalCenter
            implicitHeight: 24

            icon.name: "emblem-unreadable"

            onClicked: function(mouse) { root.closeButtonClicked(mouse); }
        }
    }

    signal closeButtonClicked(var mouse)
}
