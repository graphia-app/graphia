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
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import "../../../shared/ui/qml/Constants.js" as Constants

import "Controls"

Window
{
    id: root

    property var document

    title: qsTr("Manage Bookmarks")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 320
    height: 240
    minimumWidth: 320
    minimumHeight: 240

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        TableView
        {
            id: tableView

            implicitWidth: 192
            Layout.fillWidth: true
            Layout.fillHeight: true

            property var currentItem:
            {
                // This is gigantic hack that delves deep in TableView in order to get the
                // currently selected item
                if(!__currentRowItem)
                    return null;

                let item = __currentRowItem.rowItem; // FocusScope (id: rowitem)
                if(!item)
                    return null;

                item = item.children[1]; // Row (id: itemrow)
                if(!item)
                    return null;

                let columnIndex = 0;
                item = item.children[columnIndex]; // Repeater.delegate (TableView.__itemDelegateLoader)
                if(!item)
                    return null;

                item = item.item;
                return item;
            }

            TableViewColumn { role: "name" }
            headerDelegate: Item {}

            itemDelegate: Item
            {
                id: item

                height: Math.max(16, label.implicitHeight)
                property int implicitWidth: label.implicitWidth + 16

                Text
                {
                    id: label
                    visible: !editField.visible

                    objectName: "label"
                    width: parent.width
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: styleData.hasOwnProperty("depth") && styleData.column === 0 ? 0 :
                                        horizontalAlignment === Text.AlignRight ? 1 : 8
                    anchors.rightMargin: (styleData.hasOwnProperty("depth") && styleData.column === 0)
                                         || horizontalAlignment !== Text.AlignRight ? 1 : 8
                    horizontalAlignment: styleData.textAlignment
                    anchors.verticalCenter: parent.verticalCenter
                    elide: styleData.elideMode

                    text: styleData.value

                    color: styleData.textColor
                    renderType: Text.NativeRendering

                    MouseArea
                    {
                        anchors.fill: parent
                        onPressed:
                        {
                            if(!(mouse.modifiers & Qt.ControlModifier))
                                tableView.selection.clear();

                            if((mouse.modifiers & Qt.ShiftModifier) && tableView.currentRow !== -1)
                            {
                                for(let i = tableView.currentRow; i < styleData.row; i++)
                                    tableView.selection.select(i);
                            }

                            tableView.selection.select(styleData.row);
                            tableView.currentRow = styleData.row;
                        }

                        onDoubleClicked: { item.startEditing(); }
                    }
                }

                TextField
                {
                    id: editField
                    visible: false
                    anchors.fill: label

                    style: TextFieldStyle
                    {
                        padding.left: 0; padding.right: 0
                        padding.top: 0; padding.bottom: 0
                        background: Rectangle { color: "transparent" }
                    }

                    onAccepted: { _finishEditing(); }

                    onActiveFocusChanged:
                    {
                        if(!activeFocus)
                            _finishEditing();
                    }

                    function _finishEditing()
                    {
                        if(editField.visible)
                        {
                            editField.visible = false;

                            if(editField.text !== label.text)
                                tableView.rowRenamed(label.text, editField.text);
                        }
                    }
                }

                function startEditing()
                {
                    tableView.selection.clear();

                    editField.text = label.text;
                    editField.selectAll();
                    editField.visible = true;
                    editField.forceActiveFocus();
                }
            }

            model: document !== null ? document.bookmarks : null

            signal rowRenamed(string from, string to)

            onRowRenamed:
            {
                document.renameBookmark(from, to);
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("Rename")
                enabled: tableView.selection.count > 0
                onClicked:
                {
                    tableView.currentItem.startEditing();
                }
            }

            Button
            {
                text: qsTr("Remove")
                enabled: tableView.selection.count > 0
                onClicked:
                {
                    let names = [];
                    tableView.selection.forEach(function(rowIndex)
                    {
                        names.push(document.bookmarks[rowIndex]);
                    });

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

