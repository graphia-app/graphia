/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick 2.14
import QtQml 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import app.graphia 1.0

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

    onClicked:
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

    property bool showBorder: true

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

    property var _headerWidths: new Map()
    property var _cellWidths: new Map()

    readonly property int leftColumn: _leftLoadedColumn
    readonly property int rightColumn: _rightLoadedColumn
    readonly property int topRow: _topLoadedRow
    readonly property int bottomRow: _bottomLoadedRow

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
    }

    function _resetColumnWidths()
    {
        if(!root.model)
            return;

        root._columnWidths = new Array(root.model.columnCount()).fill(undefined);
    }

    onModelChanged:
    {
        root._resetColumnWidths();
        root._resetLoadedCells();
    }

    Connections
    {
        target: root.model

        // If the underlying data model has been reset, the column widths also need to be reset
        function onModelReset()
        {
            root._resetColumnWidths();
            root._resetLoadedCells();
        }
    }

    SystemPalette { id: systemPalette }

    property Component headerDelegate: Label
    {
        maximumLineCount: 1
        text: value

        background: Rectangle { color: systemPalette.light }
        color: systemPalette.text
        padding: root._padding
        renderType: Text.NativeRendering
    }

    onHeaderDelegateChanged:
    {
        root._resetColumnWidths();
        root.forceLayout();
    }

    property var cellValueProvider: function(value) { return value; }

    property Component cellDelegate: Label
    {
        maximumLineCount: 1
        text: { return root.cellValueProvider(value); }

        background: Rectangle { color: parent.backgroundColor }

        property var backgroundColor:
        {
            if(root.highlightedProvider(modelColumn, modelRow))
                return systemPalette.highlight;

            return modelRow % 2 ? systemPalette.window : systemPalette.alternateBase;
        }

        color: QmlUtils.contrastingColor(backgroundColor)
        leftPadding: root._padding
        rightPadding: root._padding
        topPadding: root._padding * 0.5
        bottomPadding: root._padding * 0.5
        elide: Text.ElideRight
        renderType: Text.NativeRendering
    }

    property int _cellDelegateHeight: 0

    border.width: root.showBorder ? 1 : 0
    border.color: systemPalette.dark

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
                    let headerWidth = root._headerWidths.get(column);
                    if(headerWidth === undefined)
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

                    onLoaded: { headerView.implicitHeight = height; }

                    onImplicitWidthChanged: { root._headerWidths.set(model.column, Math.max(1, implicitWidth)); }
                }

                // Column resize handle
                Rectangle
                {
                    anchors.right: parent.right
                    height: parent.height
                    width: 1
                    color: systemPalette.midlight

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

                        onDoubleClicked: { root.resizeColumnToContents(model.column); }
                    }
                }

                TableView.onReused:
                {
                    root._headerWidths.set(model.column, Math.max(1, headerDelegateLoader.implicitWidth));

                    if(typeof(headerDelegateLoader.item.onReused) === "function")
                        headerDelegateLoader.item.onReused();
                }

                TableView.onPooled:
                {
                    root._headerWidths.delete(model.column);

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
            color: systemPalette.midlight
        }

        TableView
        {
            id: tableView

            syncDirection: Qt.Horizontal
            syncView: headerView

            clip: true
            pixelAligned: true

            ScrollBar.vertical: ScrollBar {}
            ScrollBar.horizontal: ScrollBar {}
            boundsBehavior: Flickable.StopAtBounds

            Layout.fillHeight: true
            Layout.fillWidth: true
            anchors.margins: 1

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
                        if(implicitHeight !== 0)
                            root._cellDelegateHeight = Math.max(root._cellDelegateHeight, implicitHeight);
                    }

                    onImplicitWidthChanged: { root._cellWidths.set(model.column + "," + model.row, Math.max(1, implicitWidth)); }
                }

                MouseArea
                {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    onClicked: { root.clicked(model.column, model.row, mouse); }
                    onDoubleClicked: { root.doubleClicked(model.column, model.row, mouse); }
                }

                Component.onCompleted:
                {
                    root._loadedCells.add({x: model.column, y: model.row});
                    root._updateCellExtents();
                }

                TableView.onReused:
                {
                    root._loadedCells.add({x: model.column, y: model.row});
                    root._updateCellExtents();

                    root._cellWidths.set(model.column + "," + model.row, Math.max(1, cellDelegateLoader.implicitWidth));

                    if(typeof(cellDelegateLoader.item.onReused) === "function")
                        cellDelegateLoader.item.onReused();
                }

                TableView.onPooled:
                {
                    root._loadedCells.forEach((cell) =>
                    {
                        if(cell.x === model.column && cell.y === model.row)
                            root._loadedCells.delete(cell);
                    });

                    root._updateCellExtents();

                    root._cellWidths.delete(model.column + "," + model.row);

                    if(typeof(cellDelegateLoader.item.onPooled) === "function")
                        cellDelegateLoader.item.onPooled();
                }
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

    function resizeColumnToHeader(column)
    {
        root._columnWidths[column] = root._headerWidths.get(column);
        root.forceLayout();
    }

    function resizeColumnToContents(column)
    {
        let maxWidth = 0;
        for(let row = root.topRow; row <= root.bottomRow; row++)
        {
            let cellWidth = root._cellWidths.get(column + "," + row);
            maxWidth = Math.max(maxWidth, cellWidth);
        }

        if(isNaN(maxWidth))
        {
            console.log("DataTable.resizeColumnToContents: maxWidth calculated as NaN");
            return;
        }

        root._columnWidths[column] = Math.max(root._headerWidths.get(column), maxWidth);
        root.forceLayout();
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

    signal clicked(var column, var row, var mouse);
    signal doubleClicked(var column, var row, var mouse);
}

