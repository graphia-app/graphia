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

import Qt.labs.platform as Labs

Labs.MenuItem
{
    id: menuItem

    property bool hidden: false
    visible: !hidden

    property Action action

    onActionChanged:
    {
        if(!action)
        {
            menuItem.checkable = false;
            menuItem.checked = false;
            menuItem.enabled = true;
            menuItem.icon.name = "";
            menuItem.icon.source = "";
            menuItem.shortcut = "";
            menuItem.text = "";
            return;
        }

        menuItem.checkable = Qt.binding(() => action.checkable);
        menuItem.checked = Qt.binding(() => action.checked);
        menuItem.enabled = Qt.binding(() => action.enabled);
        menuItem.icon.name = Qt.binding(() => action.icon.name);
        menuItem.icon.source = Qt.binding(() => action.icon.source);
        menuItem.shortcut = Qt.binding(() => action.shortcut);
        menuItem.text = Qt.binding(() => action.text);

        menuItem.triggered.connect(function() { action.trigger(menuItem); });
    }
}
