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
    property alias contentChildren: scrollView.contentChildren
    default property alias contentData: scrollView.contentData

    color: ControlColors.background

    ScrollView
    {
        id: scrollView

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        Component.onCompleted:
        {
            // Make the scrolling behaviour more desktop-y
            contentItem.boundsBehavior = Flickable.StopAtBounds;
            contentItem.flickableDirection = Flickable.VerticalFlick;
        }

        clip: true
        padding: outline.outlineWidth
        anchors.fill: parent
    }

    readonly property real size: scrollView.ScrollBar.vertical.size

    readonly property real position: scrollView.ScrollBar.vertical.position
    function setPosition(p) { scrollView.ScrollBar.vertical.position = p; }

    readonly property real staticScrollBarWidth: scrollView.ScrollBar.vertical.width
    readonly property real scrollBarWidth:
        scrollView.ScrollBar.vertical.size < 1 ? scrollView.ScrollBar.vertical.width : 0

    Outline
    {
        id: outline
        anchors.fill: parent
    }

    function resetScrollPosition()
    {
        scrollView.ScrollBar.vertical.position = 0;
    }
}
