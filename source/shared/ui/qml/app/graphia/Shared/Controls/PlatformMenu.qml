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

import QtQuick 2.15
import QtQuick.Controls 2.15

Menu
{
    property bool hidden: false

    Component.onCompleted:
    {
        if(parent instanceof MenuBarItem)
            parent.visible = Qt.binding(() => !hidden);

        if(parent instanceof PlatformMenuItem)
        {
            parent.hidden = Qt.binding(() => hidden);
            parent.height = Qt.binding(() => hidden ? 0 : parent.implicitHeight);
        }
    }
}