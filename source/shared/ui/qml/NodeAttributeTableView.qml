import QtQuick 2.7
import QtQuick.Controls 1.5

import Qt.labs.platform 1.0 as Labs

import SortFilterProxyModel 0.2

import com.kajeka 1.0

import "Utils.js" as Utils

Item
{
    id: root

    property var nodeAttributesModel

    property bool showCalculatedAttributes: true
    onShowCalculatedAttributesChanged: { tableView._updateColumnVisibility(); }

    property var hiddenColumns: []
    onHiddenColumnsChanged: { tableView._updateColumnVisibility(); }

    property var selectedRows: []

    readonly property string sortRoleName:
    {
        if(nodeAttributesModel.columnNames.length > 0 &&
           nodeAttributesModel.columnNames[sortIndicatorColumn] !== undefined)
        {
            return nodeAttributesModel.columnNames[sortIndicatorColumn];
        }

        return "";
    }

    property alias sortIndicatorOrder: tableView.sortIndicatorOrder
    property alias sortIndicatorColumn: tableView.sortIndicatorColumn
    property alias selection: tableView.selection
    property alias rowCount: tableView.rowCount
    property alias viewport: tableView.viewport

    signal visibleRowsChanged();

    function setColumnVisibility(columnName, columnVisible)
    {
        if(columnVisible)
            Utils.setRemove(hiddenColumns, columnName);
        else
            Utils.setAdd(hiddenColumns, columnName);

        tableView._updateColumnVisibility();
    }

    // Work around for QTBUG-58594
    function resizeColumnsToContentsBugWorkaround()
    {
        for(var i = 0; i < tableView.columnCount; ++i)
        {
            var col = tableView.getColumn(i);
            var header = tableView.__listView.headerItem.headerRepeater.itemAt(i);
            if(col)
            {
                col.__index = i;
                col.resizeToContents();
                if(col.width < header.implicitWidth)
                    col.width = header.implicitWidth;
            }
        }
    }

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
        menu.addItem("").action = exportTableAction;
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

            menuItem.checked = !Utils.setContains(hiddenColumns, columnName);

            if(plugin.model.nodeAttributeTableModel.columnIsCalculated(columnName))
            {
                menuItem.enabled = Qt.binding(function()
                {
                    return toggleCalculatedAttributes.checked;
                });
            }

            menuItem.toggled.connect(function(checked)
            {
                root.setColumnVisibility(columnName, checked);
            });
        });

        tableView._tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);
    }

    Label
    {
        text: qsTr("No Visible Columns")
        visible: tableView.numVisibleColumns <= 0

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }

    Labs.FileDialog
    {
        id: exportTableDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 1
        title: "Export Table"
        nameFilters: ["CSV File (*.csv)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            document.writeTableViewToFile(tableView, file);
        }
    }

    Action
    {
        id: exportTableAction
        enabled: tableView.rowCount > 0
        text: qsTr("Export…")
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder;
            exportTableDialog.open();
        }
    }

    TableView
    {
        id: tableView

        visible: numVisibleColumns > 0
        anchors.fill: parent

        readonly property int numVisibleColumns:
        {
            var count = 0;

            for(var i = 0; i < tableView.columnCount; i++)
            {
                var tableViewColumn = tableView.getColumn(i);

                if(tableViewColumn.visible)
                    count++;
            }

            return count;
        }

        Component
        {
            id: columnComponent
            TableViewColumn { width: 200 }
        }

        function _updateColumnVisibility()
        {
            for(var i = 0; i < tableView.columnCount; i++)
            {
                var tableViewColumn = tableView.getColumn(i);
                var hidden = Utils.setContains(root.hiddenColumns, tableViewColumn.role);
                var showIfCalculated = !root.nodeAttributesModel.columnIsCalculated(tableViewColumn.role) || root.showCalculatedAttributes;

                tableViewColumn.visible = !hidden && showIfCalculated;
            }
        }

        sortIndicatorVisible: true

        function _createModel()
        {
            model = proxyModelComponent.createObject(tableView,
                {"columnNames": root.nodeAttributesModel.columnNames});

            // Dynamically create the columns
            while(columnCount > 0)
                tableView.removeColumn(0);

            for(var i = 0; i < root.nodeAttributesModel.columnNames.length; i++)
            {
                var columnName  = root.nodeAttributesModel.columnNames[i];
                tableView.addColumn(columnComponent.createObject(tableView,
                    {"role": columnName, "title": columnName}));
            }

            // Snap the view back to the start
            // Tableview can be left scrolled out of bounds if column count reduces
            tableView.flickableItem.contentX = 0;
        }

        Connections
        {
            target: root.nodeAttributesModel
            onColumnNamesChanged:
            {
                // Hack - TableView doesn't respond to rolenames changes
                // so instead we recreate the model to force an update
                tableView._createModel();
                populateTableMenu(_tableMenu);
            }
        }

        Component.onCompleted: { tableView._createModel(); }

        Component
        {
            id: proxyModelComponent

            SortFilterProxyModel
            {
                sourceModel: root.nodeAttributesModel
                sortRoleName: root.sortRoleName
                ascendingSortOrder: tableView.sortIndicatorOrder === Qt.AscendingOrder

                filterRoleName: 'nodeSelected'; filterValue: true

                // NodeAttributeTableModel fires layoutChanged whenever the nodeSelected role
                // changes which in turn affects which rows the model reflects. When the visible
                // rows change, we emit a signal so that the owners of the TableView can react.
                // Qt.callLater is used because otherwise the signal is emitted before the
                // TableView has had a chance to update.
                onLayoutChanged: { Qt.callLater(root.visibleRowsChanged); }
            }
        }

        selectionMode: SelectionMode.ExtendedSelection

        // Implementing selectedRows using a binding results in a binding loop,
        // for some reason, so do it by connection instead
        Connections
        {
            target: tableView.selection
            onSelectionChanged:
            {
                var rows = [];
                tableView.selection.forEach(function(rowIndex)
                {
                    rows.push(tableView.model.mapToSource(rowIndex));
                });

                root.selectedRows = rows;
            }
        }

        onDoubleClicked:
        {
            var mappedRow = model.mapToSource(row);
            root.nodeAttributesModel.focusNodeForRowIndex(mappedRow);
        }

        // This is just a reference to the menu, so we can repopulate it later as necessary
        property Menu _tableMenu

        // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
        // except for the text property
        itemDelegate: Item
        {
            height: Math.max(16, label.implicitHeight)
            property int implicitWidth: label.implicitWidth + 16

            Text
            {
                id: label
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

                text:
                {
                    if(styleData.value === undefined)
                        return "";

                    var column = tableView.getColumn(styleData.column);

                    if(column !== null && nodeAttributesModel.columnIsFloatingPoint(column.role))
                        return Utils.formatForDisplay(styleData.value, 1);

                    return styleData.value;
                }

                color: styleData.textColor
                renderType: Text.NativeRendering
            }
        }
    }

    Menu { id: contextMenu }

    signal rightClick();
    onRightClick: { contextMenu.popup(); }

    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onClicked: { root.rightClick(); }
    }
}
