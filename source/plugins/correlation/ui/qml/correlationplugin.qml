import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtCharts 2.1
import QtQuick.Window 2.2
import QtWebEngine 1.3

import CustomPlot 1.0
import SortFilterProxyModel 0.1
import QtQuick.Dialogs 1.2

Item
{
    anchors.fill: parent

    Component.onCompleted: {
        parent.Layout.minimumHeight = 400;
        plugin.model.selectedRowsChanged.connect(onSelection);
    }

    function onSelection()
    {
        customPlot.initCustomPlot();
    }

    RowLayout
    {
        anchors.fill: parent

        SplitView
        {
            anchors.fill: parent

            NodeAttributeTableView
            {
                Layout.fillHeight: true
                nodeAttributesModel: plugin.model.nodeAttributes
            }

            CustomPlotItem
            {
                id: customPlot
                rowCount: plugin.model.rowCount
                columnCount: plugin.model.columnCount
                data: plugin.model.dataset
                columnNames: plugin.model.columnNames
                rowNames: plugin.model.rowNames
                selectedRows: plugin.model.selectedRows
                elideLabelWidth: 120;
                onRightClick: contextMenu.popup();

                Component.onCompleted: {
                    customPlot.initCustomPlot();
                }
            }
        }
    }

    Menu
    {
        id: contextMenu;
        MenuItem
        {
            text: "Save plot as..."
            onTriggered: imageSaveDialog.open();
        }
    }

    FileDialog
    {
        id: imageSaveDialog
        visible: false;
        selectExisting: false;
        title: "Save plot as..."
        nameFilters: [ "PNG (*.png)", "JPEG (*.jpg)", "PDF (*.pdf)"  ]
        onAccepted: customPlot.savePlotImage(imageSaveDialog.fileUrl, imageSaveDialog.selectedNameFilter);
    }
}


