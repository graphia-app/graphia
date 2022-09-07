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

Menu
{
    property bool hidden: false

    onParentChanged:
    {
        if(!parent)
            return;

        if(parent instanceof MenuBarItem)
            parent.visible = Qt.binding(() => !hidden);
        else if(parent instanceof MenuItem) // This happens when .addMenu has been used
        {
            parent.clip = true;
            parent.height = Qt.binding(() => hidden || !parent ? 0 : parent.implicitHeight);
        }
        else if(parent instanceof PlatformMenuItem)
        {
            parent.hidden = Qt.binding(() => hidden);
            parent.height = Qt.binding(() => hidden || !parent ? 0 : parent.implicitHeight);
        }
    }

    implicitWidth:
    {
        const maxWidth = 600;
        let maxChildWidth = 200;

        let menus = contentChildren[0];
        for(let i = 0; i < menus.children.length; i++)
        {
            let menu = menus.children[i];
            if(!(menu instanceof MenuItem) || !(menu instanceof PlatformMenuItem))
                continue;

            if(menu.hidden)
                continue;

            maxChildWidth = Math.max(maxChildWidth, menu.implicitWidth);
        }

        return Math.min(maxWidth, maxChildWidth) + (this.padding * 2);
    }
}
