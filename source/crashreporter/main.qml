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
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import app.graphia 1.0

import "../shared/ui/qml/Constants.js" as Constants

ApplicationWindow
{
    id: window
    visible: true
    flags: Qt.Window|Qt.Dialog

    title: Qt.application.name + " " + qsTr("Crash Reporter")

    width: 640
    height: 480
    minimumWidth: 640
    minimumHeight: 480

    property bool enabled: true

    GridLayout
    {
        id: grid
        anchors.fill: parent
        anchors.margins: Constants.margin
        columns: 2

        Image
        {
            id: icon
            source: "icon.svg"
            sourceSize.width: 96
            sourceSize.height: 96
            Layout.margins: Constants.margin
            Layout.alignment: Qt.AlignTop
            Layout.rowSpan: 2

            MouseArea
            {
                anchors.fill: parent

                property int _doubleClickCount: 0
                onDoubleClicked:
                {
                    _doubleClickCount++;

                    if(_doubleClickCount >= 3)
                        Qt.exit(127);
                }
            }
        }

        Text
        {
            id: info
            text:
            {
                let apology = qsTr("<b>Oops!</b> We're sorry, ") + Qt.application.name +
                   qsTr(" has crashed. Please use the form below to let us know what happened. " +
                   "If we need more information, we may use your email address " +
                   "to contact you. Thanks.");

                let videoDriverCrash = "";

                let re = /^(nvoglv|ig\d+icd|ati[og]|libGPUSupport|AppleIntel|AMDRadeon).*/;
                if(re.test(crashedModule))
                {
                    let vendorLink = "https://www.google.com/search?q=" + glVendor +
                        "+video+driver+download&btnI";

                    videoDriverCrash = qsTr("<font color=\"red\"><b>Please note:</b></font> this crash " +
                        "occurred in your video drivers. While it is possible that ") + Qt.application.name +
                        qsTr(" is still to blame, please also ensure your drivers are " +
                        "<a href=\"") + vendorLink + qsTr("\">up to date</a>, if you have not already done so.");

                    return apology + "<br><br>" + videoDriverCrash;
                }

                return apology;
            }

            PointingCursorOnHoverLink {}
            onLinkActivated: Qt.openUrlExternally(link);

            wrapMode: Text.WordWrap
            Layout.margins: Constants.margin
            Layout.fillWidth: true
        }

        TextField
        {
            id: email
            enabled: window.enabled
            placeholderText: qsTr("Email address (optional)")
            validator: RegExpValidator { regExp: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/ }
            Layout.fillWidth: true
        }

        PlaceholderTextArea
        {
            id: description
            enabled: window.enabled
            placeholderText: qsTr("A detailed explanation of what you were doing " +
                "immediately prior to the crash\n\n(optional, but gratefully received)")
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.columnSpan: 2
        }

        MessageDialog
        {
            id: invalidEmailDialog
            icon: StandardIcon.Critical
            title: qsTr("Invalid Email Address")
            text: qsTr("Please enter a valid email address.")
        }

        Button
        {
            enabled: window.enabled
            text: qsTr("Send Report")
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignRight
            onClicked:
            {
                if(email.text.length === 0 || email.acceptableInput)
                {
                    window.enabled = false;
                    window.close();
                }
                else
                    invalidEmailDialog.open();
            }
        }

        Preferences
        {
            section: "tracking"
            property alias emailAddress: email.text
        }
    }

    onClosing:
    {
        report.email = email.text;
        report.text = description.text;
    }
}
