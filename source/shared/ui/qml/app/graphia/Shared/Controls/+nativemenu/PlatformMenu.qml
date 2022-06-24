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

import QtQuick
import QtQuick.Controls

import Qt.labs.platform as Labs

Labs.Menu
{
    id: menu

    property bool hidden: false
    visible: !hidden

    readonly property int count: items.length

    function itemAt(i)
    {
        if(i < 0 || i >= items.length)
        {
            console.log("PlatformMenu::itemAt: index out of range");
            return null;
        }

        return items[i];
    }

    function takeItem(i)
    {
        let item = itemAt(i);
        if(item !== null)
            removeItem(item);
    }

    Component { id: popupPositionComponent; Item {} }

    function popup(parent, x, y)
    {
        if(parent !== undefined && x !== undefined && y !== undefined)
        {
            let item = popupPositionComponent.createObject(parent, {x: x, y: y});
            open(item);
            item.destroy();
        }
        else
            open();
    }
}
