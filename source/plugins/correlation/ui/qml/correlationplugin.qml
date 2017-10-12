import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0

import "../../../../shared/ui/qml/Utils.js" as Utils

PluginContent
{
    id: root

    anchors.fill: parent
    minimumHeight: 320

    Action
    {
        id: toggleUiOrientationAction
        text: qsTr("Display UI &Side By Side")
        iconName: "list-add"
        checkable: true
        checked: true

        onCheckedChanged: { root.saveRequired = true; }
    }

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Fit Contents")
        iconName: "format-justify-fill"
        onTriggered: tableView.resizeColumnsToContentsBugWorkaround();
    }

    Action
    {
        id: toggleColumnNamesAction
        text: qsTr("Show &Column Names")
        iconName: "format-text-bold"
        checkable: true
        checked: true

        onCheckedChanged: { root.saveRequired = true; }
    }

    Action
    {
        id: selectColumnsAction
        text: qsTr("&Select Visible Columns")
        iconName: "computer"
        checkable: true
        checked: tableView.columnSelectionMode

        onTriggered: { tableView.columnSelectionMode = !tableView.columnSelectionMode; }
    }

    Action
    {
        id: toggleGridLines
        text: qsTr("&Grid Lines")
        checkable: true
        checked: plot.showGridLines

        onTriggered: { plot.showGridLines = !plot.showGridLines; }
    }

    Action
    {
        id: togglePlotLegend
        text: qsTr("&Legend")
        checkable: true
        checked: plot.showLegend

        onTriggered: { plot.showLegend = !plot.showLegend; }
    }

    ExclusiveGroup
    {
        Action
        {
            id: rawScaling
            text: qsTr("&Raw")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Raw
            onTriggered: { plot.plotScaleType = PlotScaleType.Raw; }
        }

        Action
        {
            id: logScaling
            text:  qsTr("Log(𝒙 + ε)");
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Log
            onTriggered: { plot.plotScaleType = PlotScaleType.Log; }
        }

        Action
        {
            id: meanCentreScaling
            text: qsTr("&Mean Centre Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.MeanCentre
            onTriggered: { plot.plotScaleType = PlotScaleType.MeanCentre; }
        }

        Action
        {
            id: unitVarianceScaling
            text: qsTr("&Unit Varience Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.UnitVariance
            onTriggered: { plot.plotScaleType = PlotScaleType.UnitVariance; }
        }

        Action
        {
            id: paretoScaling
            text: qsTr("&Pareto Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Pareto
            onTriggered: { plot.plotScaleType = PlotScaleType.Pareto; }
        }
    }

    Action
    {
        id: savePlotImageAction
        text: qsTr("Save As &Image…")
        iconName: "edit-save"
        onTriggered: { imageSaveDialog.open(); }
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            tableView.populateTableMenu(menu);
            return true;

        case 1:
            menu.title = qsTr("&Plot");
            menu.addItem("").action = toggleColumnNamesAction;
            menu.addItem("").action = savePlotImageAction;
            menu.addItem("").action = toggleGridLines;
            menu.addItem("").action = togglePlotLegend;
            var scalingMenu = menu.addMenu("Scaling");
            scalingMenu.addItem("").action = rawScaling;
            scalingMenu.addItem("").action = logScaling;
            scalingMenu.addItem("").action = meanCentreScaling;
            scalingMenu.addItem("").action = unitVarianceScaling;
            scalingMenu.addItem("").action = paretoScaling;

            Utils.cloneMenu(menu, plotContextMenu);
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
        ToolButton { action: selectColumnsAction }
        Item { Layout.fillWidth: true }
    }

    function onResized()
    {
        plot.refresh();
    }

    SplitView
    {
        id: splitView

        orientation: toggleUiOrientationAction.checked ? Qt.Horizontal : Qt.Vertical

        anchors.fill: parent

        Timer
        {
            id: plotRefreshTimer; interval: 100; onTriggered: { plot.refresh(); }
        }

        // When the orientation changes it seems to take a little while for the
        // plot's dimensional properties to "settle", and calling refresh() before
        // this doesn't really work, so wait a little bit first (FIXME)
        onOrientationChanged:
        {
            plotRefreshTimer.start();
        }

        onResizingChanged:
        {
            if(!resizing)
                plot.refresh();
        }

        NodeAttributeTableView
        {
            id: tableView
            Layout.fillHeight: splitView.orientation === Qt.Vertical
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 400 : -1

            nodeAttributesModel: plugin.model.nodeAttributeTableModel

            onVisibleRowsChanged:
            {
                selection.clear();

                if(rowCount > 0)
                    selection.selectAll();
            }

            onSortIndicatorColumnChanged: { root.saveRequired = true; }
            onSortIndicatorOrderChanged: { root.saveRequired = true; }
        }

        Column
        {
            Layout.fillWidth: true
            Layout.fillHeight: splitView.orientation !== Qt.Vertical
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 200 : -1

            CorrelationPlot
            {
                id: plot

                implicitHeight: parent.height - (scrollBarRequired ? scrollView.height : 0)
                implicitWidth: parent.width

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

                property bool scrollBarRequired: rangeSize < 1.0

                scrollAmount:
                {
                    return scrollView.flickableItem.contentX /
                        (scrollView.flickableItem.contentWidth - scrollView.viewport.width);
                }

                onRightClick: { plotContextMenu.popup(); }
            }

            ScrollView
            {
                id: scrollView
                visible: plot.scrollBarRequired
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                implicitHeight: 15
                implicitWidth: parent.width
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

    Menu { id: plotContextMenu }

    Labs.FileDialog
    {
        id: imageSaveDialog
        visible: false
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: selectedNameFilter.extensions[0]
        selectedNameFilter.index: 1
        title: qsTr("Save Plot As Image")
        nameFilters: [ "PDF Document (*.pdf)", "PNG Image (*.png)", "JPEG Image (*.jpg *.jpeg)" ]
        onAccepted: { plot.savePlotImage(file, selectedNameFilter.extensions); }
    }

    property bool saveRequired: false

    function save()
    {
        var data =
        {
            "sideBySide": toggleUiOrientationAction.checked,
            "showColumnNames": toggleColumnNamesAction.checked,
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder,
            "hiddenColumns": tableView.hiddenColumns
        };

        return data;
    }

    function load(data, version)
    {
        if(data.sideBySide !== undefined)               toggleUiOrientationAction.checked = data.sideBySide;
        if(data.showColumnNames !== undefined)          toggleColumnNamesAction.checked = data.showColumnNames;
        if(data.sortColumn !== undefined)               tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                tableView.sortIndicatorOrder = data.sortOrder;
        if(data.hiddenColumns !== undefined)            tableView.hiddenColumns = data.hiddenColumns;
    }
}
