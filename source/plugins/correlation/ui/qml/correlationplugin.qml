import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

import com.kajeka 1.0
import SortFilterProxyModel 0.1

Item
{
    anchors.fill: parent

    Component.onCompleted:
    {
        parent.Layout.minimumHeight = 400;
    }

    SplitView
    {
        anchors.fill: parent

        NodeAttributeTableView
        {
            Layout.fillHeight: true
            nodeAttributesModel: plugin.model.nodeAttributes
        }

        CorrelationPlot
        {
            id: plot
            rowCount: plugin.model.rowCount
            columnCount: plugin.model.columnCount
            data: plugin.model.dataset
            columnNames: plugin.model.columnNames
            rowNames: plugin.model.rowNames
            selectedRows: plugin.model.selectedRows
            elideLabelWidth: 120

            onRightClick: { contextMenu.popup(); }

            Component.onCompleted: { plot.refresh(); }
        }
    }

    Connections
    {
        target: plugin.model
        onSelectedRowsChanged: { plot.refresh(); }
    }

    Menu
    {
        id: contextMenu

        MenuItem
        {
            text: "Save plot as..."
            onTriggered: imageSaveDialog.open();
        }
    }

    FileDialog
    {
        id: imageSaveDialog
        visible: false
        selectExisting: false
        title: "Save plot as..."
        nameFilters: [ "PDF Document (*.pdf)", "PNG Image (*.png)", "JPEG Image (*.jpg *.jpeg)" ]
        onAccepted: plot.savePlotImage(imageSaveDialog.fileUrl, imageSaveDialog.selectedNameFilter);
    }
}


