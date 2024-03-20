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

import app.graphia
import app.graphia.Shared

// This is basically a substitute for ToolButton,
// that looks consistent across platforms
AbstractButton
{
    id: root

    property double hoverOpacity: 1.0
    property bool scaleIconToFill: false
    property alias selected: namedIcon.selected

    implicitHeight: 32
    padding: 4

    background: Rectangle
    {
        visible: root.hovered || root.checked
        border.width: 1
        border.color: "#ababab"
        radius: 2
        gradient: Gradient
        {
            GradientStop { position: 0; color: root.pressed || root.checked ?
                "#dcdcdc" : "#fefefe" }
            GradientStop { position: 1; color: root.pressed || root.checked ?
                "#dcdcdc" : "#f8f8f8" }
        }
    }

    contentItem: RowLayout
    {
        spacing: 4

        property string _iconName: root.action !== null && root.action.icon !== null ?
            root.action.icon.name : root.icon.name
        property string _text: root.action !== null ?
            root.action.text : root.text
        property string _cleansedText: { return _text.replace("&", ""); }

        NamedIcon
        {
            id: namedIcon

            opacity: root.hovered ? 1.0 : root.hoverOpacity

            Layout.alignment: Qt.AlignVCenter|Qt.AlignHCenter
            visible: valid
            Layout.preferredWidth: height
            Layout.preferredHeight: root.height - (root.topPadding + root.bottomPadding)
            iconName: parent._iconName
            fill: root.scaleIconToFill
        }

        Text
        {
            Layout.alignment: Qt.AlignVCenter|Qt.AlignHCenter
            visible: !namedIcon.valid && parent._cleansedText.length > 0
            text: parent._cleansedText
        }

        Item
        {
            // Empty placeholder that's shown if there is no
            // valid icon or text available
            visible: !namedIcon.valid && parent._cleansedText.length === 0
            Layout.preferredWidth: namedIcon.width
            Layout.preferredHeight: namedIcon.height
        }

        ToolTip.visible: namedIcon.valid && _cleansedText.length > 0 && hovered
        ToolTip.delay: Constants.toolTipDelay
        ToolTip.text: namedIcon.valid ? _cleansedText : ""
    }
}
