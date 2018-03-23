import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
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
        property alias webSearchEngineUrl: webSearchEngineField.text
    }

    ColumnLayout
    {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.margins: Constants.margin
        spacing: Constants.spacing

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
            text: qsTr("Switch To Component Mode When Finding")
        }

        Label
        {
            font.bold: true
            text: qsTr("Help")
        }

        CheckBox
        {
            id: disableHubblesCheckbox
            text: qsTr("Disable Extended Help Tooltips")
        }

        Label
        {
            font.bold: true
            text: qsTr("Web Search Engine")
        }

        TextField
        {
            id: webSearchEngineField

            implicitWidth: 320

            style: TextFieldStyle
            {
                textColor: QmlUtils.urlIsValid(webSearchEngineField.text) ? "black" : "red"
            }
        }
    }
}

