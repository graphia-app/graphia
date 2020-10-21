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
import QtQml.Models 2.2

import SortFilterProxyModel 0.2

Item
{
    property var selectedValue
    property var model

    property alias sortRoleName: sortFilterProxyModel.sortRoleName
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property bool showSections: false

    onModelChanged: { selectedValue = undefined; }

    id: root

    // Just some semi-sensible defaults
    width: 200
    height: 100

    TreeView
    {
        id: treeView

        anchors.fill: root
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
            // FIXME: There seems to be a bug in TreeView(?) where if it is hidden in
            // the middle of a click, the mouse release event never gets delivered to
            // its MouseArea, and it gets into a state where the mouse button is
            // considered held down. This results in moving the mouse over the list
            // selecting the items, without any clicks. So instead we wait until the
            // mouse button is released, and then trigger the doubleClicked signal.

            doubleClickHack.index = index;
        }

        Connections
        {
            id: doubleClickHack
            property var index: null

            target: treeView.__mouseArea

            function onPressedChanged()
            {
                if(!treeView.__mouseArea.pressed && index !== null)
                {
                    root.doubleClicked(index);
                    index = null;
                }
            }
        }
    }

    signal doubleClicked(var index)
}

