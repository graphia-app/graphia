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
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import app.graphia 1.0

import ".."

// This is basically a substitute for ToolButton,
// that looks consistent across platforms
Button
{
    id: root

    property string iconName
    property double hoverOpacity: 1.0

    implicitHeight: 32

    style: ButtonStyle
    {
        background: Rectangle
        {
            visible: control.hovered || control.checked
            border.width: 1
            border.color: "#ababab"
            radius: 2
            gradient: Gradient
            {
                GradientStop { position: 0; color: control.pressed || control.checked ?
                    "#dcdcdc" : "#fefefe" }
                GradientStop { position: 1; color: control.pressed || control.checked ?
                    "#dcdcdc" : "#f8f8f8" }
            }
        }

        label: RowLayout
        {
            spacing: 4
            anchors.fill: parent

            property string _iconName: control.action !== null ?
                control.action.iconName : root.iconName
            property string _text: control.action !== null ?
                control.action.text : root.text

            NamedIcon
            {
                id: icon

                opacity: control.hovered ? 1.0 : root.hoverOpacity

                Layout.alignment: Qt.AlignVCenter
                visible: valid
                Layout.preferredWidth: height
                Layout.preferredHeight: root.height - (padding.top + padding.bottom)
                iconName: _iconName
            }

            Text
            {
                Layout.alignment: Qt.AlignVCenter
                visible: !icon.valid && _text.length > 0
                text: _text
            }

            Item
            {
                // Empty placeholder that's shown if there is no
                // valid icon or text available
                visible: !icon.valid && _text.length === 0
                Layout.preferredWidth: icon.width
                Layout.preferredHeight: icon.height
            }
        }
    }
}
