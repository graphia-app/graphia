import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import "../Controls"
import "../../../../shared/ui/qml/Constants.js" as Constants

Item
{
    Preferences
    {
        id: misc
        section: "misc"

        property alias focusFoundNodes: focusFoundNodesCheckbox.checked
        property alias focusFoundComponents: focusFoundComponentsCheckbox.checked
        property alias disableHubbles: disableHubblesCheckbox.checked
        property alias webSearchEngineUrl: webSearchEngineField.text
        property alias maxUndoLevels: maxUndoSpinBox.value
        property alias autoBackgroundUpdateCheck: autoBackgroundUpdateCheckCheckbox.checked
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
            text: qsTr("Other")
        }

        RowLayout
        {
            Label { text: qsTr("Web Search URL:") }

            TextField
            {
                id: webSearchEngineField

                implicitWidth: 320

                style: TextFieldStyle
                {
                    textColor: QmlUtils.urlIsValid(webSearchEngineField.text) ? "black" : "red"
                }
            }

            FloatingButton
            {
                iconName: "view-refresh"
                onClicked: { misc.reset("webSearchEngineUrl"); }
            }
        }

        RowLayout
        {
            Label { text: qsTr("Maximum Undo Levels:") }

            SpinBox
            {
                id: maxUndoSpinBox
                minimumValue: 0
                maximumValue: 50
            }
        }

        CheckBox
        {
            id: autoBackgroundUpdateCheckCheckbox
            text: qsTr("Check For Updates Automatically")
        }
    }
}

