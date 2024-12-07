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

import Graphia.Utils

Window
{
    id: root

    property var model: null
    property string displayRole: "display"
    property string valueRole: "display"
    property string explanationText: ""
    property string choiceLabelText: ""

    modality: Qt.ApplicationModal
    flags: Constants.defaultWindowFlags
    color: palette.window

    width: 420
    minimumWidth: width
    maximumWidth: width

    height: 220
    minimumHeight: height
    maximumHeight: height

    onVisibleChanged:
    {
        if(visible)
            rememberThisChoiceCheckBox.checked = false;
    }

    ColumnLayout
    {
        id: layout

        spacing: Constants.spacing
        anchors.fill: parent
        anchors.margins: Constants.margin

        Text
        {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            text: explanationText
            wrapMode: Text.WordWrap
            color: palette.buttonText
        }

        RowLayout
        {
            spacing: Constants.spacing

            Text
            {
                color: palette.buttonText
                text: root.choiceLabelText
            }

            ComboBox
            {
                id: comboBox
                Layout.preferredWidth: 200

                model: root.model

                property string selectedValue:
                {
                    if(root.model === null || currentIndex < 0)
                        return "";

                    let role = NativeUtils.modelRoleForName(root.model, root.valueRole);
                    return root.model.data(root.model.index(currentIndex, 0), role);
                }

                textRole: root.displayRole
            }
        }

        CheckBox
        {
            id: rememberThisChoiceCheckBox
            Layout.alignment: Qt.AlignBottom
            text: qsTr("Remember This Choice")
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignBottom

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                onClicked: { root.close(); root.accepted(); }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); root.rejected(); }
            }
        }
    }

    property var _onAcceptedFn: null

    onAccepted:
    {
        if(_onAcceptedFn !== null)
            _onAcceptedFn(comboBox.selectedValue, rememberThisChoiceCheckBox.checked);
    }

    function open(onAcceptedFn)
    {
        // Force comboBox.selectedValue to be updated
        comboBox.currentIndex = -1;
        comboBox.currentIndex = 0;

        root._onAcceptedFn = onAcceptedFn;
        Qt.callLater(function()
        {
            // Delay the opening in case an existing choice is still "in-flight",
            // e.g. when choosing a plugin immediately after choosing file type
            show();
        });
    }

    signal accepted();
    signal rejected();
}
