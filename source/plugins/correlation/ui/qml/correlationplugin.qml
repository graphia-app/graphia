import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import com.kajeka 1.0

PluginContent
{
    anchors.fill: parent
    minimumHeight: 320

    Action
    {
        id: toggleUiOrientationAction
        text: qsTr("Display UI &Side By Side")
        iconName: "list-add"
        checkable: true
        checked: true
    }

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Contents")
        iconName: "format-justify-fill"
        onTriggered: tableView.resizeColumnsToContentsBugWorkaround();
    }

    Action
    {
        id: toggleColumnNamesAction
        text: qsTr("&Show Column Names")
        iconName: "format-text-bold"
        checkable: true
        checked: true
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            menu.title = qsTr("&Correlation");
            menu.addItem("").action = toggleUiOrientationAction;
            menu.addItem("").action = resizeColumnsToContentsAction;
            menu.addItem("").action = toggleColumnNamesAction;
            return true;
        }

        return false;
    }

    toolStrip: RowLayout
    {
        anchors.fill: parent

        ToolButton { action: toggleUiOrientationAction }
        ToolButton { action: resizeColumnsToContentsAction }
        ToolButton { action: toggleColumnNamesAction }
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
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 200 : -1

            nodeAttributesModel: plugin.model.userNodeDataModel

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

            height: 160

            Layout.fillWidth: splitView.orientation === Qt.Horizontal
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 + (height - viewport.height) : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 200 : -1

            horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            CorrelationPlot
            {
                id: plot

                width: minimumWidth > scrollView.width ? minimumWidth : scrollView.width
                height: scrollView.viewport.height

                rowCount: plugin.model.rowCount
                columnCount: plugin.model.columnCount
                data: plugin.model.rawData
                columnNames: plugin.model.columnNames
                rowNames: plugin.model.rowNames
                selectedRows: tableView.selectedRows
                showColumnNames: toggleColumnNamesAction.checked

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


