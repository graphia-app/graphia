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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import Graphia.Controls
import Graphia.SharedTypes
import Graphia.Utils

import Qt.labs.platform as Labs

ApplicationWindow
{
    id: root

    property var models
    property var wizard

    property var _currentModel:
    {
        let m = models[tabBar.currentIndex];
        return m ? m : null;
    }

    property bool _resultsAvailable: root.models.length > 0

    onXChanged: { if(x < 0 || x >= Screen.desktopAvailableWidth)  x = 0; }
    onYChanged: { if(y < 0 || y >= Screen.desktopAvailableHeight) y = 0; }

    title: qsTr("Enrichment Results")
    color: palette.window

    minimumWidth: 800
    minimumHeight: 400

    width: 1200
    height: 600

    Labs.MessageDialog
    {
        id: confirmDelete
        title: qsTr("Delete Enrichment Results?")
        text: qsTr("Are you sure you want to delete this enrichment result?")
        buttons: Labs.MessageDialog.Yes | Labs.MessageDialog.Cancel
        modality: Qt.ApplicationModal

        onYesClicked:
        {
            root.removeResults(tabBar.currentIndex);
        }
    }

    signal removeResults(int index)

    header: ToolBar
    {
        topPadding: Constants.padding
        bottomPadding: Constants.padding

        RowLayout
        {
            ToolBarButton
            {
                id: showOnlyEnrichedButton
                icon.name: "utilities-system-monitor"
                checkable: true
                checked: true
                text: qsTr("Show only significant over-represented results")

                onCheckedChanged:
                {
                    if(table.model !== null)
                        table.model.enrichedOnly = checked;

                    table.clearSelection();
                    table.positionViewAt(0);
                }
            }

            ToolBarButton
            {
                id: showHeatmapButton
                icon.name: "x-office-spreadsheet"
                checkable: true
                checked: true
                text: qsTr("Show Heatmap")
            }

            ToolBarButton
            {
                icon.name: "edit-delete"
                onClicked: confirmDelete.open();
                text: qsTr("Delete result table")
            }

            ToolBarButton
            {
                id: addEnrichment
                icon.name: "list-add"
                text: qsTr("New Enrichment")
                onClicked: wizard.show()
            }

            ToolBarButton { action: exportTableAction }
            ToolBarButton { action: saveImageAction }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        TabBar
        {
            id: tabBar
            Layout.topMargin: 4

            Repeater
            {
                model: root.models
                TabBarButton { text: Utils.format(qsTr("Results {0}"), index + 1) }
            }
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: 1
            color: ControlColors.outline
        }

        Item
        {
            visible: !splitView.visible
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text
            {
                anchors.centerIn: parent
                text: qsTr("No Results")
                color: palette.buttonText
            }
        }

        SplitView
        {
            id: splitView

            visible: root._resultsAvailable

            Layout.fillWidth: true
            Layout.fillHeight: true

            handle: SplitViewHandle {}

            DataTable
            {
                id: table

                SplitView.fillHeight: true
                SplitView.minimumWidth: 300
                SplitView.preferredWidth: root.width * 0.5

                selectionMode: DataTable.SingleSelection
                showBorder: false

                sortIndicatorColumn: 0
                sortIndicatorOrder: Qt.AscendingOrder

                onSortIndicatorColumnChanged:
                {
                    if(model !== null)
                        model.sortColumn = sortIndicatorColumn;
                }

                onSortIndicatorOrderChanged:
                {
                    if(model !== null)
                        model.ascendingSortOrder = (sortIndicatorOrder == Qt.AscendingOrder);
                }

                property int rowCount: 0
                function updateRowCount()
                {
                    table.rowCount = table.model !== null ? table.model.rowCount() : 0;
                }

                onHeaderClicked: function(column, mouse)
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

                // For some reason NativeUtils isn't available from directly within the sort expression
                function stringCompare(a, b) { return NativeUtils.compareStrings(a, b); }

                onClicked: function(column, row, mouse)
                {
                    if(mouse.button === Qt.RightButton)
                        exportTableMenu.popup();
                }

                property bool columnResizeRequired: false

                onCellExtentsChanged: function(left, right, top, bottom)
                {
                    if(columnResizeRequired)
                    {
                        resizeVisibleColumnsToContents();
                        columnResizeRequired = false;
                    }
                }

                Text
                {
                    anchors.centerIn: parent
                    text: qsTr("No Significant Results")
                    visible: table.rowCount <= 1 // rowCount includes header
                    color: palette.buttonText
                }

                model: root._currentModel

                onModelChanged:
                {
                    table.clearSelection();
                    table.columnResizeRequired = true;

                    table.sortIndicatorColumn = model ? model.sortColumn : 0;
                    table.sortIndicatorOrder = model ? (model.ascendingSortOrder === Qt.AscendingOrder) : true;
                    showOnlyEnrichedButton.checked = model ? model.enrichedOnly : true;

                    table.updateRowCount();
                }

                cellValueProvider: function(value)
                {
                    if(!isNaN(value) && value.length > 0)
                        return NativeUtils.formatNumberScientific(value);

                    return value;
                }
            }

            Item
            {
                SplitView.fillHeight: true
                SplitView.minimumWidth: 300

                visible: showHeatmapButton.checked

                EnrichmentHeatmap
                {
                    id: heatmap

                    anchors.fill: parent
                    anchors.rightMargin: verticalScrollBar.size < 1 ? verticalScrollBar.width : 0
                    anchors.bottomMargin: horizontalScrollBar.size < 1 ? horizontalScrollBar.height : 0

                    model: root._currentModel
                    elideLabelWidth: 100
                    xAxisLabel: model ? model.selectionA : ""
                    yAxisLabel: model ? model.selectionB : ""

                    onPlotValueClicked: function(row)
                    {
                        table.selectRow(row);
                        table.positionViewAt(row);
                    }

                    onRightClick: { plotContextMenu.popup(); }

                    scrollXAmount: horizontalScrollBar.position / (1.0 - horizontalScrollBar.size)
                    scrollYAmount: verticalScrollBar.position / (1.0 - verticalScrollBar.size)
                }

                ScrollBar
                {
                    id: horizontalScrollBar
                    hoverEnabled: true
                    active: hovered || pressed
                    orientation: Qt.Horizontal
                    size: heatmap.horizontalRangeSize
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: verticalScrollBar.size < 1 ? verticalScrollBar.width : 0
                    anchors.bottom: parent.bottom
                }

                ScrollBar
                {
                    id: verticalScrollBar
                    hoverEnabled: true
                    active: hovered || pressed
                    orientation: Qt.Vertical
                    size: heatmap.verticalRangeSize
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: horizontalScrollBar.size < 1 ? horizontalScrollBar.height : 0
                }

                ScrollBarCornerFiller
                {
                    horizontalScrollBar: horizontalScrollBar
                    verticalScrollBar: verticalScrollBar
                }
            }

            Connections
            {
                target: root._currentModel
                function onModelReset()
                {
                    table.updateRowCount();
                    heatmap.buildPlot();
                }
            }
        }
    }

    PlatformMenu
    {
        id: plotContextMenu
        Action
        {
            id: saveImageAction
            enabled: root._resultsAvailable && heatmap.visible
            text: qsTr("Save As Image…")
            icon.name: "camera-photo"
            onTriggered: function(source)
            {
                let folder = screenshot.path !== undefined ? screenshot.path : "";
                let path = root.saveFileName(folder);

                saveImageFileDialog.currentFolder = folder;
                saveImageFileDialog.selectedFile = NativeUtils.urlForFileName(path);
                saveImageFileDialog.open();
            }
        }
    }

    PlatformMenu
    {
        id: exportTableMenu
        Action
        {
            id: exportTableAction
            enabled: root._resultsAvailable && table.rowCount > 1 // rowCount includes header
            text: qsTr("Export Table…")
            icon.name: "document-save"
            onTriggered: function(source)
            {
                let folder = misc.fileSaveInitialFolder !== undefined ? misc.fileSaveInitialFolder : "";
                let path = root.saveFileName(folder);

                exportTableFileDialog.currentFolder = folder;
                exportTableFileDialog.selectedFile = NativeUtils.urlForFileName(path);
                exportTableFileDialog.open();
            }
        }
    }

    function saveFileName(folderUrl)
    {
        function sanitiseAttributeName(attributeName)
        {
            return attributeName.replace(/\s+/g, "_").toLowerCase();
        }

        let baseName = root.wizard.document.title.replace(/\.[^\.]+$/, "");
        let attributes = Utils.format(qsTr("{0}-vs-{1}"),
            sanitiseAttributeName(root._currentModel.selectionA),
            sanitiseAttributeName(root._currentModel.selectionB));
        let path = Utils.format(qsTr("{0}/{1}-enrichment-{2}"),
            NativeUtils.fileNameForUrl(folderUrl), baseName, attributes);

        return path;
    }

    SaveFileDialog
    {
        id: saveImageFileDialog

        title: qsTr("Save Plot As Image")
        nameFilters: [qsTr("PNG Image (*.png)"), qsTr("JPEG Image (*.jpg *.jpeg)"), qsTr("PDF Document (*.pdf)")]

        onAccepted:
        {
            screenshot.path = saveImageFileDialog.currentFolder.toString();
            heatmap.savePlotImage(saveImageFileDialog.selectedFile, saveImageFileDialog.selectedNameFilter.extensions);
        }
    }

    SaveFileDialog
    {
        id: exportTableFileDialog

        title: qsTr("Export Table")
        nameFilters: [qsTr("CSV File (*.csv)"), qsTr("TSV File (*.tsv)")]

        onAccepted:
        {
            misc.fileSaveInitialFolder = exportTableFileDialog.currentFolder.toString();
            wizard.document.writeTableModelToFile(
                table.model, exportTableFileDialog.selectedFile,
                exportTableFileDialog.selectedNameFilter.extensions[0]);
        }
    }


    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }

    Preferences
    {
        id: screenshot
        section: "screenshot"

        property var path
    }
}
