import QtQuick 2.7
import QtQuick.Controls 1.5

import SortFilterProxyModel 0.2

TableView
{
    id: tableView

    Component
    {
        id: columnComponent
        TableViewColumn { width: 200 }
    }

    property var nodeAttributesModel
    property var showCalculatedAttributes: true
    onShowCalculatedAttributesChanged:
    {
        if(nodeAttributesModel !== null)
            nodeAttributesModel.showCalculatedAttributes = showCalculatedAttributes;
    }

    sortIndicatorVisible: true
    property string sortRoleName:
    {
        if(columnNames.length > 0 && columnNames[sortIndicatorColumn] !== undefined)
            return columnNames[sortIndicatorColumn];
        else
            return "";
    }

    Connections
    {
        target: nodeAttributesModel
        onColumnNamesChanged:
        {
            // Hack - TableView doesn't respond to rolenames changes
            // so we need to set to null and reset the model + proxy
            columnNames = proxyModel.columnNames
            model = null
            proxyModel.columnNames = nodeAttributesModel.columnNames
            proxyModel.sourceModel = null
            proxyModel.sourceModel = nodeAttributesModel
            model = proxyModel
            columnNames = proxyModel.columnNames
        }
    }

    model: SortFilterProxyModel
    {
        id: proxyModel
        property var columnNames: sourceModel.columnNames
        sourceModel: nodeAttributesModel
        sortRoleName: tableView.sortRoleName
        ascendingSortOrder: tableView.sortIndicatorOrder === Qt.AscendingOrder

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
                rows.push(model.mapToSource(rowIndex));
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

        // Snap the view back to the start
        // Tableview can be left scrolled out of bounds if column count reduces
        tableView.flickableItem.contentX = 0;
    }

    // Work around for QTBUG-58594
    function resizeColumnsToContentsBugWorkaround()
    {
        for(var i = 0; i < columnCount; ++i)
        {
            var col = getColumn(i);
            var header = __listView.headerItem.headerRepeater.itemAt(i);
            if(col)
            {
                col.__index = i;
                col.resizeToContents();
                if(col.width < header.implicitWidth)
                    col.width = header.implicitWidth;
            }
        }
    }
}
