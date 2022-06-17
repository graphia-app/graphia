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

Labs.MenuBar
{
    property var _hiddenMenus: []

    property bool visible: true
    onVisibleChanged:
    {
        for(let i = 0; i < menus.length; i++)
        {
            let menu = menus[i];

            if(visible)
            {
                // Showing
                let wasHidden = _hiddenMenus.indexOf(i) >= 0;

                if(menu instanceof PlatformMenu)
                    menu.hidden = wasHidden;
                else if(menu instanceof Labs.Menu)
                    menu.visible = !wasHidden;
            }
            else
            {
                // Hiding
                if(!menu.visible)
                    _hiddenMenus.push(i);

                if(menu instanceof PlatformMenu)
                    menu.hidden = true;
                else if(menu instanceof Labs.Menu)
                    menu.visible = false;
            }
        }

        if(visible)
            _hiddenMenus = [];
    }
}