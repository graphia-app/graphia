import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

import com.kajeka 1.0

PluginContent
{
    anchors.fill: parent

    Component.onCompleted:
    {
        parent.Layout.minimumHeight = 320;
        scrollView.height = 160;
    }

    Action
    {
        id: toggleUiOrientationAction
        text: qsTr("Display UI Horizontally")
        iconName: "add"
        checkable: true
    }

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Contents")
        iconName: "format-justify-fill"
        onTriggered: tableView.resizeColumnsToContents();
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            menu.title = qsTr("&Correlation");
            menu.addItem("").action = toggleUiOrientationAction;
            menu.addItem("").action = resizeColumnsToContentsAction;
            return true;
        }

        return false;
    }

    toolStrip: RowLayout
    {
        anchors.fill: parent

        ToolButton { action: toggleUiOrientationAction }
        ToolButton { action: resizeColumnsToContentsAction }
        Item { Layout.fillWidth: true}
    }

    SplitView
    {
        id: splitView

        orientation: toggleUiOrientationAction.checked ? Qt.Horizontal : Qt.Vertical

        anchors.fill: parent

        NodeAttributeTableView
        {
            id: tableView
            Layout.fillHeight: splitView.orientation === Qt.Vertical
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 + (height - viewport.height) : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 200 + (width - viewport.width) : -1

            nodeAttributesModel: plugin.model.nodeAttributes

            onVisibleRowsChanged:
            {
                selection.clear();

                if(rowCount > 0)
                    selection.selectAll();
            }
        }

        ScrollView
        {
            id: scrollView
            Layout.fillWidth: splitView.orientation === Qt.Horizontal
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 + (height - viewport.height) : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 200 + (width - viewport.width) : -1

            horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            CorrelationPlot
            {
                id: plot

                width: minimumWidth > scrollView.width ? minimumWidth : scrollView.width
                height: scrollView.viewport.height

                rowCount: plugin.model.rowCount
                columnCount: plugin.model.columnCount
                data: plugin.model.dataset
                columnNames: plugin.model.columnNames
                rowNames: plugin.model.rowNames
                selectedRows: tableView.selectedRows

                elideLabelWidth:
                {
                    var newHeight = height * 0.25;
                    var quant = 20;
                    var quantised = Math.floor(newHeight / quant) * quant;

                    if(quantised < 40)
                        quantised = 0;

                    return quantised;
                }

                onRightClick: { contextMenu.popup(); }
            }
        }
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


