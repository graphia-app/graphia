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
import QtQuick.Controls

import Graphia.Controls

Rectangle
{
    id: root

    property alias readOnly: textArea.readOnly
    property alias text: textArea.text
    property alias wrapMode: textArea.wrapMode
    property alias textFormat: textArea.textFormat
    property alias hoveredLink: textArea.hoveredLink
    property alias placeholderText: textArea.placeholderText
    property alias cursorPosition: textArea.cursorPosition
    property alias length: textArea.length
    property alias font: textArea.font
    property alias outlineVisible: outline.outlineVisible

    color: ControlColors.background

    ScrollView
    {
        anchors.fill: parent
        anchors.margins: outline.outlineWidth
        contentWidth: availableWidth

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        Component.onCompleted:
        {
            // Make the scrolling behaviour more desktop-y
            contentItem.boundsBehavior = Flickable.StopAtBounds;
            contentItem.flickableDirection = Flickable.VerticalFlick;
        }

        TextArea
        {
            id: textArea

            wrapMode: TextArea.Wrap
            selectByMouse: true
            color: palette.windowText

            onEditingFinished: { root.editingFinished(); }
            onLinkActivated: function(link) { root.linkActivated(link); }
            onLinkHovered: function(link) { root.linkHovered(link); }
        }
    }

    Outline
    {
        id: outline
        anchors.fill: parent
    }

    function append(text)           { textArea.append(text); }
    function insert(position, text) { textArea.insert(position, text); }
    function remove(start, end)     { return textArea.remove(start, end); }

    function copyToClipboard()
    {
        textArea.selectAll();
        textArea.copy();
        textArea.deselect();
        textArea.cursorPosition = 0;
    }

    signal editingFinished()
    signal linkActivated(string link)
    signal linkHovered(string link)
}
