import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import QtWebView 1.1

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

                text: application.name + qsTr(" version ") + application.version +
                    qsTr(" is a tool for the visualisation and analysis of graphs.<br><br>") +
                    application.copyright + qsTr("<br><br>") +
                    qsTr("<a href=\"EULA\">End User License</a>") +
                        "&nbsp;&nbsp;&nbsp;" +
                    qsTr("<a href=\"OSS\">Open Source Licenses</a>")

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

