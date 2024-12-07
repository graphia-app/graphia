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

// If this item is inserted as the child of something with a hoveredLink property,
// it will change the cursor to a pointing finger, whenever said property is non-empty
MouseArea
{
    id: root
    anchors.fill: parent

    property var _parentWithHoveredLinkProperty: null

    cursorShape:
    {
        return root._parentWithHoveredLinkProperty &&
            root._parentWithHoveredLinkProperty.hoveredLink.length > 0 ?
            Qt.PointingHandCursor : Qt.ArrowCursor;
    }

    onParentChanged:
    {
        let p = parent;

        while(p)
        {
            if(p.hoveredLink !== undefined)
            {
                root._parentWithHoveredLinkProperty = p;
                return;
            }

            p = p.parent;
        }

        console.log("PointingCursorOnHoverLink: Could not find parent with 'hoveredLink' property");
    }

    // Ignore and pass through any events
    propagateComposedEvents: true
    onPressed: function(mouse) { mouse.accepted = false; }
}
