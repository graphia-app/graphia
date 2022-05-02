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
import QtQuick.Controls 2.12

Frame
{
    id: root

    property alias readOnly: textArea.readOnly
    property alias text: textArea.text
    property alias wrapMode: textArea.wrapMode
    property alias textFormat: textArea.textFormat
    property alias hoveredLink: textArea.hoveredLink
    property alias placeholderText: textArea.placeholderText

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    Component.onCompleted: { background.color = "white"; }

    Flickable
    {
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        TextArea.flickable: TextArea
        {
            id: textArea
            wrapMode: TextArea.Wrap
            selectByMouse: true

            onEditingFinished: { root.editingFinished(); }
            onLinkActivated: { root.linkActivated(link); }
            onLinkHovered: { root.linkHovered(link); }
        }

        ScrollBar.vertical: ScrollBar {}
    }

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
