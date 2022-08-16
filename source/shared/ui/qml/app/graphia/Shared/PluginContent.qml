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

Item
{
    property int minimumHeight: 100
    property var toolStrip: null

    property string baseFileName: ""
    property string baseFileNameNoExtension: ""

    // Don't use this directly, that is naughty
    property var _mainWindow: null

    property bool directed: _mainWindow ? _mainWindow.directed : false

    function updateMenu() { _mainWindow.updatePluginMenus(); }

    function attributeIsEditable(attributeName) { return _mainWindow.attributeIsEditable(attributeName); }
    function cloneAttribute(attributeName) { _mainWindow.cloneAttribute(attributeName); }
    function editAttribute(attributeName) { _mainWindow.editAttribute(attributeName); }
    function removeAttributes(attributeNames) { _mainWindow.removeAttributes(attributeNames); }

    function writeTableModelToFile(model, fileName, type, columnNames)
    {
        _mainWindow.writeTableModelToFile(model, fileName, type, columnNames);
    }

    function copyTableViewColumnToClipboard(tableView, columnName)
    {
        _mainWindow.copyTableViewColumnToClipboard(tableView, columnName);
    }
}
