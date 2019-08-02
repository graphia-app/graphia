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

    // Internal name
    property var _nodeAttributesTableModel
    property var autoColumnWidth: false
    onAutoColumnWidthChanged:
    {
        tableView.forceLayout();
        tableView.forceLayout();
    }

    property var hiddenColumns: []
    onHiddenColumnsChanged: { tableView._updateColumnVisibility(); }

    property var selectedRows: []
    function selectRow(row)
    {
        selectionModel.select(proxyModel.index(row, 0), ItemSelectionModel.Rows | ItemSelectionModel.Select)
    }
    function unselect(row)
    {
        selectionModel.select(proxyModel.index(row, 0), ItemSelectionModel.Rows | ItemSelectionModel.Deselect)
    }

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
            var child = tableView.childAt(mouseX, mouseY).childAt(
                        mouseX + tableView.contentX,
                        mouseY + tableView.contentY);
            var mappedRow = proxyModel.mapToSource(child.modelRow);
            root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
        }

        onPressed:
        {
            if(mouse.modifiers & Qt.ControlModifier)
            {

            }
            else
            {
                selectionModel.clear();
            }
            var child = tableView.childAt(mouseX, mouseY).childAt(
                        mouseX + tableView.contentX,
                        mouseY + tableView.contentY);
            startRow = child.modelRow;
            selectRow(startRow);
            proxyModel.setSubSelection(selectionModel.selectedIndexes);
        }
        onReleased:
        {
            previousRow = -1;
        }
        onPositionChanged:
        {
            if(mouse.buttons == 1 && mouseY > 0)
            {
                var child = tableView.childAt(mouseX, mouseY).childAt(
                            mouseX + tableView.contentX,
                            mouseY + tableView.contentY);
                if(child.modelRow !== previousRow)
                {
                    if(previousRow != -1)
                    {
                        let less = Math.min(startRow, previousRow);
                        let max = Math.max(startRow, previousRow);
                        for(let i = less; i < max + 1; i++)
                            unselect(i);
                    }
                    let less = Math.min(startRow, child.modelRow);
                    let max = Math.max(startRow, child.modelRow);
                    for(let i = less; i < max + 1; i++)
                        selectRow(i);
                    previousRow = child.modelRow;
                    proxyModel.setSubSelection(selectionModel.selectedIndexes);
                }
            }
        }
    }
    ItemSelectionModel
    {
        id: selectionModel
        model: proxyModel
    }
    TableView
    {
        id: tableView
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
            return root.autoColumnWidth ? -1 : 120;
        }

        Row {
            id: columnsHeader
            y: tableView.contentY
            z: 2
            Repeater {
                model: tableView.columns > 0 ? tableView.columns : 1
                QQC2.Label {
                    width: tableView.columnWidthProvider(modelData)
                    height: 35
                    text: root._nodeAttributesTableModel.columnHeaders(modelData)
                    color: '#aaaaaa'
                    font.pixelSize: 15
                    padding: 10
                    verticalAlignment: Text.AlignVCenter
                    background:  Rectangle { color: "#333333" }
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
                    return model.row % 2 ? "white" : "lightgrey";
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
