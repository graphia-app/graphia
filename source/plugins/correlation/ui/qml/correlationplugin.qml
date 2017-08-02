import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs

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

    Action
    {
        id: toggleCalculatedAttributes
        text: qsTr("&Show Calculated Attributes")
        iconName: "computer"
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
            menu.addItem("").action = toggleCalculatedAttributes;
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
        ToolButton { action: toggleCalculatedAttributes }
        Item { Layout.fillWidth: true }
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

            nodeAttributesModel: plugin.model.nodeAttributeTableModel
            showCalculatedAttributes: toggleCalculatedAttributes.checked

            onVisibleRowsChanged:
            {
                selection.clear();

                if(rowCount > 0)
                    selection.selectAll();
            }
        }

        ColumnLayout
        {
            spacing: 0

            CorrelationPlot
            {
                id: plot

                Layout.fillWidth: true
                Layout.fillHeight: true

                rowCount: plugin.model.rowCount
                columnCount: plugin.model.columnCount
                data: plugin.model.rawData
                columnNames: plugin.model.columnNames
                rowColors: plugin.model.nodeColors
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

                scrollAmount:
                {
                    return (scrollView.flickableItem.contentX) /
                            (scrollView.flickableItem.contentWidth - scrollView.viewport.width);
                }

                onRightClick: { contextMenu.popup(); }
            }
            ScrollView
            {
                id: scrollView
                visible: { return plot.rangeSize < 1.0 }
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                implicitHeight: 15
                Layout.fillWidth: true
                Rectangle
                {
                    // This is a fake object to make native scrollbars appear
                    // Prevent Qt opengl texture overflow (2^14 pixels)
                    width: Math.min(plot.width / plot.rangeSize, 16383);
                    height: 1
                    color: "transparent"
                }
            }
        }
    }

    Menu
    {
        id: contextMenu

        MenuItem
        {
            text: "Save plot as…"
            onTriggered: imageSaveDialog.open();
        }
    }

    Labs.FileDialog
    {
        id: imageSaveDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 1
        title: "Save plot as…"
        nameFilters: [ "PDF Document (*.pdf)", "PNG Image (*.png)", "JPEG Image (*.jpg *.jpeg)" ]
        onAccepted: { plot.savePlotImage(file, selectedNameFilter.extensions); }
    }
}


