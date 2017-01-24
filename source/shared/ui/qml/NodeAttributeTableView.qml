import QtQuick 2.5
import QtQuick.Controls 1.4

import SortFilterProxyModel 0.1

TableView
{
    Component
    {
        id: columnComponent
        TableViewColumn { width: 200 }
    }

    property var nodeAttributesModel

    id: tableView

    sortIndicatorVisible: true
    property string sortRoleName: columnNames.length > 0 ? columnNames[sortIndicatorColumn] : ""

    model: SortFilterProxyModel
    {
        property var columnNames: sourceModel.columnNames
        sourceModel: nodeAttributesModel
        sortRoleName: tableView.sortRoleName
        sortOrder: tableView.sortIndicatorOrder

        filterRoleName: 'nodeSelected'; filterValue: true

        // NodeAttributeTableModel fires layoutChanged whenever the nodeSelected role
        // changes which in turn affects which rows the model reflects. When the visible
        // rows change, we emit a signal so that the owners of the TableView can react.
        // Qt.callLater is used because otherwise the signal is emitted before the
        // TableView has had a chance to update.
        onLayoutChanged: { Qt.callLater(visibleRowsChanged); }
    }

    signal visibleRowsChanged();

    selectionMode: SelectionMode.ExtendedSelection

    property var selectedRows: []

    // Implementing selectedRows using a binding results in a binding loop,
    // for some reason, so do it by connection instead
    Connections
    {
        target: selection
        onSelectionChanged:
        {
            var rows = [];
            tableView.selection.forEach(function(rowIndex)
            {
                var modelIndex = model.index(rowIndex, 0);
                var sourceModelIndex = model.mapToSource(modelIndex);
                var sourceRowIndex = nodeAttributesModel.modelIndexRow(sourceModelIndex);
                rows.push(sourceRowIndex);
            });

            selectedRows = rows;
        }
    }

    property var columnNames: model.columnNames

    onColumnNamesChanged:
    {
        // Dynamically create the columns
        while(columnCount > 0)
            tableView.removeColumn(0);

        for(var i = 0; i < columnNames.length; i++)
        {
            var columnName  = columnNames[i];
            tableView.addColumn(columnComponent.createObject(tableView,
                {"role": columnName, "title": columnName}));
        }
    }
}
