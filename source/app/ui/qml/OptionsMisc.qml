import QtQuick 2.5
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import com.kajeka 1.0

import "Constants.js" as Constants

Item
{
    Preferences
    {
        section: "misc"

        property alias focusFoundNodes: focusFoundNodesCheckbox.checked
    }

    Column
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        ColumnLayout
        {
            Label
            {
                font.bold: true
                text: qsTr("Find")
            }

            CheckBox
            {
                id: focusFoundNodesCheckbox
                text: qsTr("Focus Found Nodes")
            }
        }
    }
}

