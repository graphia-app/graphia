import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import SortFilterProxyModel 0.2
import com.kajeka 1.0

Window
{
    property var models
    title: qsTr("Enrichment Results")
    height: 200
    width: 800

    ColumnLayout
    {
        anchors.fill: parent
        ToolBar
        {
            anchors.fill: parent
            RowLayout
            {
                ToolButton
                {
                    iconName: "edit-delete"
                    onClicked: models.remove(models.get(tabView.currentIndex))
                    tooltip: qsTr("Delete result table")
                }
                ToolButton
                {
                    id: showOnlyEnrichedButton
                    iconName: "utilities-system-monitor"
                    checkable: true
                    tooltip: qsTr("Show only over-represented")
                }
                ToolButton
                {
                    id: showHeatmapButton
                    iconName: "x-office-spreadsheet"
                    checkable: true
                    tooltip: qsTr("Show Heatmap")
                }
            }
        }
        Text
        {
            anchors.centerIn: parent
            text: qsTr("No Results")
            visible: tabView.count === 0
        }
        TabView
        {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: tabView.count > 0
            Repeater
            {
                model: models
                Tab
                {
                    id: tab
                    title: "Results " + index

                    SplitView
                    {
                        id: splitView
                        TableView
                        {
                            id: tableView
                            Layout.fillWidth: true
                            // Hacks so the sorter re-evaluates
                            onSortIndicatorColumnChanged:
                            {
                                sorter.roleName = getColumn(sortIndicatorColumn).role;
                                sorter.enabled = false
                                sorter.enabled = true;
                            }
                            onSortIndicatorOrderChanged:
                            {
                                sorter.order = sortIndicatorOrder;
                                sorter.enabled = false
                                sorter.enabled = true;
                            }

                            //Layout.preferredWidth: showHeatmapButton.checked ? parent.width / 2 : parent.width
                            Layout.fillHeight: true
                            sortIndicatorVisible: true
                            model: SortFilterProxyModel
                            {
                                id: proxyModel
                                sourceModel: qtObject
                                sorters: ExpressionSorter
                                {
                                    id: sorter
                                    property var roleName
                                    property var order
                                    expression:
                                    {
                                        if(roleName === undefined)
                                            return false;

                                        var convLeft = Number(modelLeft[roleName]);
                                        var convRight = Number(modelRight[roleName]);
                                        if(!isNaN(convLeft) && !isNaN(convRight))
                                        {
                                            if(order === Qt.AscendingOrder)
                                                return convLeft < convRight;
                                            else
                                                return convLeft > convRight;
                                        }
                                        else
                                        {
                                            if(order === Qt.AscendingOrder)
                                                return modelLeft[roleName] < modelRight[roleName];
                                            else
                                                return modelLeft[roleName] < modelRight[roleName];
                                        }
                                    }
                                }
                                filters: ExpressionFilter
                                {
                                    enabled: showOnlyEnrichedButton.checked
                                    expression:
                                    {
                                        return Number(model["OverRep"]) > 1.0;
                                    }
                                }
                            }

                            TableViewColumn { role: "Attribute Group"; title:  qsTr("Attribute Group"); width: 100 }
                            TableViewColumn { role: "Selection"; title: qsTr("Selection"); width: 100 }
                            TableViewColumn { role: "Observed"; title: qsTr("Observed"); width: 100 }
                            TableViewColumn { role: "Expected"; title: qsTr("Expected"); width: 100 }
                            TableViewColumn { role: "ExpectedTrial"; title: qsTr("Expected Trial"); width: 100 }
                            TableViewColumn { role: "OverRep"; title: qsTr("Over-Representation"); width: 100 }
                            TableViewColumn { role: "Fishers"; title: qsTr("Fishers"); width: 100 }
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
                                Layout.minimumHeight: 100
                                Layout.minimumWidth: 170
                                Layout.preferredWidth: (tab.width * 0.5) - 5
                                model: qtObject
                                elideLabelWidth: 100
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
    onModelsChanged:
    {
        for(var i=0; i<tabView.count; i++)
            tabView.getTab(i).children[0].update();
    }
}
