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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

Window
{
    id: root

    property string text: ""
    property bool showCopyToClipboard: true

    flags: Constants.defaultWindowFlags
    color: palette.window

    minimumWidth: 500
    minimumHeight: 200

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        ScrollableTextArea
        {
            id: textArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            readOnly: true
            text: root.text
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                visible: root.showCopyToClipboard

                text: qsTr("Copy To Clipboard")
                onClicked: function(mouse) { textArea.copyToClipboard(); }
            }

            Button
            {
                text: qsTr("Close")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }
}
