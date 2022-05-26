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
import QtQml
import QtQuick.Dialogs

import app.graphia.Shared

Item
{
    id: root

    implicitWidth: 64
    implicitHeight: 24

    property string color: "#FF00FF" // Obvious default colour
    property string dialogTitle: qsTr("Select a Colour")

    ColorDialog
    {
        id: colorDialog
        title: root.dialogTitle
        onColorChanged: { root.color = color; }
    }

    Button
    {
        id: button
        width: root.width
        height: root.height

        contentItem: Rectangle
        {
            color: button.enabled ? root.color : Utils.desaturate(root.color, 0.2)
            radius: 2
        }

        onClicked: function(mouse)
        {
            colorDialog.color = root.color;
            colorDialog.open();
        }
    }
}
