import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import QtWebView 1.1

import "../../../shared/ui/qml/Constants.js" as Constants
import "../../../shared/ui/qml/Utils.js" as Utils

Window
{
    id: root

    property var application: null

    title: qsTr("About ") + application.name
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 550
    minimumHeight: licenseTextArea.visible ? 400 : 200

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        Image
        {
            Layout.alignment: Qt.AlignTop
            source: "qrc:///icon/Icon128x128.png"
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

                onLinkActivated:
                {
                    licenseTextArea.text = Utils.readFile("qrc:///licensing/" + link + ".html");
                    licenseTextArea.visible = true;
                }
            }

            TextArea
            {
                id: licenseTextArea

                textFormat: Text.RichText

                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button
            {
                text: qsTr("Close")
                Layout.alignment: Qt.AlignRight
                onClicked: { root.close(); }
            }
        }
    }

    onVisibleChanged:
    {
        if(visible)
        {
            licenseTextArea.visible = false;
            root.height = root.minimumHeight;
        }
    }
}

