/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

Item
{
    property alias placeholderText: placeholder.text
    property alias text: textArea.text

    property bool __shouldShowPlaceholderText:
        !textArea.text.length && !textArea.activeFocus

    // This only exists to get at the default TextFieldStyle.placeholderTextColor
    // ...maybe there is a better way?
    TextField
    {
        visible: false
        style: TextFieldStyle
        {
            Component.onCompleted: placeholder.textColor = placeholderTextColor
        }
    }

    TextArea
    {
        id: placeholder
        anchors.fill: parent
        visible: __shouldShowPlaceholderText
        activeFocusOnTab: false
    }

    TextArea
    {
        id: textArea
        anchors.fill: parent
        backgroundVisible: !__shouldShowPlaceholderText
    }
}
