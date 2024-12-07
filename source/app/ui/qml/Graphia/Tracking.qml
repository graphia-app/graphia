/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

Rectangle
{
    id: root

    property bool validValues: preferences.permission !== "unresolved"

    Image
    {
        anchors.right: root.right
        anchors.top: root.top

        source: "tracking-background.png"
    }

    color: "#575757"

    Preferences
    {
        id: preferences
        section: "tracking"
        property alias emailAddress: emailField.text
        property string permission: "unresolved"
    }

    ColumnLayout
    {
        anchors.centerIn: parent
        width: 400
        spacing: Constants.spacing

        RowLayout
        {
            Layout.fillWidth: true

            TextField
            {
                Layout.fillWidth: true

                id: emailField

                selectByMouse: true
                placeholderText: qsTr("Email Address")
                validator: RegularExpressionValidator
                {
                    // Check it's a valid email address
                    regularExpression: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/
                }
            }

            Button
            {
                text: qsTr("Submit")
                enabled: emailField.acceptableInput

                onClicked: function(mouse)
                {
                    preferences.permission = "given";
                    trackingDataEntered();
                }
            }
        }

        Text
        {
            id: messageText

            Layout.fillWidth: true
            Layout.topMargin: 16

            text: Utils.format(qsTr("Please provide your email address. " +
                "{0} does not track what analyses you perform or retain " +
                "any data loaded into it - your privacy is assured. We " +
                "ask for you email only so we know how often it is used " +
                "and by whom. You can also choose to use {0} " +
                "<a href=\"anonymous\">anonymously</a>, if you prefer."), application.name)

            color: "white"
            textFormat: Text.RichText
            font.underline: false
            wrapMode: Text.WordWrap

            PointingCursorOnHoverLink {}

            onLinkActivated: function(link)
            {
                preferences.emailAddress = "";
                preferences.permission = "anonymous";
                trackingDataEntered();
            }
        }

        Keys.onPressed: function(event)
        {
            if(!emailField.acceptableInput)
                return;

            switch(event.key)
            {
            case Qt.Key_Enter:
            case Qt.Key_Return:
                event.accepted = true;
                preferences.permission = "given";
                trackingDataEntered();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    onVisibleChanged:
    {
        if(visible)
            emailField.forceActiveFocus();
        else
        {
            // If not visible, remove focus from any text fields
            root.forceActiveFocus();
        }
    }

    signal trackingDataEntered()
}
