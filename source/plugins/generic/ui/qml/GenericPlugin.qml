import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import SortFilterProxyModel 0.2

PluginContent
{
    id: root

    anchors.fill: parent

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Contents")
        iconName: "format-justify-fill"
        checkable: true
        checked: tableView.autoColumnWidth
        onToggled:
        {
            tableView.autoColumnWidth = checked;
        }
    }

    Action
    {
        id: selectColumnsAction
        text: qsTr("&Select Visible Columns")
        iconName: "computer"
        checkable: true
        checked: tableView.columnSelectionMode

        onTriggered: { tableView.columnSelectionMode = !tableView.columnSelectionMode; }
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            tableView.populateTableMenu(menu);
            return true;
        }

        return false;
    }

    toolStrip: ToolBar
    {
        RowLayout
        {
            anchors.fill: parent

            ToolButton { action: resizeColumnsToContentsAction }
            ToolButton { action: selectColumnsAction }
            ToolButton { action: tableView.exportAction }

            Item { Layout.fillWidth: true }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        NodeAttributeTableView
        {
            id: tableView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: plugin.model.nodeAttributeTableModel

            onVisibleRowsChanged:
            {
                selection.clear();

                if(rowCount > 0)
                    selection.selectAll();
            }

            //onSelectedRowsChanged:
            //{
                // If the tableView's selection is less than complete, highlight
                // the corresponding nodes in the graph, otherwise highlight nothing
                //plugin.model.highlightedRows = tableView.selectedRows.length < rowCount ?
                    //tableView.selectedRows : [];
            //}

            onSortIndicatorColumnChanged: { root.saveRequired = true; }
            onSortIndicatorOrderChanged: { root.saveRequired = true; }
        }
    }

    property bool saveRequired: false

    function save()
    {
        var data =
        {
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder
        };

        return data;
    }

    function load(data, version)
    {
        if(data.sortColumn !== undefined)               tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                tableView.sortIndicatorOrder = data.sortOrder;
    }
}
