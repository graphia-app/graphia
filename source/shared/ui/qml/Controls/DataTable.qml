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

    readonly property int _headerVerticalPadding: 4
    readonly property int _headerHorizontalPadding: 10
    readonly property int _minimumColumnWidth: 30
    readonly property int _minimumColumnHeight: 16

    SystemPalette { id: systemPalette }
    FontMetrics { id: fontMetrics }

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

            model: tableView.model
            height: fontMetrics.height + (2 * root._headerVerticalPadding)
            Layout.fillWidth: true
            interactive: false
            clip: true

            rowHeightProvider: function(row)
            {
                if(row === 0)
                    return -1;

                // Hide every other row
                return 0;
            }

            columnWidthProvider: tableView.columnWidthProvider

            delegate: Item
            {
                implicitWidth: headerLabel.contentWidth + root._headerHorizontalPadding
                implicitHeight: headerView.height

                id: headerDelegate
                Rectangle
                {
                    anchors.fill: parent
                    color: systemPalette.light
                }

                Label
                {
                    id: headerLabel
                    clip: true
                    maximumLineCount: 1
                    width: parent.width
                    text: modelData

                    color: systemPalette.text
                    padding: root._headerVerticalPadding
                    renderType: Text.NativeRendering
                }

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
                                let currentWidth = tableView.userColumnWidths[model.column];
                                if(currentWidth === undefined)
                                    currentWidth = headerDelegate.implicitWidth;

                                tableView.userColumnWidths[model.column] =
                                    Math.max(root._minimumColumnWidth, currentWidth + mouseX);
                                root.forceLayout();
                            }
                        }
                    }
                }

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
            property int delegateHeight: fontMetrics.height
            id: tableView
            syncDirection: Qt.Horizontal
            syncView: headerView

            clip: true
            ScrollBar.vertical: ScrollBar {}
            ScrollBar.horizontal: ScrollBar {}
            Layout.fillHeight: true
            Layout.fillWidth: true
            anchors.margins: 1

            rowHeightProvider: function(row)
            {
                if(row === 0)
                    return 0;

                return -1;
            }

            property var userColumnWidths: []

            columnWidthProvider: function(column)
            {
                let userWidth = userColumnWidths[column];
                if(userWidth !== undefined)
                    return userWidth;

                let headerIndex = tabularDataParser.model.index(0, column);
                let headerText = tabularDataParser.model.data(headerIndex);
                let textWidth = fontMetrics.advanceWidth(headerText);

                return textWidth + (2 * root._headerVerticalPadding);
            }

            delegate: Item
            {
                // Based on Qt source for BaseTableView delegate
                implicitHeight: Math.max(root._minimumColumnHeight, label.implicitHeight)
                onImplicitHeightChanged: { tableView.delegateHeight = implicitHeight; }

                implicitWidth: label.implicitWidth + 16
                visible: model.row > 0

                clip: true

                Rectangle
                {
                    width: parent.width

                    anchors.centerIn: parent
                    height: parent.height
                    color:
                    {
                        if(root.highlightedProvider(model.column, model.row))
                            return systemPalette.highlight;

                        return model.row % 2 ? systemPalette.window : systemPalette.alternateBase;
                    }

                    Label
                    {
                        id: label
                        elide: Text.ElideRight
                        width: parent.width

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10

                        color: QmlUtils.contrastingColor(parent.color)
                        renderType: Text.NativeRendering

                        text: modelData
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: { root.clicked(model.column, model.row); }
                    }
                }
            }
        }
    }

    function forceLayout()
    {
        tableView.forceLayout();
        headerView.forceLayout();
    }

    function cellIsVisible(column, row)
    {
        let firstDataColumnPosition = 0;
        for(let c = 0; c < column; c++)
            firstDataColumnPosition += tableView.columnWidthProvider(c);

        let firstDataRowPosition = 0;
        for(let r = 0; r < row; r++)
            firstDataRowPosition += tableView.delegateHeight;

        let x = firstDataColumnPosition - tableView.contentX;
        let y = (firstDataRowPosition - tableView.contentY) - tableView.delegateHeight;

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

    function scrollToCell(column, row)
    {

        let columnPosition = -tableView.topMargin;
        for(let c = 0; c < column - 1; c++)
            columnPosition += tableView.columnWidthProvider(c);

        let rowPosition = tableView.delegateHeight * ((row - 2) - 1);
        if(rowPosition < 0)
            rowPosition = -tableView.topMargin;

        scrollXAnimation.to = columnPosition;
        scrollXAnimation.running = true;
        scrollYAnimation.to = rowPosition;
        scrollYAnimation.running = true;
    }

    signal clicked(var column, var row);
}

