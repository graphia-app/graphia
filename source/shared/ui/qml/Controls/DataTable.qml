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

    property var highlightedProvider: function(column, row) { return false; }

    readonly property int _padding: 4
    readonly property int _minimumColumnWidth: 32

    onWidthChanged: { root.forceLayout(); }
    onHeightChanged: { root.forceLayout(); }

    property var _columnWidths: []

    function resetColumnWidths()
    {
        if(!root.model)
            return;

        root._columnWidths = new Array(root.model.columnCount()).fill(undefined);
    }

    onModelChanged:
    {
        root.resetColumnWidths();
    }

    Connections
    {
        target: root.model

        // If the underlying data model has been reset, the column widths also need to be reset
        function onModelReset() { root.resetColumnWidths(); }
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
        root.resetColumnWidths();
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
                    dummyHeader.destroy();
                }

                return root._columnWidths[column];
            }

            delegate: Item
            {
                clip: true

                implicitWidth: headerDelegateLoader.width
                implicitHeight: headerDelegateLoader.height

                Loader
                {
                    id: headerDelegateLoader
                    anchors.left: parent.left
                    anchors.right: parent.right

                    sourceComponent: root.headerDelegate
                    readonly property string value: modelData
                    readonly property int modelColumn: model.column

                    onLoaded: { headerView.implicitHeight = height; }
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
                        width: 5
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter
                        drag.target: parent
                        drag.axis: Drag.XAxis

                        onMouseXChanged:
                        {
                            if(drag.active)
                            {
                                let currentWidth = root._columnWidths[model.column];
                                if(currentWidth === undefined)
                                    currentWidth = parent.width;

                                root._columnWidths[model.column] =
                                    Math.max(root._minimumColumnWidth, currentWidth + mouseX);
                                root.forceLayout();
                            }
                        }
                    }
                }

                // Header underline
                Rectangle
                {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: systemPalette.midlight
                }
            }
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
                if(row === 0)
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
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: { root.clicked(model.column, model.row); }
                    onDoubleClicked: { root.doubleClicked(model.column, model.row); }
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
        let cellX = 0;
        for(let c = 0; c < column; c++)
            cellX += headerView.columnWidthProvider(c);

        let cellY = 0;
        for(let r = 0; r < row; r++)
            cellY += root._cellDelegateHeight;

        let x = cellX - (tableView.contentX - tableView.originX);
        let y = (cellY - (tableView.contentY - tableView.originY)) - root._cellDelegateHeight;

        return x >= 0.0 && x < tableView.width && y >= 0.0 && y < tableView.height;
    }

    PropertyAnimation
    {
        id: scrollXAnimation
        target: tableView
        property: "contentX"
        duration: 750
        easing.type: Easing.OutQuad
    }

    PropertyAnimation
    {
        id: scrollYAnimation
        target: tableView
        property: "contentY"
        duration: 750
        easing.type: Easing.OutQuad
    }

    function scrollToCell(targetColumn, targetRow)
    {
        // Goto the preceding column
        targetColumn = Math.max(0, targetColumn - 1);

        let columnPosition = tableView.originX;
        for(let c = 0; c < targetColumn; c++)
            columnPosition += headerView.columnWidthProvider(c);

        // Account for input being in whole table coordinates (i.e. including header row)
        targetRow -= 1;

        // Goto the preceding column
        targetRow = Math.max(0, targetRow - 1);

        let rowPosition = tableView.originY + (root._cellDelegateHeight * targetRow);

        scrollXAnimation.to = columnPosition;
        scrollXAnimation.restart();
        scrollYAnimation.to = rowPosition;
        scrollYAnimation.restart();
    }

    signal clicked(var column, var row);
    signal doubleClicked(var column, var row);
}

