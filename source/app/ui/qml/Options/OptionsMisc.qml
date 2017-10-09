import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants

Item
{
    Preferences
    {
        section: "misc"

        property alias focusFoundNodes: focusFoundNodesCheckbox.checked
        property alias focusFoundComponents: focusFoundComponentsCheckbox.checked
        property alias disableHubbles: disableHubblesCheckbox.checked
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

            CheckBox
            {
                id: focusFoundComponentsCheckbox
                text: qsTr("Switch To Component Mode When Searching")
            }

            CheckBox
            {
                id: disableHubblesCheckbox
                text: qsTr("Disable extended help tooltips")
            }
        }
    }
}

