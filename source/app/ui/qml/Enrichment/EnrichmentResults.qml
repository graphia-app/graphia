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

import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import SortFilterProxyModel 0.2
import app.graphia 1.0

import Qt.labs.platform 1.0 as Labs

import "../Controls"

ApplicationWindow
{
    id: root

    property var models
    property var wizard

    property var currentTable: null
    property var currentHeatmap: null

    onXChanged: { if(x < 0 || x >= Screen.desktopAvailableWidth)  x = 0; }
    onYChanged: { if(y < 0 || y >= Screen.desktopAvailableHeight) y = 0; }

    function updateCurrent()
    {
        let tab = tabView.getTab(tabView.currentIndex);
        if(!tab)
            return;

        let item = tabView.getTab(tabView.currentIndex).item;
        if(!item)
            return;

        root.currentTable = item.childTable;
        root.currentHeatmap = item.childHeatmap;

        root.currentTable.resizeVisibleColumnsToContents();
    }

    onModelsChanged: { root.updateCurrent(); }
    onVisibleChanged: { if(visible) root.updateCurrent(); }

    title: qsTr("Enrichment Results")

    minimumWidth: 800
    minimumHeight: 400

    width: 1200
    height: 600

    MessageDialog
    {
        id: confirmDelete
        title: qsTr("Delete Enrichment Results?")
        text: qsTr("Are you sure you want to delete this enrichment result?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Yes | StandardButton.Cancel
        onYes:
        {
            root.removeResults(tabView.currentIndex);
        }
    }

    signal removeResults(int index)

    toolBar: ToolBar
    {
        RowLayout
        {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            ToolButton
            {
                id: showOnlyEnrichedButton
                iconName: "utilities-system-monitor"
                checkable: true
                checked: true
                tooltip: qsTr("Show only significant over-represented results")
            }
            ToolButton
            {
                id: showHeatmapButton
                iconName: "x-office-spreadsheet"
                checkable: true
                checked: true
                tooltip: qsTr("Show Heatmap")
            }
            ToolButton
            {
                iconName: "edit-delete"
                onClicked: confirmDelete.open();
                tooltip: qsTr("Delete result table")
            }
            ToolButton
            {
                id: addEnrichment
                iconName: "list-add"
                tooltip: qsTr("New Enrichment")
                onClicked: wizard.show()
            }
            ToolButton { action: exportTableAction }
            ToolButton { action: saveImageAction }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent

        Text
        {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("No Results")
            visible: tabView.count === 0
        }

        TabView
        {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: tabView.count > 0

            onCountChanged: { currentIndex = count - 1; }
            onCurrentIndexChanged: { root.updateCurrent(); }

            Repeater
            {
                model: root.models

                Tab
                {
                    id: tab
                    title: qsTr("Results") + " " + (index + 1)

                    SplitView
                    {
                        id: splitView

                        property alias childTable: table
                        property alias childHeatmap: heatmap

                        DataTable
                        {
                            id: table

                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumWidth: 100

                            selectionMode: DataTable.SingleSelection
                            showBorder: false

                            sortIndicatorColumn: 0
                            sortIndicatorOrder: Qt.AscendingOrder

                            property int rowCount: model ? model.rowCount() : 0

                            onHeaderClicked:
                            {
                                if(mouse.button !== Qt.LeftButton)
                                    return;

                                clearSelection();

                                if(sortIndicatorColumn === column)
                                {
                                    sortIndicatorOrder = sortIndicatorOrder === Qt.AscendingOrder ?
                                        Qt.DescendingOrder : Qt.AscendingOrder;

                                    return;
                                }

                                sortIndicatorColumn = column;
                                sortIndicatorOrder = Qt.AscendingOrder;
                            }

                            // For some reason QmlUtils isn't available from directly within the sort expression
                            property var stringCompare: QmlUtils.localeCompareStrings

                            onClicked:
                            {
                                if(mouse.button === Qt.RightButton)
                                    exportTableMenu.popup();
                            }

                            Text
                            {
                                anchors.centerIn: parent
                                text: qsTr("No Significant Results")
                                visible: table.rowCount <= 1 // rowCount includes header
                            }

                            model: SortFilterProxyModel
                            {
                                id: proxyModel
                                sourceModel: modelData

                                filters: ExpressionFilter
                                {
                                    enabled: showOnlyEnrichedButton.checked
                                    expression: model.enriched
                                }

                                sorters: ExpressionSorter
                                {
                                    enabled: table.sortIndicatorColumn >= 0
                                    expression:
                                    {
                                        let descending = table.sortIndicatorOrder === Qt.DescendingOrder;

                                        if(table.sortIndicatorColumn < 0)
                                            return true;

                                        let rowA = modelLeft.index;
                                        let rowB = modelRight.index;

                                        if(rowA < 0 || rowB < 0)
                                            return true;

                                        // Exclude header row from sort
                                        if(rowA === 0 || rowB === 0)
                                            return rowA === 0;

                                        let valueA = proxyModel.sourceModel.data(proxyModel.sourceModel.index(rowA, table.sortIndicatorColumn));
                                        let valueB = proxyModel.sourceModel.data(proxyModel.sourceModel.index(rowB, table.sortIndicatorColumn));

                                        if(descending)
                                            [valueA, valueB] = [valueB, valueA];

                                        if(!isNaN(valueA) && !isNaN(valueB))
                                            return Number(valueA) < Number(valueB);

                                        return table.stringCompare(valueA, valueB) < 0;
                                    }
                                }
                            }

                            cellValueProvider: function(value)
                            {
                                if(!isNaN(value) && value.length > 0)
                                    return QmlUtils.formatNumberScientific(value, 1);

                                return value;
                            }
                        }

                        GridLayout
                        {
                            columns: 2
                            Layout.fillHeight: true
                            visible: showHeatmapButton.checked

                            EnrichmentHeatmap
                            {
                                id: heatmap
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.minimumHeight: 200
                                Layout.minimumWidth: 170
                                Layout.preferredWidth: (tab.width * 0.5) - 5
                                model: modelData
                                elideLabelWidth: 100
                                showOnlyEnriched: showOnlyEnrichedButton.checked
                                property bool horizontalScrollBarRequired: (heatmap.width / heatmap.horizontalRangeSize) > scrollView.viewport.width;
                                property bool verticalScrollBarRequired: (heatmap.height / heatmap.verticalRangeSize) > scrollView.viewport.height;
                                xAxisPadding: horizontalScrollBarRequired ? 20 : 0
                                yAxisPadding: verticalScrollBarRequired ? 20 : 0
                                xAxisLabel: modelData.selectionA
                                yAxisLabel: modelData.selectionB

                                onShowOnlyEnrichedChanged: { table.clearSelection(); }

                                onPlotValueClicked:
                                {
                                    let proxyRow = proxyModel.mapFromSource(row);
                                    if(proxyRow < 0)
                                    {
                                        table.clearSelection();
                                        return;
                                    }

                                    table.selectRow(proxyRow);
                                    //FIXME scroll into view/indicate selection some way
                                }

                                scrollXAmount:
                                {
                                    return scrollView.flickableItem.contentX /
                                            (scrollView.flickableItem.contentWidth - scrollView.viewport.width);
                                }

                                scrollYAmount:
                                {
                                    return scrollView.flickableItem.contentY /
                                            (scrollView.flickableItem.contentHeight - scrollView.viewport.height);
                                }

                                onRightClick: { plotContextMenu.popup(); }

                                ScrollView
                                {
                                    id: scrollView
                                    visible: heatmap.horizontalScrollBarRequired || heatmap.verticalScrollBarRequired
                                    anchors.fill: parent
                                    Item
                                    {
                                        // This is a fake item to make native scrollbars appear
                                        // Prevent Qt opengl texture overflow (2^14 pixels)
                                        width: Math.min(heatmap.width / heatmap.horizontalRangeSize, 16383)
                                        height: Math.min(heatmap.height / heatmap.verticalRangeSize, 16383)
                                    }
                                }
                            }
                        }

                        Connections
                        {
                            target: modelData
                            function onModelReset() { heatmap.buildPlot(); }
                        }
                    }
                }
            }
        }
    }

    Menu
    {
        id: plotContextMenu
        MenuItem { action: saveImageAction }
    }

    Action
    {
        id: saveImageAction
        enabled: root.currentHeatmap
        text: qsTr("Save As Image…")
        iconName: "camera-photo"
        onTriggered:
        {
            heatmapSaveDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                misc.fileSaveInitialFolder : "";

            heatmapSaveDialog.open();
        }
    }

    Menu
    {
        id: exportTableMenu
        MenuItem { action: exportTableAction }
    }

    Action
    {
        id: exportTableAction
        enabled: root.currentTable && root.currentTable.rowCount > 1 // rowCount includes header
        text: qsTr("Export Table…")
        iconName: "document-save"
        onTriggered:
        {
            exportTableDialog.folder = misc.fileSaveInitialFolder !== undefined ?
                misc.fileSaveInitialFolder : "";

            exportTableDialog.open();
        }
    }

    Labs.FileDialog
    {
        id: heatmapSaveDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 1
        title: qsTr("Save Plot As Image")
        nameFilters: [ "PDF Document (*.pdf)", "PNG Image (*.png)", "JPEG Image (*.jpg *.jpeg)" ]
        onAccepted:
        {
            currentHeatmap.savePlotImage(file, selectedNameFilter.extensions);
        }
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
            wizard.document.writeTableModelToFile(
                root.currentTable.model, file, defaultSuffix);
        }
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }
}
