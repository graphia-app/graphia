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
import QtQuick.Window 2.2
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0

Window
{
    id: root

    property var document

    title: qsTr("Manage Bookmarks")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
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

            model: document !== null ? document.bookmarks : null

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
                    leftInset: -8

                    property var highlightColor: listBox.highlightedProvider(index) ?
                        palette.highlight : "transparent"

                    background: Rectangle { color: parent.highlightColor }

                    text: modelData

                    color: QmlUtils.contrastingColor(highlightColor)
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }

                TextField
                {
                    id: editField
                    visible: false
                    selectByMouse: true

                    anchors.fill: label
                    padding: 0
                    background: Item {}
                    renderType: Text.NativeRendering

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
                                listBox.rowRenamed(label.text, editField.text);
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

            onDoubleClicked: function(mouse) { edit(index); }

            signal rowRenamed(string from, string to)
            onRowRenamed: { document.renameBookmark(from, to); }
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
                        names.push(document.bookmarks[index]);

                    document.removeBookmarks(names);
                }
            }

            Button
            {
                text: qsTr("Close")
                onClicked: root.close();
            }
        }
    }
}

