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
import QtQml
import QtQuick.Dialogs

import Qt.labs.platform as Labs

import app.graphia
import app.graphia.Shared

Item
{
    id: root

    implicitWidth: 64
    implicitHeight: 24

    property string color: "#FF00FF" // Obvious default colour
    property string dialogTitle: qsTr("Select a Colour")

    Labs.ColorDialog
    {
        id: colorDialog
        title: root.dialogTitle
        modality: Qt.ApplicationModal

        onColorChanged: { root.color = color; }
    }

    Rectangle
    {
        id: button
        width: root.width
        height: root.height

        border.width: 1
        border.color: mouseArea.containsMouse ? "dimgrey" : "black";
        radius: 2

        color:
        {
            if(!root.enabled)
                return Utils.desaturate(root.color, 0.2);

            if(mouseArea.containsMouse)
                return Qt.lighter(root.color, 1.35);

            return root.color;
        }

        MouseArea
        {
            id: mouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: function(mouse)
            {
                colorDialog.color = root.color;
                colorDialog.open();
            }
        }
    }
}
