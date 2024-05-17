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

import Graphia.Utils

Item
{
    id: root

    WasmLocalFileAccess { id: wasmLocalFileAccess }

    // Dummy properties to satisfy user expectations
    property string title
    property var nameFilters: []
    property url currentFolder
    property var selectedNameFilter: Item
    {
        property int index: 0
        property var extensions: root.defaultExtensions
    }

    property var defaultExtensions:
    {
        if(root.nameFilters.length === 0)
            return [];

        const defaultNameFilter = root.nameFilters[0];
        const regex = /\*\.(\w+)/g;
        let matches;
        let extensions = [];

        while((matches = regex.exec(defaultNameFilter)) !== null)
            extensions.push(matches[1]);

        return extensions;
    }

    property url selectedFile

    function open()
    {
        const extension = root.defaultExtensions.length > 0 ? "." + root.defaultExtensions[0] : "";
        const filenameHint = NativeUtils.baseFileNameForUrl(root.selectedFile) + extension;

        // WasmLocalFileAccess::save returns a temporary file name in which it
        // expects the data to be written, thereafter waiting for the file to be
        // closed at which point it shows the web browser's save dialog in order
        // to choose where the file will be downloaded to the user's local FS
        root.selectedFile = wasmLocalFileAccess.save(filenameHint);

        // Unconditionally accepting should cause the file to be saved to the
        // temporary file that selectedFile now refers to
        root.accepted();
    }

    signal accepted();
    signal rejected();
}
