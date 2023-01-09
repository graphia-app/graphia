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
import QtQuick.Layouts

RowLayout
{
    id: root

    property var document
    property string name
    property string description
    property double value: -1.0

    onValueChanged:
    {
        slider.value = root.value;
    }

    Label
    {
        id: label
        text: root.name

        MouseArea
        {
            anchors.fill: parent
            onDoubleClicked: function(mouse) { root.reset(); }
        }

        ToolTip { text: root.description }
    }

    Item { Layout.fillWidth: true }

    Slider
    {
        id: slider
        Layout.preferredWidth: 170

        from: 0.0
        to: 1.0

        onValueChanged:
        {
            if(pressed)
                root.value = slider.value;
        }
    }

    signal reset();
}
