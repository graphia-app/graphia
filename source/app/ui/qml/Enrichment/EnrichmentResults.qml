/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

ApplicationWindow
{
    id: root

    property var models
    property var wizard

    property var currentTableView: null
    property var currentHeatmap: null

    function updateCurrent()
    {
        let tab = tabView.getTab(tabView.currentIndex);
        if(!tab)
            return;

        let item = tabView.getTab(tabView.currentIndex).item;
        if(!item)
            return;

        root.currentTableView = item.childTableView;
        root.currentHeatmap = item.childHeatmap;

    }

    onModelsChanged: { root.updateCurrent(); }

    title: qsTr("Enrichment Results")
    minimumHeight: 400
    minimumWidth: 800

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
                onItemAdded:
                {
                    Qt.callLater(function()
                    {
                        let tab = item;
                        tab.item.childTableView.resizeColumnsToContents();
                    });
                }

                Tab
                {
                    id: tab
                    title: qsTr("Results") + " " + (index + 1)

                    SplitView
                    {
                        id: splitView

                        property alias childTableView: tableView
                        property alias childHeatmap: heatmap

                        TableView
                        {
                            id: tableView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumWidth: 100

                            MouseArea
                            {
                                anchors.fill: parent
                                acceptedButtons: Qt.RightButton
                                propagateComposedEvents: true
                                onClicked: { exportTableMenu.popup(); }
                            }

                            Text
                            {
                                anchors.centerIn: parent
                                text: qsTr("No Significant Results")
                                visible: tableView.rowCount === 0
                            }

                            sortIndicatorVisible: true
                            selectionMode: SelectionMode.SingleSelection
                            model: SortFilterProxyModel
                            {
                                id: proxyModel
                                sourceModel: modelData

                                sorters:
                                [
                                    RoleSorter
                                    {
                                        enabled: modelData.resultIsNumerical(tableView.sortIndicatorColumn)
                                        roleName: tableView.getColumn(tableView.sortIndicatorColumn).role
                                        sortOrder: tableView.sortIndicatorOrder
                                    },
                                    StringSorter
                                    {
                                        enabled: !modelData.resultIsNumerical(tableView.sortIndicatorColumn)
                                        roleName: tableView.getColumn(tableView.sortIndicatorColumn).role
                                        sortOrder: tableView.sortIndicatorOrder
                                        numericMode: true
                                    }
                                ]

                                filters: ExpressionFilter
                                {
                                    enabled: showOnlyEnrichedButton.checked
                                    expression:
                                    {
                                        return Number(model["OverRep"]) > 1.0 && Number(model["BonferroniAdjusted"]) <= 0.05;
                                    }
                                }
                            }

                            itemDelegate: Item
                            {
                                height: Math.max(16, label.implicitHeight)
                                property int implicitWidth: label.implicitWidth + 16

                                Text
                                {
                                    id: label
                                    objectName: "label"
                                    width: parent.width
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.leftMargin: styleData.hasOwnProperty("depth") && styleData.column === 0 ? 0 :
                                                        horizontalAlignment === Text.AlignRight ? 1 : 8
                                    anchors.rightMargin: (styleData.hasOwnProperty("depth") && styleData.column === 0)
                                                         || horizontalAlignment !== Text.AlignRight ? 1 : 8
                                    horizontalAlignment: styleData.textAlignment
                                    anchors.verticalCenter: parent.verticalCenter
                                    elide: styleData.elideMode

                                    text:
                                    {
                                        if(styleData.value === undefined || typeof(styleData.value) === 'object')
                                            return "";

                                        let column = tableView.getColumn(styleData.column);

                                        if(column !== null && !isNaN(styleData.value) && styleData.value !== "")
                                            return QmlUtils.formatNumberScientific(styleData.value, 1);

                                        return styleData.value;
                                    }

                                    color: styleData.textColor
                                    renderType: Text.NativeRendering
                                }
                            }

                            TableViewColumn
                            {
                                role: modelData.resultToString(EnrichmentRoles.SelectionA)
                                title: modelData.selectionA.length > 0 ? modelData.selectionA : qsTr("Selection A")
                            }

                            TableViewColumn
                            {
                                role: modelData.resultToString(EnrichmentRoles.SelectionB)
                                title: modelData.selectionB.length > 0 ? modelData.selectionB : qsTr("Selection B")
                            }

                            TableViewColumn { role: modelData.resultToString(EnrichmentRoles.Observed); title: qsTr("Observed"); }
                            TableViewColumn { role: modelData.resultToString(EnrichmentRoles.ExpectedTrial); title: qsTr("Expected"); }
                            TableViewColumn { role: modelData.resultToString(EnrichmentRoles.OverRep); title: qsTr("Representation"); }
                            TableViewColumn { role: modelData.resultToString(EnrichmentRoles.Fishers); title: qsTr("Fishers"); }
                            TableViewColumn { role: modelData.resultToString(EnrichmentRoles.BonferroniAdjusted); title: qsTr("Bonferroni Adjusted"); }

                            Connections
                            {
                                target: modelData
                                function onDataChanged() { resizeColumnsToContentsBugWorkaround(); }
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

                                onPlotValueClicked:
                                {
                                    let convertedRow = proxyModel.mapFromSource(row);
                                    if(convertedRow === -1)
                                        return;
                                    tableView.selection.clear();
                                    tableView.selection.select(convertedRow);
                                    tableView.positionViewAtRow(convertedRow, ListView.Beginning);
                                    tableView.forceActiveFocus();
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
        enabled: root.currentTableView && root.currentTableView.rowCount > 0
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
            wizard.document.writeTableViewToFile(
                root.currentTableView, file, defaultSuffix);
        }
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }
}
