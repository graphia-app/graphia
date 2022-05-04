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
import QtQuick.Controls 2.15

ScrollView
{
    id: root
    clip: true
    padding: frame.background.border.width

    background: Frame
    {
        id: frame

        topPadding: 0
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        // Re-parent the actual meat of the Frame to the ScrollView itself, so it
        // appears above the content; doing this with Frame directly is impossible
        // as Frame is a Control, and as such it consumes all mouse events
        background.z: root.z + 1
        Component.onCompleted: { frame.background.parent = root; }
    }
}
