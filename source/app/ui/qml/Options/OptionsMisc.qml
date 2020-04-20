/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import app.graphia 1.0

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

    Preferences
    {
        id: tracking
        section: "tracking"
        property string emailAddress
        property string permission
    }

    Component.onCompleted:
    {
        if(tracking.permission === "refused")
        {
            noTrackingRadioButton.checked = true;
            emailField.text = "";
        }
        else if(tracking.permission === "given")
        {
            yesTrackingRadioButton.checked = true;
            emailField.text = tracking.emailAddress;
        }
        else
        {
            anonTrackingRadioButton.checked = true;
            emailField.text = "";
        }
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
            text: qsTr("Allow Tracking")
        }

        ExclusiveGroup { id: trackingGroup }
        RowLayout
        {
            RadioButton
            {
                id: yesTrackingRadioButton
                exclusiveGroup: trackingGroup
                text: qsTr("Yes")

                onCheckedChanged:
                {
                    if(checked)
                        tracking.permission = "given";
                }
            }

            TextField
            {
                id: emailField

                Layout.preferredWidth: 250

                enabled: yesTrackingRadioButton.checked

                placeholderText: qsTr("Email Address")
                validator: RegExpValidator
                {
                    // Check it's a valid email address
                    regExp: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/
                }

                onTextChanged:
                {
                    if(acceptableInput)
                        tracking.emailAddress = text;
                }
            }
        }

        RadioButton
        {
            id: anonTrackingRadioButton
            exclusiveGroup: trackingGroup
            text: qsTr("Yes, Anonymously")

            onCheckedChanged:
            {
                if(checked)
                    tracking.permission = "anonymous";
            }
        }

        RadioButton
        {
            id: noTrackingRadioButton
            exclusiveGroup: trackingGroup
            text: qsTr("No")

            onCheckedChanged:
            {
                if(checked)
                    tracking.permission = "refused";
            }
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

                property string _defaultValue: "https://www.google.com/#q=%1"
                function reset() { text = _defaultValue; }

                Component.onCompleted:
                {
                    if(text.length === 0)
                        text = _defaultValue;
                }
            }

            FloatingButton
            {
                iconName: "view-refresh"
                onClicked: { webSearchEngineField.reset(); }
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

