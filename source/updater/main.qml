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
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import app.graphia.Shared
import app.graphia.Shared.Controls

ApplicationWindow
{
    id: root
    visible: true
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    title: Qt.application.name + " " + qsTr("Update")

    width: 640
    height: 480
    minimumWidth: 640
    minimumHeight: 480

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        Image
        {
            Layout.alignment: Qt.AlignTop

            source: "icon.svg"
            sourceSize.width: 96
            sourceSize.height: 96
        }

        // Update options
        ColumnLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: installer !== null && !installer.busy && !installer.complete

            Text
            {
                text: Utils.format(qsTr("<b>A new version of {0} is available!</b><br>" +
                    "<br>" +
                    "{0} version {1} is available. You are currently using version {2}. " +
                    "Would you like to update now?<br>" +
                    "<br>" +
                    "<b>Release Notes:</b>"), Qt.application.name, version, Qt.application.version)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            ScrollableTextArea
            {
                id: description

                Layout.fillWidth: true
                Layout.fillHeight: true
                readOnly: true

                textFormat: TextEdit.MarkdownText
                text:
                {
                    let prefix = "file:///" + imagesLocation + "/";
                    let adjustedChangeLog = changeLog;

                    adjustedChangeLog = adjustedChangeLog.replace(
                        /(?:!\[(.*?)\]\(file:(.*?)\))/g,
                        "![$1](" + prefix + "$2)");

                    return adjustedChangeLog;
                }

                onLinkActivated: function(link) { Qt.openUrlExternally(link); }
            }

            RowLayout
            {
                Button
                {
                    text: qsTr("Skip This Version")

                    onClicked: function(mouse)
                    {
                        installer.setStatus("skipped");
                        root.close();
                    }
                }

                Item { Layout.fillWidth: true }

                Button
                {
                    text: qsTr("Remind Me Later")
                    onClicked: function(mouse) { root.close(); }
                }

                Button
                {
                    text: qsTr("Update Now")
                    onClicked: function(mouse) { installer.start(); }
                }
            }
        }
    }

    // Update in progess
    ColumnLayout
    {
        visible: installer !== null && installer.busy && !installer.complete

        anchors.centerIn: parent
        spacing: Constants.spacing * 3

        Text
        {
            Layout.alignment: Qt.AlignHCenter
            text: Utils.format(qsTr("Updating to Version {0}…"), version)
            font.pointSize: 16
            font.bold: true
        }

        BusyIndicator
        {
            visible: parent.visible
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 96
            Layout.preferredHeight: 96
        }
    }

    // Update complete
    ColumnLayout
    {
        visible: installer !== null && installer.complete

        anchors.centerIn: parent
        spacing: Constants.spacing * 3

        Text
        {
            Layout.alignment: Qt.AlignHCenter

            text: (installer !== null && installer.success) ?
                qsTr("Update Complete!") : qsTr("Update Failed:")
            font.pointSize: 16
            font.bold: true
        }

        TextArea
        {
            visible: installer !== null && !installer.success
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 400
            readOnly: true
            wrapMode: TextArea.Wrap
            selectByMouse: true

            text: installer !== null && installer.error
            font.family: "monospace"
            font.pointSize: 8
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            Button
            {
                visible: installer !== null && !installer.success
                text: qsTr("Retry");

                onClicked: function(mouse) { installer.retry(); }
            }

            Button
            {
                text: qsTr("Open " + Qt.application.name);
                onClicked: function(mouse) { root.close(); }
            }
        }
    }

    Connections
    {
        target: installer
        function onCompleteChanged()
        {
            if(installer.complete)
                installer.setStatus(installer.success ? "installed" : "failed");
        }
    }
}
