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

import app.graphia

Rectangle
{
    property ScrollBar horizontalScrollBar
    property ScrollBar verticalScrollBar

    width: verticalScrollBar && verticalScrollBar.size < 1 ? verticalScrollBar.width : 0
    height: horizontalScrollBar && horizontalScrollBar.size < 1 ? horizontalScrollBar.height : 0

    anchors.right: parent.right
    anchors.bottom: parent.bottom

    color:
    {
        if(!horizontalScrollBar || !horizontalScrollBar.background)
            return "transparent";

        if(!verticalScrollBar || !verticalScrollBar.background)
            return "transparent";

        // Special case for dark macOS, loosely matching transparent NSScrollers
        if(Qt.platform.os === "osx" && ControlColors.scheme === Qt.Dark)
            return "#08FFFFFF";

        return ControlColors.midlight;
    }
}
