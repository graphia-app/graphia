/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

        property alias webSearchEngineUrl: webSearchEngineField.text
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

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        ColumnLayout
        {
            spacing: Constants.spacing

            Label
            {
                Layout.topMargin: Constants.margin * 2

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

            Item { Layout.fillHeight: true }
        }

        ColumnLayout
        {
            spacing: Constants.spacing

            Label
            {
                Layout.topMargin: Constants.margin * 2

                font.bold: true
                text: qsTr("Other")
            }

            Label { text: qsTr("Web Search URL:") }

            RowLayout
            {
                TextField
                {
                    id: webSearchEngineField

                    Layout.fillWidth: true

                    style: TextFieldStyle
                    {
                        textColor: QmlUtils.userUrlStringIsValid(webSearchEngineField.text) ? "black" : "red"
                    }

                    property string _defaultValue: "https://www.google.com/search?q=%1"
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

            CheckBox
            {
                id: autoBackgroundUpdateCheckCheckbox
                text: qsTr("Check For Updates Automatically")
            }

            Item { Layout.fillHeight: true }
        }
    }
}
