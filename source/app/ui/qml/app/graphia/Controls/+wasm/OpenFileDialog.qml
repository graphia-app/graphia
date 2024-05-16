/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import app.graphia.Utils

Item
{
    id: root

    WasmLocalFileAccess
    {
        id: wasmLocalFileAccess

        onAccepted: { root.accepted(); }
        onRejected: { root.rejected(); }
    }

    // Dummy properties to satisfy user expectations
    property string title
    property var nameFilters: []
    property url currentFolder
    property var selectedNameFilter: Item { property int index }

    property url selectedFile: wasmLocalFileAccess.fileUrl

    function open() { wasmLocalFileAccess.open(root.nameFilters); }

    signal accepted();
    signal rejected();
}
