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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.Utils

Window
{
    id: root

    property alias model: listBox.model

    title: qsTr("Manage List")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    width: 360
    height: 240
    minimumWidth: 360
    minimumHeight: 240

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        ListBox
        {
            id: listBox

            Layout.fillWidth: true
            Layout.fillHeight: true

            allowMultipleSelection: true

            model: []

            delegate: Item
            {
                anchors.left: parent.left
                anchors.right: parent.right

                implicitHeight: label.height

                Label
                {
                    id: label
                    visible: !editField.visible

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: -leftInset
                    rightPadding: listBox.scrollBarWidth
                    leftInset: -8

                    background: Rectangle
                    {
                        visible: listBox.highlightedProvider(index)
                        color: palette.highlight
                    }

                    text: modelData

                    color: listBox.highlightedProvider(index) ?
                        palette.highlightedText : palette.windowText
                    elide: Text.ElideRight
                }

                TextInput
                {
                    id: editField
                    visible: false
                    selectByMouse: true
                    color: palette.text
                    selectionColor: palette.highlight
                    selectedTextColor: palette.highlightedText

                    anchors.fill: label
                    padding: 0

                    onAccepted: { finish(); }

                    onActiveFocusChanged:
                    {
                        if(!activeFocus)
                            finish();
                    }

                    function finish()
                    {
                        if(editField.visible)
                        {
                            editField.visible = false;

                            if(editField.text !== label.text)
                                root.rename(label.text, editField.text);
                        }
                    }
                }

                function edit()
                {
                    listBox.clearSelection();

                    editField.text = label.text;
                    editField.selectAll();
                    editField.visible = true;
                    editField.forceActiveFocus();
                }
            }

            function edit(index)
            {
                let item = itemAt(index);
                if(item !== null)
                    item.edit();
            }

            onDoubleClicked: function(index) { edit(index); }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("Rename")
                enabled: listBox.selectedIndex >= 0
                onClicked: function(mouse) { listBox.edit(listBox.selectedIndex); }
            }

            Button
            {
                text: qsTr("Remove")
                enabled: listBox.selectedIndex >= 0
                onClicked: function(mouse)
                {
                    let names = [];
                    for(const index of listBox.selectedIndices)
                        names.push(root.model[index]);

                    root.remove(names);
                }
            }

            Button
            {
                text: qsTr("Close")
                onClicked: root.close();
            }
        }
    }

    signal remove(var names)
    signal rename(string from, string to)
}

