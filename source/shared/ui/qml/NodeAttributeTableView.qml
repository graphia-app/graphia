/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick.Controls 1.5
import QtQuick 2.14
import QtQml 2.12
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Private 1.0 // StyleItem
import QtGraphicalEffects 1.0
import QtQml.Models 2.13
import QtQuick.Shapes 1.13

import Qt.labs.platform 1.0 as Labs

import app.graphia 1.0

import "Controls"
import "../../../shared/ui/qml/Utils.js" as Utils
import "../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    // External name
    property alias model: root._nodeAttributesTableModel
    property var defaultColumnWidth: 120
    property var selectedRows: []
    property alias rowCount: tableView.rows
    property alias sortIndicatorColumn: proxyModel.sortColumn
    property alias sortIndicatorOrder: proxyModel.sortOrder

    // Internal name
    property var _nodeAttributesTableModel

    function defaultColumnVisibility()
    {
        root._nodeAttributesTableModel.columnNames.forEach(function(columnName, index)
        {
            if(plugin.model.nodeAttributeTableModel.columnIsHiddenByDefault(columnName))
                setColumnVisibility(index, false);
        });

        tableView._updateColumnVisibility();
    }

    function resizeColumnsToContents()
    {
        // Resizing columns to contents is difficult with TableView2. The solution here
        // is to connect the tableview delegates to a signal so they report their implicitWidths
        // when the fetchColumnSizes signal is emitted. This will only fetch the sizes for the delegates
        // currently on screen. We store previous widths calculated in the currentColumnWidths array
        // and partially update them each time this function is called to best-guess offscreen columns
        tableView.userColumnWidths = [];
        tableView.fetchColumnSizes();
        if(tableView.columnWidths.length > 0)
        {
            // Update array
            for(let i = 0; i < tableView.columns; i++)
            {
                let newValue = tableView.columnWidths[i];
                if(newValue !== undefined)
                    tableView.currentColumnWidths[i] = newValue;
            }

            let tempArr = tableView.currentColumnWidths;
            tableView.currentColumnWidths = [];
            tableView.currentColumnWidths = tempArr;

            tableView.columnWidths = []
        }
        tableView.currentTotalColumnWidth = 0;
        for(let i = 0; i < tableView.columns; i++)
        {
            var tempCalculatedWidth = tableView.calculateMinimumColumnWidth(i);
            tableView.currentTotalColumnWidth += tempCalculatedWidth;
        }

        tableView.forceLayoutSafe();
    }

    property var hiddenColumns: []

    function setHiddenColumns(hiddenColumns)
    {
        root.hiddenColumns = hiddenColumns;
        tableView._updateColumnVisibility();
    }

    property bool columnSelectionMode: false
    onColumnSelectionModeChanged:
    {
        let indexArray = Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys());

        if(columnSelectionMode)
            columnSelectionControls.show();
        else
            columnSelectionControls.hide();

        // Reset the scroll position in case the new visible columns are no longer in view
        horizontalTableViewScrollBar.position = 0;

        tableView._updateColumnVisibility();
        tableView.forceLayoutSafe();
    }

    function setColumnVisibility(sourceColumnIndex, columnVisible)
    {
        if(columnVisible)
            hiddenColumns = Utils.setRemove(hiddenColumns, sourceColumnIndex);
        else
            hiddenColumns = Utils.setAdd(hiddenColumns, sourceColumnIndex);
    }

    function showAllColumns()
    {
        hiddenColumns = [];
    }

    function showAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        hiddenColumns = [];
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName, index)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setRemove(columns, index);
        });

        hiddenColumns = columns;
    }

    function hideAllColumns()
    {
        var columns = Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys());
        hiddenColumns = columns;
    }

    function hideAllCalculatedColumns()
    {
        var columns = hiddenColumns;
        hiddenColumns = [];
        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName, index)
        {
            if(root._nodeAttributesTableModel.columnIsCalculated(columnName))
                columns = Utils.setAdd(columns, index);
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
        menu.addItem("").action = selectAllTableAction;

        tableView._tableMenu = menu;
        Utils.cloneMenu(menu, contextMenu);
    }

    function selectAll()
    {
        selectRows(0, proxyModel.rowCount() - 1);
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
        title: qsTr("Export Table")
        nameFilters: ["CSV File (*.csv)", "TSV File (*.tsv)"]
        onAccepted:
        {
            misc.fileSaveInitialFolder = folder.toString();
            document.writeTableView2ToFile(tableView, file, defaultSuffix);
        }
    }

    property alias exportAction: exportTableAction

    Action
    {
        id: exportTableAction
        enabled: tableView.rows > 0
        text: qsTr("Export…")
        iconName: "document-save"
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                        misc.fileSaveInitialFolder : "";

            exportTableDialog.open();
        }
    }

    Action
    {
        id: selectAllTableAction
        text: qsTr("Select All")
        iconName: "edit-select-all"
        enabled: tableView.rows > 0

        onTriggered: { root.selectAll(); }
    }

    SystemPalette { id: sysPalette }

    function selectRows(inStartRow, inEndRow)
    {
        let less = Math.min(inStartRow, inEndRow);
        let max = Math.max(inStartRow, inEndRow);

        var range = proxyModel.buildRowSelectionRange(less, max);
        selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Select)

        root.selectedRows = selectionModel.selectedRows(0).map(index => proxyModel.mapToSourceRow(index.row));
    }

    function deselectRows(inStartRow, inEndRow)
    {
        let less = Math.min(inStartRow, inEndRow);
        let max = Math.max(inStartRow, inEndRow);

        var range = proxyModel.buildRowSelectionRange(less, max);
        selectionModel.select([range], ItemSelectionModel.Rows | ItemSelectionModel.Deselect)

        root.selectedRows = selectionModel.selectedRows(0).map(index => proxyModel.mapToSourceRow(index.row));
    }

    ItemSelectionModel
    {
        id: selectionModel
        model: proxyModel
        onSelectionChanged:
        {
            proxyModel.setSubSelection(selectionModel.selection, deselected);
        }
    }


    Item
    {
        clip: true

        anchors.fill: parent
        anchors.topMargin: headerView.height
        z: 10

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

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        Item
        {
            Layout.fillWidth: true
            height: headerMetrics.height + 6

            TableView
            {
                id: headerView
                model: proxyModel.headerModel
                width: parent.width
                height: parent.height
                interactive: false
                clip: true
                rowHeightProvider: function(row)
                {
                    return row > 0 ? 0 : -1;
                }
                columnWidthProvider: tableView.columnWidthProvider;
                visible: tableView.columns != 0
                boundsBehavior: Flickable.StopAtBounds

                onOriginXChanged:
                {
                    // Weird things can happen when the origin shifts, for some reason when the origin
                    // returns to 0 contentX can be left in a -ve position offsetting the content.
                    // This corrects that behaviour.
                    // (Reproduce by scrolling to far right, deselecting, selecting. Then scroll to far left slowly)
                    // Then select another node...)
                    contentX = originX + (horizontalTableViewScrollBar.position * contentWidth);
                }

                QQC2.ScrollBar.horizontal: QQC2.ScrollBar
                {
                    parent: horizontalScrollItem
                    z: 100
                    anchors.fill: parent
                    id: horizontalTableViewScrollBar
                    policy: QQC2.ScrollBar.AsNeeded
                    contentItem: Rectangle
                    {
                        implicitHeight: 5
                        radius: width / 2
                        color: sysPalette.dark
                    }
                    onPositionChanged:
                    {
                        // Sometimes syncViews don't actually sync visibleAreas
                        // however contentX and contentWidth are synced.
                        // The only reliable way to position both views is using
                        // the scrollbar position directly
                        if(position + size > 1)
                            position = 1 - size;
                        if(position < 0)
                            position = 0;
                    }

                    minimumSize: 0.1
                    visible: (size < 1.0 && tableView.columns > 0) || columnSelectionMode
                }

                property var sortIndicatorWidth: 7
                property var sortIndicatorMargin: 3
                property var delegatePadding: 4
                property var columnOrder: Array.from(new Array(_nodeAttributesTableModel.columnNames.length).keys());

                Rectangle
                {
                    height: headerView.height
                    width: headerView.width
                    color: sysPalette.light
                }

                delegate: DropArea
                {
                    id: headerItem
                    TableView.onReused:
                    {
                        refreshState();
                    }

                    function refreshState()
                    {
                        sourceColumn = Qt.binding(function() { return proxyModel.mapOrderedToSourceColumn(model.column) } );
                        implicitWidth =  Qt.binding(function() { return tableView.columnWidthProvider(model.column); });
                    }

                    implicitWidth: tableView.columnWidthProvider(model.column);
                    implicitHeight: headerLabel.height
                    property var modelColumn: model.column
                    property var sourceColumn: proxyModel.mapOrderedToSourceColumn(model.column);

                    Binding { target: headerContent; property: "sourceColumn"; value: sourceColumn }
                    Binding { target: headerContent; property: "modelColumn"; value: modelColumn }

                    Connections
                    {
                        target: proxyModel
                        function onColumnOrderChanged()
                        {
                            refreshState();
                        }
                    }

                    onEntered:
                    {
                        drag.source.target = proxyModel.mapOrderedToSourceColumn(model.column);
                        tableView.forceLayoutSafe();
                    }

                    Rectangle
                    {
                        anchors.fill: parent
                        visible: dragHandler.active
                        color: Qt.lighter(sysPalette.highlight, 1.99)
                    }

                    Item
                    {
                        id: headerContent
                        opacity: Drag.active ? 0.5 : 1
                        property int sourceColumn: 0
                        property int modelColumn: 0
                        property int target: -1
                        width: headerItem.implicitWidth
                        height: headerItem.implicitHeight
                        anchors.left: parent.left
                        anchors.top: parent.top
                        clip: true

                        states:
                        [
                            State
                            {
                                when: headerContent.Drag.active
                                ParentChange
                                {
                                    target: headerContent
                                    parent: headerView
                                }
                                AnchorChanges
                                {
                                    target: headerContent
                                    anchors.left: undefined
                                    anchors.top: undefined
                                }
                            }
                        ]

                        Rectangle
                        {
                            anchors.fill: parent
                            color: headerMouseArea.containsMouse ?
                                       Qt.lighter(sysPalette.highlight, 2.0) : sysPalette.light
                        }

                        Item
                        {
                            anchors.fill: parent
                            anchors.rightMargin: 5
                            anchors.leftMargin: 5

                            clip: true
                            CheckBox
                            {
                                anchors.verticalCenter: parent.verticalCenter

                                visible: columnSelectionMode
                                text: headerLabel.text
                                height: headerLabel.height

                                function isChecked()
                                {
                                    return !Utils.setContains(root.hiddenColumns, headerItem.sourceColumn);
                                }
                                checked: { return isChecked(); }
                                onCheckedChanged:
                                {
                                    // Unbind to prevent binding loop
                                    checked = checked;
                                    root.setColumnVisibility(headerItem.sourceColumn, checked);

                                    // Rebind so that the delegate doesn't hold the state
                                    checked = Qt.binding(isChecked);
                                }
                            }
                        }

                        QQC2.Label
                        {
                            id: headerLabel
                            visible: !columnSelectionMode
                            clip: true
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            width: parent.width - (headerView.sortIndicatorMargin + headerView.sortIndicatorWidth);
                            text:
                            {
                                let index = headerItem.sourceColumn;

                                if(index < 0 || index >= root._nodeAttributesTableModel.columnNames.length)
                                    return "";

                                return root._nodeAttributesTableModel.columnNames[index];
                            }
                            color: sysPalette.text
                            padding: headerView.delegatePadding
                            renderType: Text.NativeRendering
                        }

                        Shape
                        {
                            id: sortIndicator
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: headerView.sortIndicatorMargin
                            antialiasing: false
                            width: headerView.sortIndicatorWidth
                            height: headerView.delegatePadding
                            visible: proxyModel.sortColumn === headerItem.sourceColumn && !columnSelectionMode
                            transform: Rotation
                            {
                                origin.x: sortIndicator.width * 0.5
                                origin.y: sortIndicator.height * 0.5
                                angle: proxyModel.sortOrder === Qt.DescendingOrder ? 0 : 180
                            }

                            ShapePath
                            {
                                miterLimit: 0
                                strokeColor: sysPalette.mid
                                fillColor: "transparent"
                                strokeWidth: 2
                                startY: sortIndicator.height - 1
                                PathLine { x: Math.round((sortIndicator.width - 1) * 0.5); y: 0 }
                                PathLine { x: sortIndicator.width - 1; y: sortIndicator.height - 1 }
                            }
                        }

                        DragHandler
                        {
                            id: dragHandler
                            yAxis.enabled: false
                        }

                        Drag.active: dragHandler.active
                        Drag.source: headerContent
                        Drag.hotSpot.x: headerContent.width * 0.5
                        Drag.hotSpot.y: headerContent.height * 0.5
                        property bool dragActive: Drag.active
                        onDragActiveChanged:
                        {
                            if(!dragActive)
                            {
                                if(headerContent.target > -1)
                                {
                                    let currentIndex = headerView.columnOrder.indexOf(sourceColumn);
                                    let targetIndex = headerView.columnOrder.indexOf(headerContent.target);
                                    array_move(headerView.columnOrder, currentIndex, targetIndex);
                                    headerContent.target = -1;
                                }

                                proxyModel.columnOrder = headerView.columnOrder;
                                tableView.forceLayoutSafe();
                            }
                        }

                        MouseArea
                        {
                            id: headerMouseArea
                            enabled: !columnSelectionMode
                            anchors.fill: headerContent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton

                            onClicked:
                            {
                                if(proxyModel.sortColumn === headerItem.sourceColumn)
                                {
                                    proxyModel.sortOrder = proxyModel.sortOrder ?
                                        Qt.AscendingOrder : Qt.DescendingOrder;
                                }
                                else
                                    proxyModel.sortColumn = headerItem.sourceColumn;
                            }
                        }

                        Rectangle
                        {
                            anchors.right: parent.right
                            height: parent.height
                            width: 1
                            color: sysPalette.midlight
                            MouseArea
                            {
                                id: resizeHandleMouseArea
                                cursorShape: Qt.SizeHorCursor
                                width: 5
                                height: parent.height
                                anchors.horizontalCenter: parent.horizontalCenter
                                drag.target: parent
                                drag.axis: Drag.XAxis

                                onMouseXChanged:
                                {
                                    if(drag.active)
                                    {
                                        let sourceColumn = proxyModel.mapOrderedToSourceColumn(model.column);
                                        let userWidth = Math.max(30, headerItem.implicitWidth + mouseX);
                                        tableView.userColumnWidths[sourceColumn] = userWidth;
                                        headerItem.refreshState();
                                        tableView.forceLayoutSafe();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Item
        {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label
            {
                z: 3
                text: qsTr("No Visible Columns")
                visible: tableView.columns === 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            TableView
            {
                id: tableView
                anchors.fill: parent

                syncDirection: Qt.Horizontal
                syncView: headerView

                property var userColumnWidths: []
                property var currentColumnWidths: []
                property var currentTotalColumnWidth: 0
                property var columnWidths: []
                property var rowHeight: delegateMetrics.height + 1

                function visibleColumnNames() // Called from C++ when exporting table
                {
                    let columnNames = [];

                    for(let i = 0; i < tableView.columns; i++)
                    {
                        let sourceIndex = proxyModel.mapOrderedToSourceColumn(i);
                        columnNames.push(root._nodeAttributesTableModel.columnNames[sourceIndex])
                    }

                    return columnNames;
                }

                signal fetchColumnSizes;

                clip: true
                visible: tableView.columns != 0
                boundsBehavior: Flickable.StopAtBounds

                layer
                {
                    enabled: columnSelectionMode
                    effect: FastBlur
                    {
                        visible: columnSelectionMode
                        radius: 32
                    }
                }

                Canvas
                {
                    id: backgroundCanvas
                    width: tableView.width
                    height: tableView.height + (tableView.rowHeight * 2)
                    x: tableView.contentX
                    y: tableView.contentY - (tableView.contentY % (tableView.rowHeight * 2))
                    onPaint:
                    {
                        var ctx = getContext("2d");
                        for(let i = 0; i < Math.ceil(tableView.height / tableView.rowHeight) + 1; i++)
                        {
                            let yOffset = (i * tableView.rowHeight);
                            ctx.fillStyle = i % 2 ? sysPalette.window : sysPalette.alternateBase;
                            ctx.fillRect(0, yOffset, width, tableView.rowHeight);
                        }
                    }

                    Connections
                    {
                        target: tableView
                        function onContentYChanged()
                        {
                            backgroundCanvas.requestPaint();
                        }
                    }
                }

                QQC2.ScrollBar.vertical: QQC2.ScrollBar
                {
                    z: 100
                    id: verticalTableViewScrollBar
                    policy: QQC2.ScrollBar.AsNeeded
                    contentItem: Rectangle
                    {
                        implicitWidth: 5
                        radius: width / 2
                        color: sysPalette.dark
                    }
                    minimumSize: 0.1
                    visible: size < 1.0 && tableView.rows > 0
                }

                model: TableProxyModel
                {
                    id: proxyModel
                    sourceModel: root._nodeAttributesTableModel
                }

                columnWidthProvider: function(col)
                {
                    var calculatedWidth = 0;
                    var userWidth = userColumnWidths[proxyModel.mapOrderedToSourceColumn(col)];

                    // Use the user specified column width if available
                    if(userWidth !== undefined)
                        calculatedWidth = userWidth;
                    else
                        calculatedWidth = calculateMinimumColumnWidth(col);

                    return calculatedWidth;
                }

                function forceLayoutSafe()
                {
                    if(tableView.rows > 0 && tableView.columns > 0)
                        tableView.forceLayout();
                    if(headerView.rows > 0 && headerView.columns > 0)
                        headerView.forceLayout();
                }

                function getItem(mouseX, mouseY)
                {
                    var tableViewContentContainsMouse = mouseY > 0 && mouseY < tableView.height &&
                            mouseX < tableView.width && mouseX < tableView.contentWidth
                            && mouseY < tableView.contentHeight;

                    if(!tableViewContentContainsMouse)
                        return false;

                    var hoverItem = tableView.childAt(mouseX, mouseY);
                    if(hoverItem !== null && (hoverItem === horizontalTableViewScrollBar ||
                                              hoverItem === verticalTableViewScrollBar))
                    {
                        return false;
                    }
                    if(hoverItem === null)
                        return false;

                    var tableItem = hoverItem.childAt(
                                mouseX + tableView.contentX,
                                mouseY + tableView.contentY);
                    return tableItem;
                }

                function calculateMinimumColumnWidth(col)
                {
                    var delegateWidth = tableView.currentColumnWidths[col];
                    let headerActualWidth = headerFullWidth(col);
                    if(headerActualWidth === null)
                    {
                        console.log("Null CMCW", headerView.columns, col);
                        return defaultColumnWidth;
                    }

                    if(delegateWidth === undefined)
                        return Math.max(defaultColumnWidth, headerActualWidth);
                    else
                        return Math.max(delegateWidth, headerActualWidth);
                }

                function headerFullWidth(column)
                {
                    let sourceColumn = proxyModel.mapOrderedToSourceColumn(column);
                    let sortIndicatorSpacing = ((headerView.delegatePadding + headerView.sortIndicatorMargin) * 2.0) +
                        headerView.sortIndicatorWidth;

                    if(sourceColumn > -1)
                    {
                        let headerName = root._nodeAttributesTableModel.columnNameFor(sourceColumn);
                        let width = headerMetrics.advanceWidth(headerName);
                        width += sortIndicatorSpacing;
                        return width;
                    }

                    return sortIndicatorSpacing;
                }

                FontMetrics
                {
                    id: delegateMetrics
                }

                FontMetrics
                {
                    id: headerMetrics
                }

                // Ripped more or less verbatim from qtquickcontrols/src/controls/Styles/Desktop/TableViewStyle.qml
                // except for the text property
                delegate: Item
                {
                    // Based on Qt source for BaseTableView delegate
                    implicitHeight: tableView.rowHeight
                    implicitWidth: label.implicitWidth + 16

                    clip: false

                    property var modelColumn: model.column
                    property var modelRow: model.row

                    TableView.onReused:
                    {
                        tableView.fetchColumnSizes.connect(updateColumnWidths);
                    }

                    TableView.onPooled:
                    {
                        tableView.fetchColumnSizes.disconnect(updateColumnWidths)
                    }

                    Component.onCompleted:
                    {
                        tableView.fetchColumnSizes.connect(updateColumnWidths)
                    }

                    function updateColumnWidths()
                    {
                        if(typeof model === 'undefined')
                            return;
                        var storedWidth = tableView.columnWidths[modelColumn];
                        if(storedWidth !== undefined)
                            tableView.columnWidths[modelColumn] = Math.max(implicitWidth, storedWidth);
                        else
                            tableView.columnWidths[modelColumn] = implicitWidth;
                    }

                    SystemPalette { id: systemPalette }

                    Rectangle
                    {
                        width: tableView.width
                        height: parent.height
                        color: systemPalette.highlight;
                        visible: (model.column === (proxyModel.columnCount() - 1)) && model.subSelected
                    }

                    Rectangle
                    {
                        width: parent.width

                        anchors.centerIn: parent
                        height: parent.height
                        color:
                        {
                            if(model.subSelected)
                                return systemPalette.highlight;

                            return model.row % 2 ? sysPalette.window : sysPalette.alternateBase;
                        }

                        Text
                        {
                            id: label
                            objectName: "label"
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                            textFormat: Text.PlainText
                            width: parent.width
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            color: QmlUtils.contrastingColor(parent.color)

                            text:
                            {
                                let sourceColumn = proxyModel.mapOrderedToSourceColumn(modelColumn);

                                // This can happen during column removal
                                if(sourceColumn === undefined || sourceColumn < 0)
                                {
                                    console.log("Model Column Unable to map", modelRow, modelColumn);
                                    return "";
                                }

                                let columnName = root._nodeAttributesTableModel.columnNameFor(sourceColumn);

                                // AbstractItemModel required empty values to return empty variant
                                // but TableView2 delgates cast them to undefined js objects.
                                // It's difficult to tell if the model is corrupted or accessing
                                // invalid data now as they both return undefined.
                                if(model.display === undefined)
                                    return "";

                                if(_nodeAttributesTableModel.columnIsFloatingPoint(columnName))
                                    return QmlUtils.formatNumberScientific(model.display, 1);

                                // Replace newlines with spaces
                                if(typeof(model.display) === "string")
                                    return model.display.replace(/[\r\n]+/g, " ");

                                return model.display;
                            }
                            renderType: Text.NativeRendering
                        }
                    }
                }

                function _updateColumnVisibility()
                {
                    if(root.columnSelectionMode)
                        proxyModel.hiddenColumns = [];
                    else
                        proxyModel.hiddenColumns = hiddenColumns;
                }

                Connections
                {
                    target: plugin.model.nodeAttributeTableModel
                    function onSelectionChanged()
                    {
                        proxyModel.invalidateFilter();
                        selectRows(0, proxyModel.rowCount() - 1);
                        verticalTableViewScrollBar.position = 0;
                    }

                    function onColumnAdded(index, name)
                    {
                        // When a column is added at a particular index, all subsequent column
                        // indices in the model are incremented by 1, so our record of which
                        // columns are hidden must be adjusted to match...
                        root.hiddenColumns = root.hiddenColumns.map(v => v >= index ? v + 1 : v);
                        tableView._updateColumnVisibility();
                    }

                    function onColumnRemoved(index, name)
                    {
                        // (index might not be in hiddenColumns, but setRemove will ignore it in that case)
                        root.hiddenColumns = Utils.setRemove(root.hiddenColumns, index);

                        // ...the opposite is the case when removing
                        root.hiddenColumns = root.hiddenColumns.map(v => v >= index ? v - 1 : v);
                        tableView._updateColumnVisibility();
                    }
                }

                Component.onCompleted:
                {
                    populateTableMenu(tableView._tableMenu);

                    root.resizeColumnsToContents();
                    tableView._updateColumnVisibility();
                }

                // This is just a reference to the menu, so we can repopulate it later as necessary
                property Menu _tableMenu
            }

            MouseArea
            {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                property var previousRow: -1
                property var startRow: -1
                property var endRow: -1
                property var deselectDrag: false
                anchors.fill: parent
                anchors.rightMargin: verticalTableViewScrollBar.width
                z: 5
                hoverEnabled: true
                visible: !columnSelectionMode

                onDoubleClicked:
                {
                    var tableItem = tableView.getItem(mouseX, mouseY);
                    if(tableItem === false || !tableItem.hasOwnProperty('modelRow'))
                        return;
                    var mappedRow = proxyModel.mapToSourceRow(tableItem.modelRow);
                    root._nodeAttributesTableModel.moveFocusToNodeForRowIndex(mappedRow);
                }

                onClicked:
                {
                    if(mouse.button === Qt.RightButton)
                        root.rightClick();
                }

                onPressed:
                {
                    forceActiveFocus();
                    if(tableView.rows === 0)
                        return;

                    var tableItem = tableView.getItem(mouseX, mouseY);
                    if(tableItem === false || !tableItem.hasOwnProperty('modelRow'))
                        return;

                    startRow = tableItem.modelRow;

                    if(mouse.modifiers & Qt.ShiftModifier)
                    {
                        if(endRow != -1)
                            selectRows(startRow, endRow);
                    }
                    else if(!(mouse.modifiers & Qt.ControlModifier))
                        selectionModel.clear();

                    endRow = tableItem.modelRow;

                    // Deselect with ctrl-click
                    let modelIndex = proxyModel.index(startRow, 0);
                    if((mouse.modifiers & Qt.ControlModifier) &&
                            selectionModel.isSelected(modelIndex))
                    {
                        deselectRows(startRow, startRow);
                        deselectDrag = true;
                    }
                    else
                    {
                        selectRows(startRow, startRow);
                    }

                    previousRow = startRow;
                }

                Keys.onDownPressed:
                {
                    if(endRow != -1 && (endRow + 1) < tableView.rows)
                    {
                        endRow++;
                        arrowPress(event.modifiers);
                    }
                }
                Keys.onUpPressed:
                {
                    if(endRow != -1 && (endRow - 1) >= 0)
                    {
                        endRow--;
                        arrowPress(event.modifiers);
                    }
                }

                function arrowPress(modifier)
                {
                    // Horrible hack to scroll the view
                    let bottomTableItem = tableView.getItem(1, tableView.height - 1);
                    let topTableItem = tableView.getItem(1, 1);
                    let diff = 0;

                    if(bottomTableItem)
                        diff = Math.max(endRow - (bottomTableItem.modelRow - 1) , 0);
                    if(topTableItem)
                        diff += Math.min(endRow - (topTableItem.modelRow + 1), 0);
                    tableView.contentY += diff * tableView.rowHeight;

                    // Clamp scrollbar to prevent overscrolling
                    // (scrollbar seems to be the only safe way)
                    let scrollbarMax = 1.0 - verticalTableViewScrollBar.size;
                    verticalTableViewScrollBar.position = Math.min(Math.max(verticalTableViewScrollBar.position, 0), scrollbarMax);

                    selectionModel.clear();
                    if(modifier & Qt.ShiftModifier)
                    {
                        if(startRow == -1)
                            startRow = endRow;
                        selectRows(startRow, endRow);
                    }
                    else
                    {
                        startRow = endRow;
                        selectRows(endRow, endRow);
                    }
                }

                onReleased:
                {
                    previousRow = -1;
                    deselectDrag = false;
                }
                onPositionChanged:
                {
                    if(mouse.buttons !== Qt.LeftButton)
                        return;

                    var tableItem = tableView.getItem(mouseX, mouseY);
                    if(tableItem && tableItem.modelRow !== previousRow)
                    {
                        if(deselectDrag)
                        {
                            deselectRows(startRow, tableItem.modelRow);
                        }
                        else
                        {
                            if(previousRow != -1)
                                deselectRows(startRow, previousRow);

                            selectRows(startRow, tableItem.modelRow);
                        }

                        previousRow = tableItem.modelRow;
                        endRow = tableItem.modelRow
                    }
                }
            }
        }

        Item
        {
            id: horizontalScrollItem
            height: 9
            Layout.fillWidth: true
            visible: horizontalTableViewScrollBar.size != 1.0
        }
    }

    Menu { id: contextMenu }

    signal rightClick();
    onRightClick: { contextMenu.popup(); }

    function array_move(arr, old_index, new_index)
    {
        if (new_index >= arr.length)
        {
            var k = new_index - arr.length + 1;
            while(k--)
                arr.push(undefined);
        }
        arr.splice(new_index, 0, arr.splice(old_index, 1)[0]);
        return arr; // for testing
    }
}
