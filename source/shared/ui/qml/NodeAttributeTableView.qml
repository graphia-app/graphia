import QtQuick 2.7
import QtQuick.Controls 1.5

import SortFilterProxyModel 0.2

import "Utils.js" as Utils

TableView
{
    id: tableView

    Component
    {
        id: columnComponent
        TableViewColumn { width: 200 }
    }

    property var nodeAttributesModel

    property bool showCalculatedAttributes: true
    onShowCalculatedAttributesChanged: { tableView.updateColumnVisibility(); }

    property var hiddenColumns: []
    onHiddenColumnsChanged: { tableView.updateColumnVisibility(); }

    function setColumnVisibility(columnName, columnVisible)
    {
        if(columnVisible)
            Utils.setRemove(hiddenColumns, columnName);
        else
            Utils.setAdd(hiddenColumns, columnName);

        updateColumnVisibility();
    }

    function updateColumnVisibility()
    {
        for(var i = 0; i < tableView.columnCount; i++)
        {
            var tableViewColumn = tableView.getColumn(i);
            var hidden = Utils.setContains(hiddenColumns, tableViewColumn.role);
            var showIfCalculated = !nodeAttributesModel.columnIsCalculated(tableViewColumn.role) || showCalculatedAttributes;

            tableViewColumn.visible = !hidden && showIfCalculated;
        }
    }

    sortIndicatorVisible: true
    property string sortRoleName:
    {
        if(nodeAttributesModel.columnNames.length > 0 &&
           nodeAttributesModel.columnNames[sortIndicatorColumn] !== undefined)
        {
            return nodeAttributesModel.columnNames[sortIndicatorColumn];
        }

        return "";
    }

    function createModel()
    {
        model = proxyModelComponent.createObject(tableView,
            {"columnNames": nodeAttributesModel.columnNames});

        // Dynamically create the columns
        while(columnCount > 0)
            tableView.removeColumn(0);

        for(var i = 0; i < nodeAttributesModel.columnNames.length; i++)
        {
            var columnName  = nodeAttributesModel.columnNames[i];
            tableView.addColumn(columnComponent.createObject(tableView,
                {"role": columnName, "title": columnName}));
        }

        // Snap the view back to the start
        // Tableview can be left scrolled out of bounds if column count reduces
        tableView.flickableItem.contentX = 0;
    }

    Connections
    {
        target: nodeAttributesModel
        onColumnNamesChanged:
        {
            // Hack - TableView doesn't respond to rolenames changes
            // so instead we recreate the model to force an update
            createModel();
            populateTableMenu(tableMenu);
        }
    }

    Component.onCompleted: { createModel(); }

    Component
    {
        id: proxyModelComponent

        SortFilterProxyModel
        {
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

    onDoubleClicked:
    {
        var mappedRow = model.mapToSource(row);
        nodeAttributesModel.focusNodeForRowIndex(mappedRow);
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

    // This is just a reference to the menu, so we can repopulate it later as necessary
    property Menu tableMenu

    function populateTableMenu(menu)
    {
        if(menu === null)
            return;

        // Clear out any existing items
        while(menu.items.length > 0)
            menu.removeItem(menu.items[0]);

        menu.title = qsTr("&Table");

        var setVisibleFn = function(visible)
        {
            return function()
            {
                for(var i = 0; i < menu.items.length; i++)
                {
                    var item = menu.items[i];
                    if(item.showColumnNameTag !== undefined)
                        item.checked = visible;
                }
            };
        };

        menu.addItem("").action = resizeColumnsToContentsAction;
        menu.addSeparator();
        menu.addItem("").action = toggleCalculatedAttributes;

        var showAll = menu.addItem(qsTr("&Show All Columns"));
        showAll.triggered.connect(setVisibleFn(true));

        var hideAll = menu.addItem(qsTr("&Hide All Columns"));
        hideAll.triggered.connect(setVisibleFn(false));

        menu.addSeparator();

        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName)
        {
            var menuItem = menu.addItem(columnName);
            menuItem.checkable = true;

            // This is just to let the hide/show all functions know this is a menu item
            menuItem.showColumnNameTag = columnName;

            menuItem.checked = !Utils.setContains(tableView.hiddenColumns, columnName);

            if(plugin.model.nodeAttributeTableModel.columnIsCalculated(columnName))
            {
                menuItem.enabled = Qt.binding(function()
                {
                    return toggleCalculatedAttributes.checked;
                });
            }

            menuItem.toggled.connect(function(checked)
            {
                tableView.setColumnVisibility(columnName, checked);
            });
        });

        tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);
    }

    Menu { id: contextMenu }

    signal rightClick();
    onRightClick: { contextMenu.popup(); }

    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onClicked: { rightClick(); }
    }
}
