/* Copyright © 2013-2020 Graphia Technologies Ltd.
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
import QtQml.Models 2.2

import SortFilterProxyModel 0.2

Item
{
    property var selectedValue
    property var model: null

    property var currentIndex:
    {
        return sortFilterProxyModel.mapToSource(treeView.selection.currentIndex);
    }

    property alias sortRoleName: sortFilterProxyModel.sortRoleName
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property bool showSections: false

    property bool showSearch: false

    onModelChanged: { selectedValue = undefined; }

    id: root

    // Just some semi-sensible defaults
    width: 200
    height: 100

    ColumnLayout
    {
        anchors.fill: root

        TreeBoxSearch
        {
            id: treeBoxSearch

            Layout.fillWidth: true

            visible: false

            onVisibleChanged:
            {
                if(!visible)
                    treeView.forceActiveFocus();
            }

            treeBox: root
            onAccepted: { visible = false; }
        }

        TreeView
        {
            id: treeView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: SortFilterProxyModel
            {
                id: sortFilterProxyModel
                sourceModel: root.model !== undefined ? root.model : null
            }

            // Clear the selection when the model is changed
            selection: ItemSelectionModel { model: treeView.model }
            onModelChanged: { selection.clear(); }

            TableViewColumn { role: "display" }

            // Hide the header
            headerDelegate: Item {}

            alternatingRowColors: false

            section.property: showSections && root.sortRoleName.length > 0 ? root.sortRoleName : ""
            section.delegate: Component
            {
                Text
                {
                    // "Hide" when the section text is empty
                    height: text.length > 0 ? implicitHeight : 0
                    text: section
                    font.italic: true
                    font.bold: true
                }
            }

            Connections
            {
                target: treeView.selection
                function onSelectionChanged()
                {
                    if(!root.model)
                        return;

                    var sourceIndex = treeView.model.mapToSource(target.currentIndex);

                    if(typeof root.model.get === 'function')
                        root.selectedValue = root.model.get(sourceIndex);
                    else if(typeof root.model.data === 'function')
                        root.selectedValue = root.model.data(sourceIndex);
                    else
                        root.selectedValue = undefined;
                }
            }

            onDoubleClicked:
            {
                root.doubleClicked(index);

                // FIXME: There seems to be a bug in TreeView where if it is hidden in
                // the middle of a click event, it gets into a strange state where the
                // mouse button state gets stuck. Thereafter, if it is shown again simply
                // moving the mouse over the items in the list selects them. This is
                // obviously undesirable and needs to be investigated fully, but for now
                // toggling the visibility of the TreeView's MouseArea seems to stop it
                // happening:
                treeView.__mouseArea.visible = false;
                treeView.__mouseArea.visible = true;
            }

            Keys.onPressed:
            {
                if(!root.showSearch)
                    return;

                if((event.key === Qt.Key_F && event.modifiers & Qt.ControlModifier) ||
                    event.key === Qt.Key_Slash)
                {
                    event.accepted = true;
                    root.toggleSearch();
                }
            }

            FloatingButton
            {
                visible: root.showSearch && root.enabled

                anchors.rightMargin: treeView.__verticalScrollBar.visible ?
                    treeView.__verticalScrollBar.width + 4 : 4
                anchors.bottomMargin: 4
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                iconName: "edit-find"
                hoverOpacity: 0.7

                onClicked: { root.toggleSearch(); }
            }
        }
    }

    function toggleSearch()
    {
        if(!root.showSearch)
            return;

        treeBoxSearch.visible = !treeBoxSearch.visible;
    }

    function _mapIndexToRow(index)
    {
        // This hideous hack is the only obvious way to get from
        // a QModelIndex to a row index that corresponds to an item
        // in the TreeView
        for(let i = 0; i < treeView.__listView.count; i++)
        {
            if(treeView.__model.mapRowToModelIndex(i) === index)
                return i;
        }

        return -1;
    }

    function select(modelIndex)
    {
        // Do the selection
        let index = sortFilterProxyModel.mapFromSource(modelIndex);
        treeView.selection.setCurrentIndex(index, ItemSelectionModel.NoUpdate);
        treeView.selection.select(index, ItemSelectionModel.ClearAndSelect);

        // Ensure any containing tree branches are expanded
        let parentIndex = index.parent;
        while(parentIndex.valid)
        {
            if(!treeView.isExpanded(parentIndex))
                treeView.expand(parentIndex);

            parentIndex = parentIndex.parent;
        }

        // Scroll so that it's visible
        let row = _mapIndexToRow(index);
        treeView.__listView.positionViewAtIndex(row, ListView.Center);
    }

    function deselect()
    {
        treeView.selection.clearCurrentIndex();
    }

    signal doubleClicked(var index)
}

