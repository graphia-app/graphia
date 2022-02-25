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

    property var highlightedProvider: function(column, row) { return false; }

    readonly property int _padding: 4
    readonly property int _minimumColumnWidth: 32

    readonly property int headerHeight: headerView.height

    onWidthChanged: { root.forceLayout(); }
    onHeightChanged: { root.forceLayout(); }

    property var _naturalHeaderWidths: []
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

    property Component cellDelegate: Label
    {
        maximumLineCount: 1
        text: value

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

    border.width: 1
    border.color: systemPalette.dark

    ColumnLayout
    {
        anchors.margins: 1
        anchors.fill: parent
        spacing: 0

        TableView
        {
            id: headerView
            Layout.fillWidth: true

            syncDirection: Qt.Horizontal
            syncView: tableView
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
                    let headerIndex = root.model.index(0, column);
                    let headerText = root.model.data(headerIndex);

                    // Create a header so we can determine the default width for headerText
                    let dummyHeader = root.headerDelegate.createObject(null, {text: headerText});
                    root._columnWidths[column] = Math.max(root._minimumColumnWidth, dummyHeader.implicitWidth);
                    root._naturalHeaderWidths[column] = root._columnWidths[column];
                    dummyHeader.destroy();
                }

                return root._columnWidths[column];
            }

            delegate: Item
            {
                implicitWidth: headerDelegateLoader.width
                implicitHeight: headerDelegateLoader.height

                Loader
                {
                    id: headerDelegateLoader
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true

                    sourceComponent: root.headerDelegate
                    readonly property string value: modelData
                    readonly property int modelColumn: model.column

                    onLoaded: { headerView.implicitHeight = height; }
                }

                // Pass through hover events to the header to immediately to the left
                HoverMousePassthrough { anchors.fill: parent }

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
                        width: 8
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter
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

                implicitWidth: cellDelegateLoader.width
                implicitHeight: Math.max(1, cellDelegateLoader.height)

                Loader
                {
                    id: cellDelegateLoader
                    anchors.left: parent.left
                    anchors.right: parent.right

                    sourceComponent: root.cellDelegate
                    readonly property string value: modelData
                    readonly property int modelColumn: model.column
                    readonly property int modelRow: model.row

                    onLoaded:
                    {
                        if(root._cellDelegateHeight === 0 && height !== 0)
                            root._cellDelegateHeight = height;
                    }

                    onImplicitWidthChanged: { root._cellWidths.set(model.column + "," + model.row, implicitWidth); }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: { root.clicked(model.column, model.row); }
                    onDoubleClicked: { root.doubleClicked(model.column, model.row); }
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

                    root._cellWidths.set(model.column + "," + model.row, cellDelegateLoader.implicitWidth);
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

        root._columnWidths[column] = Math.max(root._naturalHeaderWidths[column], maxWidth);
        root.forceLayout();
    }

    function resizeVisibleColumnsToContents()
    {
        for(let column = root.rightColumn; column >= root.leftColumn; column--)
            resizeColumnToContents(column);
    }

    signal clicked(var column, var row);
    signal doubleClicked(var column, var row);
}

