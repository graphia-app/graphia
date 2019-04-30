import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import "../../../shared/ui/qml/Constants.js" as Constants

import "Controls"

Window
{
    id: root

    property string text: ""

    flags: Qt.Window|Qt.Dialog

    minimumWidth: 500
    minimumHeight: 200

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        TextArea
        {
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
                text: qsTr("Close")
                onClicked: { root.close(); }
            }
        }
    }
}
