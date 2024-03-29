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

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia
import app.graphia.Shared
import app.graphia.Shared.Controls

Window
{
    id: root
    title: "Commands"

    width: 800
    height: 640
    minimumWidth: 800
    minimumHeight: 640

    Settings
    {
        id: settings
        category: "updateEditor"

        property string operatingSystems: ""
    }

    property var operatingSystems: []

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        Repeater
        {
            id: osControls

            model: root.operatingSystems

            delegate: ColumnLayout
            {
                Layout.fillWidth: true

                property alias name: osLabel.text
                property alias command: commandTextArea.text

                Label
                {
                    id: osLabel

                    font.bold: true
                    font.pointSize: 12

                    text: modelData.name
                }

                ScrollableTextArea
                {
                    id: commandTextArea

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    wrapMode: TextArea.Wrap

                    font.family: "monospace"
                    font.pointSize: 9

                    text: modelData.command
                }
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                onClicked: function(mouse)
                {
                    let osCommands = [];

                    for(let i = 0; i < osControls.count; i++)
                    {
                        let item = osControls.itemAt(i);

                        osCommands.push({"name": item.name, "command": item.command});
                    }

                    root.close();

                    root.operatingSystems = osCommands;
                    root.accepted();
                }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }

    signal accepted()
}
