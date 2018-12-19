import QtQuick.Controls 1.5
import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.4 as QQC2
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
            //tableView.headerDelegate = columnSelectionHeaderDelegate
            columnSelectionControls.show();
        }
        else
        {
            columnSelectionControls.hide();

            // The column selection mode will probably have changed these values, so set them
            // back to what they were before
            //tableView.sortIndicatorColumn = root.sortIndicatorColumn;
            //tableView.sortIndicatorOrder = root.sortIndicatorOrder;
            _sortEnabled = true;
            //tableView.flickableItem.contentX = 0;

            //tableView.headerDelegate = defaultTableView.headerDelegate;
        }

        tableView._updateColumnVisibility();
    }

    property bool _sortEnabled: true

    property int sortIndicatorColumn
    onSortIndicatorColumnChanged:
    {
        //tableView.sortIndicatorColumn = root.sortIndicatorColumn;
    }

    property int sortIndicatorOrder
    onSortIndicatorOrderChanged:
    {
        //tableView.sortIndicatorOrder = root.sortIndicatorOrder;
    }

    //property alias selection: tableView.selection
    property alias rowCount: tableView.rows
    property alias viewport: tableView.childrenRect

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
        menu.addItem("").action = selectColumnsAction;
        menu.addItem("").action = exportTableAction;
        menu.addSeparator();
        menu.addItem("").action = selectAllAction;

        tableView._tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);
    }

//    Label
//    {
//        text: qsTr("No Visible Columns")
//        visible: tableView.numVisibleColumns <= 0

//        anchors.horizontalCenter: parent.horizontalCenter
//        anchors.verticalCenter: parent.verticalCenter
//    }

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
        title: qsTr("Export Table")
        nameFilters: ["CSV File (*.csv)", "TSV File (*.tsv)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            document.writeTableViewToFile(tableView, file, defaultSuffix);
        }
    }

    property alias exportAction: exportTableAction

    Action
    {
        id: exportTableAction
        enabled: tableView.rowCount > 0
        text: qsTr("Export…")
        iconName: "document-save"
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                misc.fileSaveInitialFolder : "";

            exportTableDialog.open();
        }
    }

    TableView
    {
        id: tableView
        clip: true
        QQC2.ScrollBar.vertical: QQC2.ScrollBar { }
        QQC2.ScrollBar.horizontal: QQC2.ScrollBar { }
        model: root._nodeAttributesTableModel

        visible: true
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1

        rowHeightProvider: function(row)
        {
            return -1;
            if(row === 0)
                return -1;
            return !tableView.model.rowVisible(row) ? 0 : -1;
        }

        // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
        // except for the text property
        delegate: Item
        {
            // Based on Qt source for BaseTableView delegate
            implicitHeight: Math.max(16, label.implicitHeight)
            implicitWidth: label.implicitWidth + 16
            clip: true

            property var modelColumn: model.column

            SystemPalette { id: systemPalette }

            Rectangle
            {
                Rectangle
                {
                    anchors.right: parent.right
                    height: parent.height
                    width: 1
                    color: systemPalette.light
                }

                width: parent.width
                anchors.centerIn: parent
                height: parent.height
                color: "transparent"

                Text
                {
                    id: label
                    objectName: "label"
                    width: parent.width
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text:
                    {
                        return model.display;
                    }
                    renderType: Text.NativeRendering
                }
            }
        }

        Connections
        {
            target: plugin.model.nodeAttributeTableModel
            onSelectionChanged:
            {
                tableView.forceLayout();
            }
        }

        Component.onCompleted:
        {
            tableView._resetSortFilterProxyModel();

            populateTableMenu(tableView._tableMenu);
        }

        //selectionMode: SelectionMode.ExtendedSelection

//        onDoubleClicked:
//        {
//            var mappedRow = model.mapToSource(row);
//            root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
//        }

        // This is just a reference to the menu, so we can repopulate it later as necessary
        property Menu _tableMenu
    }

//    ShaderEffectSource
//    {
//        id: effectSource
//        visible: columnSelectionMode

//        x: tableView.contentItem.x + 1
//        y: tableView.contentItem.y + 10 + 1
//        width: tableView.contentItem.width
//        height: tableView.contentItem.height - 10

//        sourceItem: tableView
//        sourceRect: Qt.rect(x, y, width, height)
//    }

//    FastBlur
//    {
//        visible: columnSelectionMode
//        anchors.fill: effectSource
//        source: effectSource
//        radius: 32
//    }

//    Item
//    {
//        clip: true

//        anchors.fill: tableView
//        anchors.topMargin: 10 + 1

//        SlidingPanel
//        {
//            id: columnSelectionControls
//            visible: tableView.visible

//            alignment: Qt.AlignTop|Qt.AlignLeft

//            anchors.left: parent.left
//            anchors.top: parent.top
//            anchors.leftMargin: -Constants.margin
//            anchors.topMargin: -Constants.margin

//            initiallyOpen: false
//            disableItemWhenClosed: false

//            item: Rectangle
//            {
//                width: row.width
//                height: row.height

//                border.color: "black"
//                border.width: 1
//                radius: 4
//                color: "white"

//                RowLayout
//                {
//                    id: row

//                    // The RowLayout in a RowLayout is just a hack to get some padding
//                    RowLayout
//                    {
//                        Layout.topMargin: Constants.padding + Constants.margin - 2
//                        Layout.bottomMargin: Constants.padding
//                        Layout.leftMargin: Constants.padding + Constants.margin - 2
//                        Layout.rightMargin: Constants.padding

//                        Button
//                        {
//                            text: qsTr("Show All")
//                            onClicked: { root.showAllColumns(); }
//                        }

//                        Button
//                        {
//                            text: qsTr("Hide All")
//                            onClicked: { root.hideAllColumns(); }
//                        }

//                        Button
//                        {
//                            text: qsTr("Show Calculated")
//                            onClicked: { root.showAllCalculatedColumns(); }
//                        }

//                        Button
//                        {
//                            text: qsTr("Hide Calculated")
//                            onClicked: { root.hideAllCalculatedColumns(); }
//                        }

//                        Button
//                        {
//                            text: qsTr("Done")
//                            iconName: "emblem-unreadable"
//                            onClicked: { columnSelectionMode = false; }
//                        }
//                    }
//                }
//            }
//        }
//    }

    Menu { id: contextMenu }

    signal rightClick();
    onRightClick: { contextMenu.popup(); }

}
