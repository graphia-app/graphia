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
import QtQuick.Controls 1.5

Item
{
    id: root

    property var selectedValue
    property var selectedValues: []
    property var selectedIndices: []
    property var model

    onModelChanged: { selectedValue = undefined; }

    readonly property int count:
    {
        if(!model)
            return 0;

        if(model.length !== undefined)
            return model.length;

        if(model.count !== undefined)
            return model.count;

        if(typeof model.rowCount === 'function')
            return model.rowCount();

        return 0;
    }

    property bool allowMultipleSelection: false

    // Just some semi-sensible defaults
    width: 200
    height: 100

    TableView
    {
        id: tableView

        anchors.fill: root
        model: root.model

        TableViewColumn { role: "display" }

        // Hide the header
        headerDelegate: Item {}

        alternatingRowColors: false

        selectionMode: root.allowMultipleSelection ?
            SelectionMode.ExtendedSelection : SelectionMode.SingleSelection

        Connections
        {
            target: tableView.selection

            function onSelectionChanged()
            {
                root.selectedValues = [];
                root.selectedIndices = [];

                if(target.count > 0)
                {
                    let newSelectedValues = [];
                    let newSelectedIndices = [];
                    target.forEach(function(rowIndex)
                    {
                        let value;
                        if(typeof root.model.get === 'function')
                            value = root.model.get(rowIndex);
                        else if(typeof root.model.data === 'function')
                            value = root.model.data(root.model.index(rowIndex, 0));
                        else
                            value = root.model[rowIndex];

                        root.selectedValue = value;
                        newSelectedValues.push(value);
                        newSelectedIndices.push(rowIndex);
                    });

                    root.selectedValue = newSelectedValues[newSelectedValues.length - 1];
                    root.selectedValues = newSelectedValues;
                    root.selectedIndices = newSelectedIndices;
                }
                else
                {
                    root.selectedValue = undefined;
                    root.selectedValues = [];
                    root.selectedIndices = [];
                }
            }
        }

        onClicked: { root.clicked(row); }

        onDoubleClicked:
        {
            // See TreeBox for an explanation
            doubleClickHack.row = row;
        }

        Connections
        {
            id: doubleClickHack
            property int row: -1

            target: tableView.__mouseArea

            function onPressedChanged()
            {
                if(!tableView.__mouseArea.pressed && row !== -1)
                {
                    root.doubleClicked(row);
                    row = -1;

                    if(root.selectedValue)
                        root.accepted();
                }
            }
        }

        Keys.onPressed:
        {
            switch(event.key)
            {
            case Qt.Key_Enter:
            case Qt.Key_Return:
                if(root.selectedValue)
                {
                    event.accepted = true;
                    root.accepted();
                }
                break;
            }
        }
    }

    function clear() { tableView.selection.clear(); }
    function selectAll() { if(root.count > 0) tableView.selection.selectAll(); }

    signal accepted()

    signal clicked(var row)
    signal doubleClicked(var row)
}

