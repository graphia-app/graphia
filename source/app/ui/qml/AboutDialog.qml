/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import "../../../shared/ui/qml/Constants.js" as Constants
import "../../../shared/ui/qml/Utils.js" as Utils

import "Controls"

Window
{
    id: root

    property var application: null

    title: qsTr("About ") + application.name
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

                text: application.name +
                    qsTr(" is a tool for the visualisation and analysis of graphs.<br><br>") +
                    qsTr("Version ") + application.version + qsTr(".<br>") +
                    application.copyright + qsTr("<br><br>") +
                    qsTr("<a href=\"LICENSE\">License</a>") +
                        "&nbsp;&nbsp;&nbsp;" +
                    qsTr("<a href=\"OSS\">Third Party Licenses</a>")

                PointingCursorOnHoverLink {}
                onLinkActivated:
                {
                    licenseTextArea.text = Utils.readFile("qrc:///licensing/" + link + ".html");

                    if(root.width < 800)
                        root.width = 800;

                    if(root.height < 500)
                        root.height = 500;

                    licenseTextArea.visible = true;
                }
            }

            TextArea
            {
                id: licenseTextArea
                visible: false
                readOnly: true

                textFormat: Text.RichText

                Layout.fillWidth: true
                Layout.fillHeight: true

                PointingCursorOnHoverLink {}
                onLinkActivated: Qt.openUrlExternally(link);
            }

            Button
            {
                text: qsTr("Close")
                Layout.alignment: Qt.AlignRight
                onClicked: { root.close(); }
            }
        }
    }

    signal hiddenSwitchActivated()
}

