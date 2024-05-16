/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

import app.graphia
import app.graphia.Controls
import app.graphia.Utils

Rectangle
{
    id: root

    property alias model: tableView.model

    property bool useFirstRowAsHeader: true
    onUseFirstRowAsHeaderChanged:
    {
        //FIXME ideally this would reposition to show the first row when it was unhidden,
        // but it's really hard to get TableView to do this, at least it is with 5.14
        root.forceLayout();
    }

    property string cellDisplayRole: "display"

    property int sortIndicatorColumn: -1
    property int sortIndicatorOrder: Qt.AscendingOrder

    property bool showBorder: true

    color: palette.window

    enum DataTableSelectionMode
    {
        NoSelection,
        SingleSelection
        //TODO: multiple selection
    }

    property int selectionMode: DataTable.NoSelection
    property var selectedRows: []

    function selectRow(row) { root.selectedRows = [row]; }
    function clearSelection() { root.selectedRows = []; }

    onClicked: function(column, row, mouse)
    {
        if(mouse.button !== Qt.LeftButton)
            return;

        if(root.selectionMode !== DataTable.NoSelection)
            selectRow(row);
    }

    property var highlightedProvider: function(column, row)
    {
        if(root.selectionMode !== DataTable.NoSelection)
            return root.selectedRows.indexOf(row) !== -1;

        return false;
    }

    function rowIsVisible(row)
    {
        return row >= _topLoadedRow && row <= _bottomLoadedRow;
    }

    function positionViewAt(row)
    {
        if(rowIsVisible(row))
            return;

        tableView.contentY = Math.max(0, (row - 2) * _cellDelegateHeight);
    }

    readonly property int _padding: 4
    readonly property int _minimumColumnWidth: 32

    readonly property int headerHeight: headerView.height

    onWidthChanged: { root.forceLayout(); }
    onHeightChanged: { root.forceLayout(); }

    property var _columnWidths: []

    property var _loadedCells: new Set()
    property int _leftLoadedColumn: -1
    property int _rightLoadedColumn: -1
    property int _topLoadedRow: -1
    property int _bottomLoadedRow: -1

    function _resetLoadedCells()
    {
        root._loadedCells = new Set();
        root._leftLoadedColumn = -1;
        root._rightLoadedColumn = -1;
        root._topLoadedRow = -1;
        root._bottomLoadedRow = -1;
    }

    property var _headerItems: new Map()
    property var _cellWidths: new Map()

    readonly property int leftColumn: _leftLoadedColumn
    readonly property int rightColumn: _rightLoadedColumn
    readonly property int topRow: _topLoadedRow
    readonly property int bottomRow: _bottomLoadedRow

    signal cellExtentsChanged(int left, int right, int top, int bottom);

    function _updateCellExtents()
    {
        if(root._loadedCells.size === 0)
            return;

        let left = Number.MAX_SAFE_INTEGER;
        let right = Number.MIN_SAFE_INTEGER;
        let top = Number.MAX_SAFE_INTEGER;
        let bottom = Number.MIN_SAFE_INTEGER;

        root._loadedCells.forEach((cell) =>
        {
            left = Math.min(left, cell.x);
            right = Math.max(right, cell.x);
            top = Math.min(top, cell.y);
            bottom = Math.max(bottom, cell.y);
        });

        root._leftLoadedColumn = left;
        root._rightLoadedColumn = right;
        root._topLoadedRow = top;
        root._bottomLoadedRow = bottom;

        root.cellExtentsChanged(left, right, top, bottom);
    }

    function _resetColumnWidths()
    {
        if(!root.model)
            return;

        root._columnWidths = new Array(root.model.columnCount()).fill(undefined);
    }

    function _onModelChanged()
    {
        root._resetColumnWidths();
        root._resetLoadedCells();
    }

    onModelChanged: { _onModelChanged(); }

    Connections
    {
        target: root.model

        // If the underlying data model has been reset, the column widths also need to be reset
        function onModelReset() { _onModelChanged(); }
    }

    property Component headerDelegate: RowLayout
    {
        spacing: 0

        Label
        {
            id: headerLabel

            Layout.fillWidth: true

            maximumLineCount: 1
            text: value

            background: Rectangle { color: palette.button }
            color: palette.buttonText
            padding: root._padding
            elide: Text.ElideRight

            MouseArea
            {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                anchors.fill: parent
                onClicked: function(mouse) { root.headerClicked(modelColumn, mouse); }
                onDoubleClicked: function(mouse) { root.headerDoubleClicked(modelColumn, mouse); }
            }
        }

        Rectangle
        {
            Layout.fillHeight: true
            Layout.preferredWidth: sortIndicator.width + 4

            color: palette.button

            Shape
            {
                id: sortIndicator

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

                antialiasing: false
                width: 7
                height: 4
                visible: showSortIndicator

                transform: Rotation
                {
                    origin.x: sortIndicator.width * 0.5
                    origin.y: sortIndicator.height * 0.5
                    angle: sortIndicatorOrder === Qt.DescendingOrder ? 0 : 180
                }

                ShapePath
                {
                    miterLimit: 0
                    strokeColor: ControlColors.mid
                    fillColor: "transparent"
                    strokeWidth: 2
                    startY: sortIndicator.height - 1
                    PathLine { x: Math.round((sortIndicator.width - 1) * 0.5); y: 0 }
                    PathLine { x: sortIndicator.width - 1; y: sortIndicator.height - 1 }
                }
            }

            MouseArea
            {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                anchors.fill: parent
                onClicked: function(mouse) { root.headerClicked(modelColumn, mouse); }
                onDoubleClicked: function(mouse) { root.headerDoubleClicked(modelColumn, mouse); }
            }
        }
    }

    onHeaderDelegateChanged:
    {
        root._resetColumnWidths();
        root.forceLayout();
    }

    property var cellValueProvider: (value) => value

    property Component cellDelegate: Label
    {
        maximumLineCount: 1
        text: { return root.cellValueProvider(value); }

        background: Rectangle { color: parent.backgroundColor }

        property var backgroundColor:
        {
            if(root.highlightedProvider(modelColumn, modelRow))
                return palette.highlight;

            return modelRow % 2 ? ControlColors.tableRow1 : ControlColors.tableRow2;
        }

        color: { return NativeUtils.contrastingColor(backgroundColor); }
        leftPadding: root._padding
        rightPadding: root._padding
        topPadding: root._padding * 0.5
        bottomPadding: root._padding * 0.5
        elide: Text.ElideRight

        MouseArea
        {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            anchors.fill: parent
            onClicked: function(mouse) { root.clicked(modelColumn, modelRow, mouse); }
            onDoubleClicked: function(mouse) { root.doubleClicked(modelColumn, modelRow, mouse); }
        }
    }

    property int _cellDelegateHeight: 0

    border.width: root.showBorder ? 1 : 0
    border.color: ControlColors.outline

    ColumnLayout
    {
        anchors.margins: root.showBorder ? 1 : 0
        anchors.fill: parent
        spacing: 0

        TableView
        {
            id: headerView
            Layout.fillWidth: true

            model: tableView.model

            interactive: false
            clip: true
            pixelAligned: true

            rowHeightProvider: function(row)
            {
                if(row === 0)
                    return -1;

                // Hide every other row
                return 0;
            }

            columnWidthProvider: function(column)
            {
                if(root._columnWidths[column] === undefined)
                {
                    let headerWidth = root._headerWidth(column);
                    if(headerWidth === -1)
                        return -1;

                    root._columnWidths[column] = headerWidth;
                }

                return root._columnWidths[column];
            }

            delegate: Item
            {
                implicitWidth: Math.max(1, headerDelegateLoader.implicitWidth)
                implicitHeight: Math.max(1, headerDelegateLoader.implicitHeight)

                Loader
                {
                    id: headerDelegateLoader
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true

                    sourceComponent: root.headerDelegate
                    readonly property string value: model[root.cellDisplayRole]
                    readonly property int modelColumn: model.column
                    readonly property bool showSortIndicator: _forceSortIndicator || model.column === root.sortIndicatorColumn
                    readonly property int sortIndicatorOrder: root.sortIndicatorOrder

                    property bool _forceSortIndicator: false

                    onLoaded:
                    {
                        headerView.implicitHeight = Math.max(headerView.implicitHeight,
                            Math.max(item.implicitHeight, item.height));
                        root._headerItems.set(model.column, headerDelegateLoader);
                    }
                }

                // Column resize handle
                Rectangle
                {
                    anchors.right: parent.right
                    height: parent.height
                    width: 1
                    color: ControlColors.outline

                    MouseArea
                    {
                        cursorShape: Qt.SizeHorCursor
                        width: 4
                        height: parent.height
                        anchors.right: parent.right
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.threshold: 0

                        onMouseXChanged:
                        {
                            if(drag.active)
                            {
                                let currentColumnWidth = root._columnWidths[model.column];
                                if(currentColumnWidth === undefined)
                                    currentColumnWidth = parent.width;

                                root._columnWidths[model.column] =
                                    Math.max(root._minimumColumnWidth, currentColumnWidth + (mouseX - width));
                                root.forceLayout();
                            }
                        }

                        onDoubleClicked: function(mouse) { root.resizeColumnToContents(model.column); }
                    }
                }

                TableView.onReused:
                {
                    root._headerItems.set(model.column, headerDelegateLoader);

                    if(typeof(headerDelegateLoader.item.onReused) === "function")
                        headerDelegateLoader.item.onReused();
                }

                TableView.onPooled:
                {
                    root._headerItems.delete(model.column);

                    if(typeof(headerDelegateLoader.item.onPooled) === "function")
                        headerDelegateLoader.item.onPooled();
                }
            }
        }

        // Header underline
        Rectangle
        {
            Layout.fillWidth: true
            height: 1
            color: ControlColors.outline
        }

        Item
        {
            // This Item exists solely as a parent for the Flickable's vertical scrollbar
            // See https://doc.qt.io/qt-6/qml-qtquick-controls2-scrollbar.html#attaching-scrollbar-to-a-flickable

            Layout.fillHeight: true
            Layout.fillWidth: true

            TableView
            {
                id: tableView
                anchors.fill: parent

                syncDirection: Qt.Horizontal
                syncView: headerView

                clip: true
                pixelAligned: true

                ScrollBar.vertical: ScrollBar
                {
                    id: verticalTableViewScrollBar

                    parent: tableView.parent
                    anchors.top: tableView.top
                    anchors.bottom: tableView.bottom
                    anchors.bottomMargin: horizontalTableViewScrollBar.size < 1 ? horizontalTableViewScrollBar.height : 0
                    anchors.right: tableView.right
                }

                ScrollBar.horizontal: ScrollBar
                {
                    id: horizontalTableViewScrollBar

                    parent: tableView.parent
                    anchors.left: tableView.left
                    anchors.right: tableView.right
                    anchors.rightMargin: verticalTableViewScrollBar.size < 1 ? verticalTableViewScrollBar.width : 0
                    anchors.bottom: tableView.bottom
                }

                boundsBehavior: Flickable.StopAtBounds

                rowHeightProvider: function(row)
                {
                    // Hide first row
                    if(root.useFirstRowAsHeader && row === 0)
                        return 0;

                    // If the static height is set, use it, otherwise automatically size
                    return root._cellDelegateHeight ? root._cellDelegateHeight : -1;
                }

                columnWidthProvider: headerView.columnWidthProvider

                delegate: Item
                {
                    clip: true

                    implicitWidth: Math.max(1, cellDelegateLoader.implicitWidth)
                    implicitHeight: Math.max(1, cellDelegateLoader.implicitHeight)

                    Loader
                    {
                        id: cellDelegateLoader
                        anchors.fill: parent

                        sourceComponent: root.cellDelegate
                        readonly property string value: model[root.cellDisplayRole]
                        readonly property int modelColumn: model.column
                        readonly property int modelRow: model.row

                        onLoaded:
                        {
                            if(item.implicitHeight !== 0)
                                root._cellDelegateHeight = Math.max(root._cellDelegateHeight, item.implicitHeight);

                            root._cellWidths.set(model.column + "," + model.row, Math.max(1, item.implicitWidth));
                        }
                    }

                    Component.onCompleted:
                    {
                        root._loadedCells.add({x: model.column, y: model.row});
                        Qt.callLater(root._updateCellExtents);
                    }

                    TableView.onReused:
                    {
                        root._loadedCells.add({x: model.column, y: model.row});
                        root._cellWidths.set(model.column + "," + model.row, Math.max(1, cellDelegateLoader.item.implicitWidth));

                        if(typeof(cellDelegateLoader.item.onReused) === "function")
                            cellDelegateLoader.item.onReused();

                        Qt.callLater(root._updateCellExtents);
                    }

                    TableView.onPooled:
                    {
                        root._loadedCells.forEach((cell) =>
                        {
                            if(cell.x === model.column && cell.y === model.row)
                                root._loadedCells.delete(cell);
                        });

                        root._cellWidths.delete(model.column + "," + model.row);

                        if(typeof(cellDelegateLoader.item.onPooled) === "function")
                            cellDelegateLoader.item.onPooled();

                        Qt.callLater(root._updateCellExtents);
                    }
                }
            }

            ScrollBarCornerFiller
            {
                horizontalScrollBar: horizontalTableViewScrollBar
                verticalScrollBar: verticalTableViewScrollBar
            }
        }
    }

    function forceLayout()
    {
        headerView.forceLayout();
        tableView.forceLayout();
    }

    function cellIsVisible(column, row)
    {
        return column >= root._leftLoadedColumn && column <= root._rightLoadedColumn &&
            row >= root._topLoadedRow && row <= root._bottomLoadedRow;
    }

    function _headerWidth(column)
    {
        let loader = root._headerItems.get(column);
        if(loader === undefined)
            return -1;

        loader._forceSortIndicator = true;
        let width = loader.item.implicitWidth;
        loader._forceSortIndicator = false;

        return width;
    }

    function resizeColumnToHeader(column)
    {
        root._columnWidths[column] = root._headerWidth(column);
        root.forceLayout();
    }

    function resizeColumnToContents(column)
    {
        let maxCellWidth = 0;
        for(let row = root.topRow; row <= root.bottomRow; row++)
        {
            let cellWidth = root._cellWidths.get(column + "," + row);
            maxCellWidth = Math.max(maxCellWidth, cellWidth);
        }

        if(isNaN(maxCellWidth))
        {
            console.log("DataTable.resizeColumnToContents: maxWidth calculated as NaN");
            return;
        }

        let headerWidth = root._headerWidth(column);

        root._columnWidths[column] = Math.max(headerWidth, maxCellWidth);
        Qt.callLater(root.forceLayout);
    }

    function resizeVisibleColumnsToContents()
    {
        if(root.leftColumn < 0 || root.rightColumn < 0)
        {
            console.log("DataTable.resizeVisibleColumnsToContents: column extents not determined");
            return;
        }

        for(let column = root.rightColumn; column >= root.leftColumn; column--)
            resizeColumnToContents(column);
    }

    // These signals won't be emitted if the respective delegates are customised
    signal clicked(var column, var row, var mouse);
    signal doubleClicked(var column, var row, var mouse);
    signal headerClicked(var column, var mouse);
    signal headerDoubleClicked(var column, var mouse);
}
