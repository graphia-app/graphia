/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

Item
{
    id: root

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

    property bool _trackingPermissionUnresolved: tracking.permission.length === 0 ||
        tracking.permission === "unresolved"

    Preferences
    {
        id: proxy
        section: "proxy"

        property string type
        property alias host: proxyHostnameTextField.text
        property alias port: proxyPortTextField.text
        property alias username: proxyUsernameTextField.text
        property alias password: proxyPasswordTextField.text
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
        else if(!root._trackingPermissionUnresolved)
        {
            anonTrackingRadioButton.checked = true;
            emailField.text = "";
        }

        if(proxy.type === "disabled")
            proxyTypeComboBox.currentIndex = proxyTypeComboBox.model.indexOf(qsTr("Disabled"));
        else if(proxy.type === "http")
            proxyTypeComboBox.currentIndex = proxyTypeComboBox.model.indexOf(qsTr("HTTP"));
        else if(proxy.type === "socks5")
            proxyTypeComboBox.currentIndex = proxyTypeComboBox.model.indexOf(qsTr("SOCKS5"));
    }

    ButtonGroup
    {
        buttons: [yesTrackingRadioButton, anonTrackingRadioButton, noTrackingRadioButton]
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
                font.bold: true
                text: qsTr("Allow Tracking")
            }

            RowLayout
            {
                RadioButton
                {
                    id: yesTrackingRadioButton
                    text: qsTr("Yes")

                    onCheckedChanged:
                    {
                        if(checked && !root._trackingPermissionUnresolved)
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
                text: qsTr("Yes, Anonymously")

                onCheckedChanged:
                {
                    if(checked && !root._trackingPermissionUnresolved)
                        tracking.permission = "anonymous";
                }
            }

            RadioButton
            {
                id: noTrackingRadioButton
                text: qsTr("No")

                onCheckedChanged:
                {
                    if(checked && !root._trackingPermissionUnresolved)
                        tracking.permission = "refused";
                }
            }

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

                    color: QmlUtils.userUrlStringIsValid(webSearchEngineField.text) ? "black" : "red"

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

        ColumnLayout
        {
            Layout.minimumWidth: 250
            spacing: Constants.spacing

            Label
            {
                font.bold: true
                text: qsTr("Proxy")
            }

            ComboBox
            {
                id: proxyTypeComboBox

                model: [qsTr("Disabled"), qsTr("HTTP"), qsTr("SOCKS5")]

                onCurrentIndexChanged:
                {
                    let v = model[currentIndex];

                    if(proxyHostnameTextField.text.length === 0)
                    {
                        if(v === qsTr("HTTP"))
                            proxyPortTextField.text = "8080";
                        else if(v === qsTr("SOCKS5"))
                            proxyPortTextField.text = "1080";
                    }

                    if(v === qsTr("Disabled"))
                        proxy.type = "disabled";
                    else if(v === qsTr("HTTP"))
                        proxy.type = "http";
                    else if(v === qsTr("SOCKS5"))
                        proxy.type = "socks5";
                }
            }

            RowLayout
            {
                Layout.fillWidth: true

                enabled: proxyTypeComboBox.currentIndex > 0

                TextField
                {
                    id: proxyHostnameTextField

                    Layout.fillWidth: true

                    placeholderText: qsTr("Host")
                    validator: RegExpValidator { regExp: /(([a-z0-9]+|([a-z0-9]+[-]+[a-z0-9]+))[.])+/ }
                }

                TextField
                {
                    id: proxyPortTextField

                    Layout.preferredWidth: 64

                    placeholderText: qsTr("Port")
                    validator: IntValidator { bottom: 1; top: 65535 }
                }
            }

            RowLayout
            {
                Layout.fillWidth: true

                enabled: proxyTypeComboBox.currentIndex > 0

                TextField
                {
                    id: proxyUsernameTextField

                    Layout.fillWidth: true

                    placeholderText: qsTr("Username")
                }

                TextField
                {
                    id: proxyPasswordTextField

                    Layout.fillWidth: true

                    placeholderText: qsTr("Password")
                    echoMode: TextInput.Password
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
