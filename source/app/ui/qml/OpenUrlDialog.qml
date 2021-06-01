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
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import app.graphia 1.0

import "../../../shared/ui/qml/Constants.js" as Constants

Window
{
    id: root

    property string url

    title: qsTr("Open URL")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 480
    minimumWidth: 480

    property int _contentHeight: columnLayout.implicitHeight + (2 * Constants.margin)

    height: _contentHeight
    minimumHeight: _contentHeight
    maximumHeight: _contentHeight

    onVisibleChanged:
    {
        if(visible)
            textField.text = "";
    }

    ColumnLayout
    {
        id: columnLayout

        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        Label { text: qsTr("Enter a resource URL:") }

        RowLayout
        {
            TextField
            {
                id: textField

                property bool valid: { return textField.length > 0 && QmlUtils.userUrlStringIsValid(text); }

                style: TextFieldStyle
                {
                    textColor: textField.length === 0 || textField.valid ?
                        "black" : "red"
                }

                Layout.fillWidth: true
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: textField.valid

                onClicked: root.accept();
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: root.reject();
            }
        }

        Keys.onPressed:
        {
            event.accepted = true;
            switch(event.key)
            {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                root.reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                root.accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function accept()
    {
        if(!textField.valid)
            return;

        root.url = QmlUtils.urlFrom(textField.text);
        root.close();
        root.accepted();
    }

    function reject()
    {
        root.close();
        root.rejected();
    }

    signal accepted()
    signal rejected()
}

