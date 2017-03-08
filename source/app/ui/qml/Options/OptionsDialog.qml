import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import "../Constants.js" as Constants

Window
{
    id: optionsWindow

    title: qsTr("Options")
    flags: Qt.Window|Qt.Dialog
    width: 640
    height: 480
    minimumWidth: 640
    minimumHeight: 480

    ColumnLayout
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin

        TabView
        {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Tab
            {
                title: qsTr("Appearance")
                OptionsAppearance {}
            }

            Tab
            {
                title: qsTr("Misc")
                OptionsMisc {}
            }
        }

        Button
        {
            text: qsTr("Close")
            anchors.right: column.right
            onClicked: optionsWindow.close()
        }
    }
}

