/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

Window
{
    id: root

    property var application: null

    title: Utils.format(qsTr("About {0}"), application.name)
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 500
    minimumHeight: 200

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        Image
        {
            Layout.alignment: Qt.AlignTop
            source: "qrc:///icon/Icon128x128.png"

            HiddenSwitch { onActivated: hiddenSwitchActivated(); }
        }

        ColumnLayout
        {
            Text
            {
                Layout.fillWidth: true
                Layout.fillHeight: !licenseTextArea.visible

                wrapMode: Text.WordWrap
                textFormat: Text.StyledText

                text: Utils.format(
                    qsTr("{0} is a tool for the visualisation and analysis of graphs.<br><br>" +
                    "Version {1}.<br>{2}<br><br>" +
                    "<a href=\"LICENSE\">License</a>&nbsp;&nbsp;&nbsp;<a href=\"OSS\">Third Party Licenses</a>"),
                    application.name, application.version, application.copyright)

                PointingCursorOnHoverLink {}
                onLinkActivated: function(link)
                {
                    licenseTextArea.text = QmlUtils.readFromFile(":/licensing/" + link + ".html");

                    if(root.width < 800)
                        root.width = 800;

                    if(root.height < 500)
                        root.height = 500;

                    licenseTextArea.visible = true;
                }
            }

            ScrollableTextArea
            {
                id: licenseTextArea

                Layout.fillWidth: true
                Layout.fillHeight: true

                visible: false
                readOnly: true

                textFormat: TextEdit.RichText
                wrapMode: TextEdit.Wrap

                PointingCursorOnHoverLink {}
                onLinkActivated: function(link) { Qt.openUrlExternally(link); }
            }

            Button
            {
                text: qsTr("Close")
                Layout.alignment: Qt.AlignRight
                onClicked: function(mouse) { root.close(); }
            }
        }
    }

    signal hiddenSwitchActivated()
}

