/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import app.graphia.Controls
import app.graphia.Utils

Window
{
    id: root

    enum MethodType
    {
        AlwaysAsk,
        Append,
        Replace
    }

    title: qsTr("Add Template")

    property var document: null

    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    color: palette.window

    width: 640
    minimumWidth: 640

    height: 480
    minimumHeight: 480

    ColumnLayout
    {
        id: layout

        spacing: Constants.spacing
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            Layout.fillWidth: true
            spacing: Constants.spacing

            Text
            {
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                color: palette.buttonText

                text: qsTr("A template is a named set of transforms and visualisations " +
                    "that can be saved and then applied to other graphs in future, saving " +
                    "the effort of manual recreation. Please select which of your existing " +
                    "transforms and visualisations you wish to use to create the template. " +
                    "Templates can be applied either by appending to the existing " +
                    "configuration, or by replacing it entirely.")
            }

            NamedIcon
            {
                iconName: "x-office-document-template"

                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop
            }
        }

        RowLayout
        {
            Label
            {
                text: qsTr("Name:")
                color: palette.buttonText
            }

            TextField
            {
                id: nameField
                Layout.fillWidth: true
                Layout.rightMargin: Constants.spacing * 2
                selectByMouse: true
            }

            Label
            {
                text: qsTr("Application Method:")
                color: palette.buttonText
            }

            ComboBox
            {
                id: methodComboBox
                model: [qsTr("Always Ask…"), qsTr("Append"), qsTr("Replace")]
            }
        }

        Label
        {
            text: qsTr("Transforms:")
            color: palette.buttonText
        }

        CheckBoxList
        {
            id: transformsCheckBoxList

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: root.document ? root.document.transforms : []
            textProvider: function(modelData)
            {
                if(!root.document)
                    return modelData;

                return root.document.displayTextForGraphTransform(modelData);
            }
        }

        Label
        {
            text: qsTr("Visualisations:")
            color: palette.buttonText
        }

        CheckBoxList
        {
            id: visualisationsCheckBoxList

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: root.document ? root.document.visualisations : []
            textProvider: function(modelData)
            {
                if(!root.document)
                    return modelData;

                return root.document.displayTextForVisualisation(modelData);
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignBottom

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")

                enabled:
                {
                    if(nameField.text.length === 0)
                        return false;

                    if(transformsCheckBoxList.selected.length === 0 && visualisationsCheckBoxList.selected.length === 0)
                        return false;

                    return true;
                }

                onClicked:
                {
                    root.close();

                    let m = new Map();
                    m.set(qsTr("Always Ask…"),  AddTemplateDialog.MethodType.AlwaysAsk);
                    m.set(qsTr("Append"),       AddTemplateDialog.MethodType.Append);
                    m.set(qsTr("Replace"),      AddTemplateDialog.MethodType.Replace);

                    root.accepted(nameField.text, m.get(methodComboBox.currentText),
                        transformsCheckBoxList.selected, visualisationsCheckBoxList.selected);
                }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); root.rejected(); }
            }
        }
    }

    onVisibleChanged:
    {
        if(visible)
        {
            nameField.clear();
            methodComboBox.currentIndex = 0;
            transformsCheckBoxList.checkAll();
            visualisationsCheckBoxList.checkAll();

            nameField.forceActiveFocus();
        }
    }

    signal accepted(string name, int method, var transforms, var visualisations);
    signal rejected();
}
