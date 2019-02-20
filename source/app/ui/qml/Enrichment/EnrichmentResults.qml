import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import SortFilterProxyModel 0.2
import com.kajeka 1.0

import Qt.labs.platform 1.0 as Labs

ApplicationWindow
{
    property var models
    property var currentHeatmap
    property var wizard
    property var currentTableView

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
            models.remove(models.get(tabView.currentIndex));
        }
    }

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
            property var tableViews: []
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: tabView.count > 0
            onCountChanged: currentIndex = count - 1;
            Repeater
            {
                model: models
                onItemAdded: Qt.callLater(resizeColumnsToContentsBugWorkaround);
                Tab
                {
                    id: tab
                    title: qsTr("Results") + " " + (index + 1)

                    SplitView
                    {
                        id: splitView
                        TableView
                        {
                            MouseArea
                            {
                                anchors.fill: parent
                                acceptedButtons: Qt.RightButton
                                propagateComposedEvents: true
                                onClicked:
                                {
                                    currentTableView = tableView;
                                    exportTableMenu.popup();
                                }
                            }

                            Text
                            {
                                anchors.centerIn: parent
                                text: qsTr("No Significant Results")
                                visible: tableView.rowCount === 0
                            }

                            id: tableView
                            visible: false
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumWidth: 100
                            sortIndicatorVisible: true
                            selectionMode: SelectionMode.SingleSelection
                            model: SortFilterProxyModel
                            {
                                id: proxyModel
                                sourceModel: qtObject
                                sorters: StringSorter { numericMode: true }

                                filters: ExpressionFilter
                                {
                                    enabled: showOnlyEnrichedButton.checked
                                    expression:
                                    {
                                        return Number(model["OverRep"]) > 1.0 && Number(model["AdjustedFishers"]) <= 0.05;
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
                                        if(styleData.value === undefined)
                                            return "";

                                        var column = tableView.getColumn(styleData.column);

                                        if(column !== null && !isNaN(styleData.value) && styleData.value !== "")
                                            return QmlUtils.formatNumberScientific(styleData.value, 1);

                                        return styleData.value;
                                    }

                                    color: styleData.textColor
                                    renderType: Text.NativeRendering
                                }
                            }

                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.SelectionA); title: qsTr("Selection A"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.SelectionB); title: qsTr("Selection B"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.Observed); title: qsTr("Observed"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.ExpectedTrial); title: qsTr("Expected"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.OverRep); title: qsTr("Representation"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.Fishers); title: qsTr("Fishers"); }
                            TableViewColumn { role: qtObject.resultToString(EnrichmentRoles.AdjustedFishers); title: qsTr("Adjusted Fishers"); }

                            Connections
                            {
                                target: qtObject
                                onDataChanged: resizeColumnsToContentsBugWorkaround()
                            }

                            Component.onCompleted: { tabView.tableViews.push(tableView); }

                            Component.onDestruction: { tabView.tableViews.splice(tabView.tableViews.indexOf(tableView), 1); }
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
                                model: qtObject
                                elideLabelWidth: 100
                                showOnlyEnriched: showOnlyEnrichedButton.checked

                                onPlotValueClicked:
                                {
                                    var convertedRow = proxyModel.mapFromSource(row);
                                    if(convertedRow === -1)
                                        return;
                                    tableView.selection.clear();
                                    tableView.selection.select(convertedRow);
                                    tableView.positionViewAtRow(convertedRow, ListView.Beginning);
                                    tableView.forceActiveFocus();
                                }

                                scrollXAmount:
                                {
                                    return scrollViewHorizontal.flickableItem.contentX /
                                            (scrollViewHorizontal.flickableItem.contentWidth - scrollViewHorizontal.viewport.width);
                                }
                                scrollYAmount:
                                {
                                    return scrollViewVertical.flickableItem.contentY /
                                            (scrollViewVertical.flickableItem.contentHeight - scrollViewVertical.viewport.height);
                                }
                                onRightClick:
                                {
                                    currentHeatmap = heatmap;
                                    plotContextMenu.popup();
                                }
                            }
                            ScrollView
                            {
                                id: scrollViewVertical
                                visible: true
                                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                                Layout.fillHeight: true;
                                implicitWidth: 15
                                Rectangle
                                {
                                    // This is a fake object to make native scrollbars appear
                                    // Prevent Qt opengl texture overflow (2^14 pixels)
                                    width: 1
                                    height: Math.min(heatmap.height / heatmap.verticalRangeSize, 16383)
                                    color: "transparent"
                                }
                            }
                            ScrollView
                            {
                                id: scrollViewHorizontal
                                visible: true
                                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                                implicitHeight: 15
                                Layout.fillWidth: true
                                Rectangle
                                {
                                    // This is a fake object to make native scrollbars appear
                                    // Prevent Qt opengl texture overflow (2^14 pixels)
                                    width: Math.min(heatmap.width / heatmap.horizontalRangeSize, 16383)
                                    height: 1
                                    color: "transparent"
                                }
                            }
                        }
                        Connections
                        {
                            target: qtObject
                            onModelReset: heatmap.buildPlot()
                        }
                    }
                }
            }
        }

    }

    Menu
    {
        id: plotContextMenu
        MenuItem
        {
            text: qsTr("Save as Image…")
            onTriggered: { heatmapSaveDialog.open(); }
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
        enabled: currentTableView && currentTableView.rowCount > 0;
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
            wizard.document.writeTableViewToFile(tabView.tableViews[tabView.currentIndex], file, defaultSuffix);
        }
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileSaveInitialFolder
    }

    // Work around for QTBUG-58594
    function resizeColumnsToContentsBugWorkaround()
    {
        for(var j = 0; j < tabView.tableViews.length; ++j)
        {
            var inTableView = tabView.tableViews[j];
            inTableView.visible = true;
            for(var i = 0; i < inTableView.columnCount; ++i)
            {
                var col = inTableView.getColumn(i);
                var header = inTableView.__listView.headerItem.headerRepeater.itemAt(i);
                if(col)
                {
                    col.__index = i;
                    col.resizeToContents();
                    if(col.width < header.implicitWidth)
                        col.width = header.implicitWidth;
                }
            }
        }
    }
}
