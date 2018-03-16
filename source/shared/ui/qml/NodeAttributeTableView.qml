import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Private 1.0 // StyleItem
import QtGraphicalEffects 1.0

import Qt.labs.platform 1.0 as Labs

import SortFilterProxyModel 0.2

import com.kajeka 1.0

import "Controls"
import "../../../shared/ui/qml/Utils.js" as Utils
import "../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    // External name
    property alias model: root._nodeAttributesTableModel

    // Internal name
    property var _nodeAttributesTableModel

    property var hiddenColumns: []
    onHiddenColumnsChanged: { tableView._updateColumnVisibility(); }

    property var selectedRows: []

    readonly property string sortRoleName:
    {
        if(_nodeAttributesTableModel.columnNames.length > 0 &&
           _nodeAttributesTableModel.columnNames[sortIndicatorColumn] !== undefined)
        {
            return _nodeAttributesTableModel.columnNames[sortIndicatorColumn];
        }

        return "";
    }

    property bool columnSelectionMode: false
    onColumnSelectionModeChanged:
    {
        if(columnSelectionMode)
        {
            _sortEnabled = false;
            tableView.headerDelegate = columnSelectionHeaderDelegate
            columnSelectionControls.show();
        }
        else
        {
            columnSelectionControls.hide();

            // The column selection mode will probably have changed these values, so set them
            // back to what they were before
            tableView.sortIndicatorColumn = root.sortIndicatorColumn;
            tableView.sortIndicatorOrder = root.sortIndicatorOrder;
            _sortEnabled = true;
            tableView.flickableItem.contentX = 0;

            tableView.headerDelegate = defaultTableView.headerDelegate;
        }

        tableView._updateColumnVisibility();
    }

    property bool _sortEnabled: true

    Connections
    {
        target: tableView

        onSortIndicatorColumnChanged:
        {
            if(_sortEnabled)
                root.sortIndicatorColumn = tableView.sortIndicatorColumn;
        }

        onSortIndicatorOrderChanged:
        {
            if(_sortEnabled)
                root.sortIndicatorOrder = tableView.sortIndicatorOrder;
        }
    }

    property int sortIndicatorColumn
    onSortIndicatorColumnChanged:
    {
        tableView.sortIndicatorColumn = root.sortIndicatorColumn;
    }

    property int sortIndicatorOrder
    onSortIndicatorOrderChanged:
    {
        tableView.sortIndicatorOrder = root.sortIndicatorOrder;
    }

    property alias selection: tableView.selection
    property alias rowCount: tableView.rowCount
    property alias viewport: tableView.viewport

    signal visibleRowsChanged();

    function setColumnVisibility(columnName, columnVisible)
    {
        if(columnVisible)
            hiddenColumns = Utils.setRemove(hiddenColumns, columnName);
        else
            hiddenColumns = Utils.setAdd(hiddenColumns, columnName);
    }

    function showAllColumns()
    {
        hiddenColumns = [];
    }

    function showAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setRemove(columns, columnName);
        });

        hiddenColumns = columns;
    }

    function hideAllColumns()
    {
        var columns = [];
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName)
        {
            columns.push(columnName);
        });

        hiddenColumns = columns;
    }

    function hideAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setAdd(columns, columnName);
        });

        hiddenColumns = columns;
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

        menu.addItem("").action = resizeColumnsToContentsAction;
        menu.addItem("").action = exportTableAction;
        menu.addSeparator();
        menu.addItem("").action = selectColumnsAction;

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
        title: qsTr("Export Table")
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

    // This only exists to provide the real tableView with its default headerDelegate
    TableView
    {
        id: defaultTableView
        visible: false
        sortIndicatorVisible: true
        sortIndicatorColumn: root.sortIndicatorColumn
        sortIndicatorOrder: root.sortIndicatorOrder
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

                if(root.columnSelectionMode)
                    tableViewColumn.visible = true;
                else
                    tableViewColumn.visible = !Utils.setContains(root.hiddenColumns, tableViewColumn.role);
            }
        }

        sortIndicatorVisible: true

        model: SortFilterProxyModel
        {
            sourceModel: root._nodeAttributesTableModel
            sortRoleName: root.sortRoleName
            ascendingSortOrder: root.sortIndicatorOrder === Qt.AscendingOrder

            filterRoleName: 'nodeSelected'; filterValue: true

            // NodeAttributeTableModel fires layoutChanged whenever the nodeSelected role
            // changes which in turn affects which rows the model reflects. When the visible
            // rows change, we emit a signal so that the owners of the TableView can react.
            // Qt.callLater is used because otherwise the signal is emitted before the
            // TableView has had a chance to update.
            // This is a gigantic hack; it would be much nicer to react when a TableView's
            // contents change, but there is no obvious sane way to do this.
            onLayoutChanged: { Qt.callLater(root.visibleRowsChanged); }
        }

        Connections
        {
            target: root._nodeAttributesTableModel

            onColumnAdded:
            {
                tableView.insertColumn(index, columnComponent.createObject(tableView,
                    {"role": name, "title": name}));

                tableView._updateColumnVisibility();
            }

            onColumnRemoved:
            {
                // Remove columns from the hidden columns list that no longer exist in the model
                root.hiddenColumns = Utils.setIntersection(root.hiddenColumns,
                    root._nodeAttributesTableModel.columnNames);

                tableView.removeColumn(index);

                tableView._updateColumnVisibility();

                // Snap the view back to the start
                // Tableview can be left scrolled out of bounds if column count reduces
                tableView.flickableItem.contentX = 0;
            }
        }

        Component.onCompleted:
        {
            for(var i = 0; i < root._nodeAttributesTableModel.columnNames.length; i++)
            {
                var columnName = root._nodeAttributesTableModel.columnNames[i];
                tableView.addColumn(columnComponent.createObject(tableView,
                    {"role": columnName, "title": columnName}));
            }

            populateTableMenu(tableView._tableMenu);
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
                    var sourceRowIndex = tableView.model.mapToSource(rowIndex);

                    if(sourceRowIndex >= 0)
                        rows.push(sourceRowIndex);
                });

                root.selectedRows = rows;
            }
        }

        onDoubleClicked:
        {
            var mappedRow = model.mapToSource(row);
            root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
        }

        // This is just a reference to the menu, so we can repopulate it later as necessary
        property Menu _tableMenu

        Component
        {
            id: columnSelectionHeaderDelegate

            StyleItem
            {
                elementType: "header"

                implicitWidth: checkbox.width + (2 * Constants.margin)
                implicitHeight: checkbox.height

                property int _clickedColumn

                property bool _pressed: styleData.pressed
                on_PressedChanged:
                {
                    // HACK: for some reason there are two styleDatas that get bound, one of which conveniently has
                    // a resizable property whereas the other doesn't, so we just ignore one on that basis
                    if(styleData.resizable === undefined)
                        return;

                    if(!styleData.pressed && _clickedColumn === styleData.column)
                    {
                        var columnShouldBeVisible = Utils.setContains(root.hiddenColumns, styleData.value);
                        root.setColumnVisibility(styleData.value, columnShouldBeVisible);
                    }
                    else
                        _clickedColumn = styleData.column;
                }

                SystemPalette { id: systemPalette }

                // Highlight the selected columns with the blended system highlight colour
                Rectangle
                {
                    anchors.fill: parent
                    visible: checkbox.visible && checkbox.checked

                    color:
                    {
                        var c = Qt.darker(systemPalette.highlight, 1.0);
                        return Qt.rgba(c.r, c.g, c.b, 0.4);
                    }
                }

                // Having a line under the header helps delineate the button controls
                Rectangle
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    height: 1
                    color: systemPalette.dark
                }

                Item
                {
                    anchors.leftMargin: Constants.margin
                    anchors.rightMargin: Constants.margin
                    anchors.fill: parent
                    clip: true

                    CheckBox
                    {
                        id: checkbox

                        anchors.verticalCenter: parent.verticalCenter
                        visible: styleData.value.length > 0
                        text: styleData.value

                        checked: { return !Utils.setContains(root.hiddenColumns, styleData.value); }
                    }
                }
            }
        }

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

                    if(column !== null && _nodeAttributesTableModel.columnIsFloatingPoint(column.role))
                        return Utils.formatForDisplay(styleData.value, 1);

                    return styleData.value;
                }

                color: styleData.textColor
                renderType: Text.NativeRendering
            }
        }
    }

    ShaderEffectSource
    {
        id: effectSource
        visible: columnSelectionMode

        x: tableView.contentItem.x + 1
        y: tableView.contentItem.y + tableView.__listView.headerItem.height + 1
        width: tableView.contentItem.width
        height: tableView.contentItem.height - tableView.__listView.headerItem.height

        sourceItem: tableView
        sourceRect: Qt.rect(x, y, width, height)
    }

    FastBlur
    {
        visible: columnSelectionMode
        anchors.fill: effectSource
        source: effectSource
        radius: 32
    }

    Item
    {
        clip: true

        anchors.fill: tableView
        anchors.topMargin: tableView.__listView.headerItem.height + 1

        SlidingPanel
        {
            id: columnSelectionControls
            visible: tableView.visible

            alignment: Qt.AlignTop|Qt.AlignLeft

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: -Constants.margin
            anchors.topMargin: -Constants.margin

            initiallyOpen: false
            disableItemWhenClosed: false

            item: Rectangle
            {
                width: row.width
                height: row.height

                border.color: "black"
                border.width: 1
                radius: 4
                color: "white"

                RowLayout
                {
                    id: row

                    // The RowLayout in a RowLayout is just a hack to get some padding
                    RowLayout
                    {
                        Layout.topMargin: Constants.padding + Constants.margin - 2
                        Layout.bottomMargin: Constants.padding
                        Layout.leftMargin: Constants.padding + Constants.margin - 2
                        Layout.rightMargin: Constants.padding

                        Button
                        {
                            text: qsTr("Show All")
                            onClicked: { root.showAllColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Hide All")
                            onClicked: { root.hideAllColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Show Calculated")
                            onClicked: { root.showAllCalculatedColumns(); }
                        }

                        Button
                        {
                            text: qsTr("Hide Calculated")
                            onClicked: { root.hideAllCalculatedColumns(); }
                        }

                        ToolButton
                        {
                            text: qsTr("Done")
                            iconName: "emblem-unreadable"
                            onClicked: { columnSelectionMode = false; }
                        }
                    }
                }
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
