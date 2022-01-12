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

pragma Singleton
import QtQuick 2.4

QtObject
{
    property var _defaultTextObj:
    {
        return  Qt.createQmlObject('import QtQuick 2.0; Text { text: "unused" }', this);
    }

    property real p: _defaultTextObj.font.pointSize;
    property real h2: _defaultTextObj.font.pointSize * 1.5;
    property real h1: _defaultTextObj.font.pointSize * 2.0;
}
