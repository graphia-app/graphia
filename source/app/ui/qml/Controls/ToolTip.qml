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
import QtQuick.Controls.Private 1.0

// This is a gigantic hack that delves into QtQuick.Controls.Private to
// provide the ability to add passive tooltips to things that don't otherwise
// have them (like Images). Note that this prevents the underlying Item's hover
// functionality from working, so may break things in exciting ways.
Item
{
    id: root
    anchors.fill: item

    property Item item: parent
    property string text

    MouseArea
    {
        id: mouseArea

        anchors.fill: parent

        // Pass any clicks through
        propagateComposedEvents: true
        onPressed: { mouse.accepted = false; }

        hoverEnabled: true

        onExited: { Tooltip.hideText(); }
        onCanceled: { Tooltip.hideText(); }
    }

    Timer
    {
        interval: 1000
        running: mouseArea.containsMouse && !mouseArea.pressed &&
            root.text.length > 0

        onTriggered:
        {
            Tooltip.showText(root, Qt.point(mouseArea.mouseX,
                mouseArea.mouseY), root.text);
        }
    }
}
