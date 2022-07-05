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
import Qt.labs.platform as Labs

import app.graphia

// We use a Component here because for whatever reason, the Labs FileDialog only seems
// to allow you to set currentFile once. From looking at the source code it appears as
// if setting currentFile adds to the currently selected files, rather than replaces
// the currently selected files with a new one.
Component
{
    Labs.FileDialog
    {
        id: fileDialog

        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 0

        function updateFileExtension()
        {
            currentFile = QmlUtils.replaceExtension(currentFile, selectedNameFilter.extensions[0]);
        }

        Component.onCompleted: { updateFileExtension(); }

        Connections
        {
            target: fileDialog.selectedNameFilter

            function onIndexChanged()
            {
                if(fileDialog.visible)
                    fileDialog.updateFileExtension();
            }
        }
    }
}
