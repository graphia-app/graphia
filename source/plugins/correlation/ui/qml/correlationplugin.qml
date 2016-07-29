import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.1

import "Constants.js" as Constants

Item
{
    anchors.fill: parent

    Component
    {
        id: columnComponent
        TableViewColumn { width: 200 }
    }

    ColumnLayout
    {
        anchors.fill: parent

        TableView
        {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true

            sortIndicatorVisible: true
            property string sortRoleName: columnNames[sortIndicatorColumn]

            model: SortFilterProxyModel
            {
                property var columnNames: sourceModel.columnNames
                sourceModel: plugin.model.rowAttributes
                sortRoleName: tableView.sortRoleName
                sortOrder: tableView.sortIndicatorOrder
            }

            selectionMode: SelectionMode.ExtendedSelection

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
    }
}
