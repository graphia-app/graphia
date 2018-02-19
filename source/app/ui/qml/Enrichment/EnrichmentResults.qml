import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import SortFilterProxyModel 0.2

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
                    title: "Results " + index
                    TableView
                    {
                        id: tableView
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

                        Layout.fillWidth: true
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
                        TableViewColumn { role: "Expected Trial"; title: qsTr("ExpectedTrial"); width: 100 }
                        TableViewColumn { role: "Over-representation"; title: qsTr("OverRep"); width: 100 }
                        TableViewColumn { role: "Fishers"; title: qsTr("Fishers"); width: 100 }
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
