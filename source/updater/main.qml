import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import com.kajeka 1.0

import "../shared/ui/qml/Constants.js" as Constants

ApplicationWindow
{
    id: root
    visible: true
    flags: Qt.Window|Qt.Dialog

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

            visible: !installer.busy && !installer.complete

            Text
            {
                text: qsTr("<b>A new version of ") + Qt.application.name + qsTr(" is available!</b><br>" +
                    "<br>") +
                    Qt.application.name + qsTr(" version " ) + version +
                    qsTr(" is available. You are currently using version ") + Qt.application.version +
                    qsTr(". Would you like to update now?<br>" +
                    "<br>" +
                    "<b>Release Notes:</b>")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextArea
            {
                id: description

                Layout.fillWidth: true
                Layout.fillHeight: true
                readOnly: true

                textFormat: TextEdit.RichText
                text:
                {
                    var prefix = "file:///" + imagesLocation + "/";
                    var adjustedChangeLog = changeLog;

                    adjustedChangeLog = adjustedChangeLog.replace(/src *= *"/g, "src=\"" + prefix);

                    return adjustedChangeLog;
                }
            }

            RowLayout
            {
                Button
                {
                    text: qsTr("Skip This Version")

                    onClicked:
                    {
                        installer.setStatus("skipped");
                        root.close();
                    }
                }

                Item { Layout.fillWidth: true }

                Button
                {
                    text: qsTr("Remind Me Later")
                    onClicked: { root.close(); }
                }

                Button
                {
                    text: qsTr("Update Now")
                    onClicked: { installer.start(); }
                }
            }
        }
    }

    // Update in progess
    ColumnLayout
    {
        visible: installer.busy && !installer.complete

        anchors.centerIn: parent
        spacing: Constants.spacing * 3

        Text
        {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Updating to Version ") + version + "..."
            font.pointSize: 16
            font.bold: true
        }

        BusyIndicator
        {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 96
            Layout.preferredHeight: 96
        }
    }

    // Update complete
    ColumnLayout
    {
        visible: installer.complete

        anchors.centerIn: parent
        spacing: Constants.spacing * 3

        Text
        {
            Layout.alignment: Qt.AlignHCenter

            text: installer.success ? qsTr("Update Complete!") : qsTr("Update Failed:")
            font.pointSize: 16
            font.bold: true
        }

        TextArea
        {
            visible: !installer.success
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 400
            readOnly: true

            text: installer.error
            font.family: "monospace"
            font.pointSize: 8
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            Button
            {
                visible: !installer.success
                text: qsTr("Retry");

                onClicked: { installer.retry(); }
            }

            Button
            {
                text: qsTr("Open " + Qt.application.name);
                onClicked: { root.close(); }
            }
        }
    }

    Connections
    {
        target: installer
        onCompleteChanged:
        {
            if(installer.complete)
                installer.setStatus(installer.success ? "installed" : "failed");
        }
    }
}
