import QtQuick.Controls 1.5
import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Private 1.0 // StyleItem
import QtGraphicalEffects 1.0
import QtQml.Models 2.13

import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0

import "Controls"
import "../../../shared/ui/qml/Utils.js" as Utils
import "../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    // External name
    property alias model: root._nodeAttributesTableModel
    property var defaultColumnWidth: 120

    // Internal name
    property var _nodeAttributesTableModel
    function resizeColumnsToContents()
    {
        tableView.currentColumnWidths = tableView.columnWidths;
        for(let i=0; i<tableView.columns; i++)
        {
            var storedWidth = tableView.currentColumnWidths[i];
            var columnHeader = columnHeaderRepeater.itemAt(i);
            columnHeader.width = Math.max(storedWidth, columnHeader.labelWidth)
        }
        tableView.forceLayoutSafe();
    }

    property var hiddenColumns: []
    onHiddenColumnsChanged: { tableView._updateColumnVisibility(); }

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
            columnSelectionControls.show();
        }
        else
        {
            columnSelectionControls.hide();
            _sortEnabled = true;
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

    SystemPalette { id: sysPalette }


    MouseArea
    {
        ItemSelectionModel
        {
            id: selectionModel
            model: proxyModel
        }
        property var previousRow: -1
        property var startRow: -1
        property var endRow: -1
        anchors.fill: parent
        anchors.rightMargin: verticalTableViewScrollBar.width
        anchors.bottomMargin: horizontalTableViewScrollBar.height
        z: 3
        hoverEnabled: true

        onDoubleClicked:
        {
            var tableItem = tableView.getItem(mouseX, mouseY);
            if(tableItem === false)
                return;
            var mappedRow = proxyModel.mapToSource(tableItem.modelRow);
            root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
        }

        onPressed:
        {
            var tableItem = tableView.getItem(mouseX, mouseY);
            if(tableItem === false)
                return;
            startRow = tableItem.modelRow;

            if(mouse.modifiers & Qt.ShiftModifier)
            {
                if(endRow != -1)
                {
                    selectRows(startRow, endRow);
                }
            }
            else if(mouse.modifiers & Qt.ControlModifier)
            {

            }
            else
            {
                selectionModel.clear();
            }

            endRow = tableItem.modelRow;
            selectRows(startRow, startRow);
            proxyModel.setSubSelection(selectionModel.selectedIndexes);
        }
        onReleased:
        {
            previousRow = -1;
        }
        onPositionChanged:
        {
            if(mouse.buttons == 1)
            {
                var tableItem = tableView.getItem(mouseX, mouseY);
                if(tableItem && tableItem.modelRow !== previousRow)
                {
                    if(previousRow != -1)
                        deselectRows(startRow, previousRow);

                    selectRows(startRow, tableItem.modelRow);

                    previousRow = tableItem.modelRow;
                    endRow = tableItem.modelRow
                }
            }
        }

        function selectRows(inStartRow, inEndRow)
        {
            let less = Math.min(inStartRow, inEndRow);
            let max = Math.max(inStartRow, inEndRow);

            var range = proxyModel.buildRowSelectionRange(less, max);
            selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Select)

            proxyModel.setSubSelection(selectionModel.selectedIndexes);
        }

        function deselectRows(inStartRow, inEndRow)
        {
            let less = Math.min(inStartRow, inEndRow);
            let max = Math.max(inStartRow, inEndRow);

            var range = proxyModel.buildRowSelectionRange(less, max);
            selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Deselect)

            proxyModel.setSubSelection(selectionModel.selectedIndexes);
        }
    }

    TableView
    {
        id: tableView
        property var currentColumnWidths: []
        property var columnWidths: []
        clip: true
        QQC2.ScrollBar.vertical: QQC2.ScrollBar
        {
            z: 100
            id: verticalTableViewScrollBar
            policy: Qt.ScrollBarAsNeeded
            contentItem: Rectangle
            {
                    implicitWidth: 5
                    radius: width / 2
                    color: sysPalette.dark
            }
        }

        function forceLayoutSafe()
        {
            if(tableView.rows > 0)
                tableView.forceLayout();
        }

        function getItem(mouseX, mouseY)
        {
            var tableViewContentContainsMouse = mouseY > columnsHeader.height && mouseY < tableView.height &&
                    mouseX < tableView.width && mouseX < tableView.contentWidth
                    && mouseY < tableView.contentHeight;

            if(!tableViewContentContainsMouse)
                return false;

            var hoverItem = tableView.childAt(mouseX, mouseY);
            if(hoverItem !== null && (hoverItem === horizontalTableViewScrollBar || hoverItem === verticalTableViewScrollBar))
                return false;

            var tableItem = hoverItem.childAt(
                        mouseX + tableView.contentX,
                        mouseY + tableView.contentY);
            return tableItem;
        }

        QQC2.ScrollBar.horizontal: QQC2.ScrollBar
        {
            id: horizontalTableViewScrollBar
        }
        model: TableProxyModel
        {
            id: proxyModel
            sourceModel: root._nodeAttributesTableModel
        }
        interactive: false

        visible: true
        anchors.fill: parent

        topMargin: columnsHeader.implicitHeight

        rowHeightProvider: function(row)
        {
            return -1;
        }

        columnWidthProvider: function(col)
        {
            var delegateWidth = tableView.currentColumnWidths[col];
            if(delegateWidth === undefined)
                return defaultColumnWidth;
            return Math.max(delegateWidth, columnHeaderRepeater.itemAt(col).labelWidth);
        }

        Row
        {
            id: columnsHeader
            y: tableView.contentY
            z: 2
            Repeater
            {
                id: columnHeaderRepeater
                model: tableView.columns > 0 ? tableView.columns : 1
                QQC2.Label
                {
                    property var labelWidth: contentWidth + padding + padding;
                    width: defaultColumnWidth
                    text: root._nodeAttributesTableModel.columnHeaders(modelData)
                    color: sysPalette.text
                    font.pixelSize: 11
                    padding: 2
                    background:  Rectangle { color: "white" }
                }
            }
        }

        // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
        // except for the text property
        delegate: Item
        {
            // Based on Qt source for BaseTableView delegate
            implicitHeight: Math.max(16, label.implicitHeight)
            implicitWidth: label.implicitWidth + 16
            clip: true

            TableView.onReused:
            {
                updateColumnWidths()
            }

            Component.onCompleted: updateColumnWidths()

            function updateColumnWidths()
            {
                var storedWidth = tableView.columnWidths[modelColumn];
                if(storedWidth !== undefined)
                    tableView.columnWidths[modelColumn] = Math.max(implicitWidth, storedWidth);
                else
                    tableView.columnWidths[modelColumn] = implicitWidth;
            }


            property var modelColumn: model.column
            property var modelRow: model.row
            property var modelIndex: model.index

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
                color:
                {
                    if(model.subSelected)
                    {
                        return systemPalette.highlight;
                    }
                    return model.row % 2 ? sysPalette.window : sysPalette.alternateBase;
                }

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
                proxyModel.invalidateFilter();
                verticalTableViewScrollBar.position = 0;
                tableView.columnWidths = new Array(tableView.columns).fill(0);
            }
        }

        Component.onCompleted:
        {
            tableView._resetSortFilterProxyModel();

            populateTableMenu(tableView._tableMenu);
        }

        //selectionMode: SelectionMode.ExtendedSelection

        // This is just a reference to the menu, so we can repopulate it later as necessary
        property Menu _tableMenu
    }

    ShaderEffectSource
    {
        id: effectSource
        visible: columnSelectionMode

        x: tableView.contentItem.x + 1
        y: tableView.contentItem.y + 10 + 1
        width: tableView.contentItem.width
        height: tableView.contentItem.height - 10

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
        anchors.topMargin: 10 + 1

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

                        Button
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

}
