import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0

import "../../../../shared/ui/qml/Utils.js" as Utils

import "Controls"

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
        text: qsTr("Resize Columns To Fit Contents")
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
        text: qsTr("Select Visible Columns")
        iconName: "computer"
        checkable: true
        checked: tableView.columnSelectionMode

        onTriggered: { tableView.columnSelectionMode = !tableView.columnSelectionMode; }
    }

    Action
    {
        id: selectAllAction
        text: qsTr("Select All")
        iconName: "edit-select-all"
        enabled: tableView.rowCount > 0

        onTriggered: { tableView.selection.selectAll(); }
    }

    Action
    {
        id: toggleGridLines
        text: qsTr("Grid Lines")
        checkable: true
        checked: plot.showGridLines

        onTriggered: { plot.showGridLines = !plot.showGridLines; }
    }

    Action
    {
        id: togglePlotLegend
        text: qsTr("Legend")
        checkable: true
        checked: plot.showLegend

        onTriggered: { plot.showLegend = !plot.showLegend; }
    }

    ExclusiveGroup
    {
        Action
        {
            id: rawScaling
            text: qsTr("Raw")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Raw
            onTriggered: { plot.plotScaleType = PlotScaleType.Raw; }
        }

        Action
        {
            id: logScaling
            text:  qsTr("Log(x + ε)");
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Log
            onTriggered: { plot.plotScaleType = PlotScaleType.Log; }
        }

        Action
        {
            id: meanCentreScaling
            text: qsTr("Mean Centre Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.MeanCentre
            onTriggered: { plot.plotScaleType = PlotScaleType.MeanCentre; }
        }

        Action
        {
            id: unitVarianceScaling
            text: qsTr("Unit Variance Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.UnitVariance
            onTriggered: { plot.plotScaleType = PlotScaleType.UnitVariance; }
        }

        Action
        {
            id: paretoScaling
            text: qsTr("Pareto Scaling")
            checkable: true
            checked: plot.plotScaleType === PlotScaleType.Pareto
            onTriggered: { plot.plotScaleType = PlotScaleType.Pareto; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: barDeviationVisual
            enabled: plot.plotDispersionType !== PlotDispersionType.None
            text: qsTr("Error Bars")
            checkable: true
            checked: plot.plotDispersionVisualType === PlotDispersionVisualType.Bars
            onTriggered: { plot.plotDispersionVisualType = PlotDispersionVisualType.Bars; }
        }
        Action
        {
            id: graphDeviationVisual
            enabled: plot.plotDispersionType !== PlotDispersionType.None
            text: qsTr("Area")
            checkable: true
            checked: plot.plotDispersionVisualType === PlotDispersionVisualType.Area
            onTriggered: { plot.plotDispersionVisualType = PlotDispersionVisualType.Area; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: noDispersion
            text: qsTr("None")
            checkable: true
            checked: plot.plotDispersionType === PlotDispersionType.None
            onTriggered: { plot.plotDispersionType = PlotDispersionType.None; }
        }
        Action
        {
            id: stdDeviations
            text: qsTr("Standard Deviation")
            checkable: true
            checked: plot.plotDispersionType === PlotDispersionType.StdDev
            onTriggered: { plot.plotDispersionType = PlotDispersionType.StdDev; }
        }
        Action
        {
            id: stdErrorDeviations
            text: qsTr("Standard Error")
            checkable: true
            checked: plot.plotDispersionType === PlotDispersionType.StdErr
            onTriggered: { plot.plotDispersionType = PlotDispersionType.StdErr; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: individualLineAverage
            text: qsTr("Individual Line")
            checkable: true
            checked: plot.plotAveragingType === PlotAveragingType.Individual
            onTriggered: { plot.plotAveragingType = PlotAveragingType.Individual; }
        }

        Action
        {
            id: meanLineAverage
            text:  qsTr("Mean Line");
            checkable: true
            checked: plot.plotAveragingType === PlotAveragingType.MeanLine
            onTriggered: { plot.plotAveragingType = PlotAveragingType.MeanLine; }
        }

        Action
        {
            id: medianLineAverage
            text: qsTr("Median Line")
            checkable: true
            checked: plot.plotAveragingType === PlotAveragingType.MedianLine
            onTriggered: { plot.plotAveragingType = PlotAveragingType.MedianLine; }
        }

        Action
        {
            id: meanHistogramAverage
            text: qsTr("Mean Histogram")
            checkable: true
            checked: plot.plotAveragingType === PlotAveragingType.MeanHistogram
            onTriggered: { plot.plotAveragingType = PlotAveragingType.MeanHistogram; }
        }

        Action
        {
            id: iqrAverage
            text: qsTr("IQR Plot")
            checkable: true
            checked: plot.plotAveragingType === PlotAveragingType.IQRPlot
            onTriggered: { plot.plotAveragingType = PlotAveragingType.IQRPlot; }
        }
    }

    Action
    {
        id: savePlotImageAction
        text: qsTr("Save As &Image…")
        iconName: "edit-save"
        onTriggered: { imageSaveDialog.open(); }
    }

    Action
    {
        id: setXAxisLabelAction
        text: qsTr("Set X Axis Label…")
        onTriggered: { xAxisLabelDialog.open(); }
    }

    Action
    {
        id: setYAxisLabelAction
        text: qsTr("Set Y Axis Label…")
        onTriggered: { yAxisLabelDialog.open(); }
    }

    Dialog
    {
        id: xAxisLabelDialog
        visible: false
        title: "X Axis Label"

        standardButtons: StandardButton.Ok | StandardButton.Cancel
        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: "Please enter an X Axis label:"
            }
            TextField
            {
                id: xAxisTextField
                Layout.bottomMargin: 12
                implicitWidth: 150
                text: plot.xAxisLabel
            }
        }
        onAccepted: plot.xAxisLabel = xAxisTextField.text;
    }

    Dialog
    {
        id: yAxisLabelDialog
        visible: false
        title: "Y Axis Label"

        standardButtons: StandardButton.Ok | StandardButton.Cancel
        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: "Please enter an Y Axis label:"
            }
            TextField
            {
                id: yAxisTextField
                Layout.bottomMargin: 12
                implicitWidth: 150
                text: plot.xAxisLabel
            }
        }

        onAccepted: plot.yAxisLabel = yAxisTextField.text;
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

            var axisLabels = menu.addMenu(qsTr("Axis Labels"));
            axisLabels.addItem("").action = setXAxisLabelAction;
            axisLabels.addItem("").action = setYAxisLabelAction;
            menu.addSeparator();

            var scalingMenu = menu.addMenu(qsTr("Scaling"));
            scalingMenu.addItem("").action = rawScaling;
            scalingMenu.addItem("").action = logScaling;
            scalingMenu.addItem("").action = meanCentreScaling;
            scalingMenu.addItem("").action = unitVarianceScaling;
            scalingMenu.addItem("").action = paretoScaling;
            scalingMenu.enabled = Qt.binding(function()
            {
                return plot.plotAveragingType == PlotAveragingType.Individual
            });

            var averagingMenu = menu.addMenu(qsTr("Averaging"));
            averagingMenu.addItem("").action = individualLineAverage;
            averagingMenu.addItem("").action = meanLineAverage;
            averagingMenu.addItem("").action = medianLineAverage;
            averagingMenu.addItem("").action = meanHistogramAverage;
            averagingMenu.addItem("").action = iqrAverage;

            var dispersionMenu = menu.addMenu(qsTr("Dispersion"));
            dispersionMenu.addItem("").action = noDispersion;
            dispersionMenu.addItem("").action = stdDeviations;
            dispersionMenu.addItem("").action = stdErrorDeviations;
            dispersionMenu.addSeparator();
            dispersionMenu.addItem("").action = barDeviationVisual;
            dispersionMenu.addItem("").action = graphDeviationVisual;
            dispersionMenu.enabled = Qt.binding(function()
            {
                return plot.plotAveragingType == PlotAveragingType.MeanLine ||
                        plot.plotAveragingType == PlotAveragingType.MeanHistogram
            });

            Utils.cloneMenu(menu, plotContextMenu);
            return true;
        }

        return false;
    }

    toolStrip: RowLayout
    {
        anchors.fill: parent

        ToolButton { action: toggleUiOrientationAction }
        ToolBarSeparator {}
        ToolButton { action: resizeColumnsToContentsAction }
        ToolButton { action: selectColumnsAction }
        ToolButton { action: tableView.exportAction }
        ToolBarSeparator {}
        ToolButton { action: toggleColumnNamesAction }
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
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 400 : -1

            model: plugin.model.nodeAttributeTableModel

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
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? _minimumHeight : root.minimumHeight
            Layout.minimumWidth: _minimumWidth

            readonly property int _minimumWidth: 200
            readonly property int _minimumHeight: 100

            CorrelationPlot
            {
                id: plot

                // When the SplitView orientation is changed, it makes various seemingly spurious dimensional
                // changes to its children, so this is an attempt to effectively filter out the more wacky ones
                height:
                {
                    var minimumHeight = Math.min(splitView.height, Math.max(parent._minimumHeight, parent.height));
                    return minimumHeight - (scrollBarRequired ? scrollView.height : 0);
                }

                width: { return Math.min(splitView.width, Math.max(parent._minimumWidth, parent.width)); }

                rowCount: plugin.model.rowCount
                columnCount: plugin.model.columnCount
                rawData: plugin.model.rawData
                columnNames: plugin.model.columnNames
                rowColors: plugin.model.nodeColors
                rowNames: plugin.model.rowNames
                selectedRows: tableView.selectedRows
                showColumnNames: toggleColumnNamesAction.checked

                columnAnnotations: plugin.model.columnAnnotations

                property var _availableColumnAnnotationNames:
                {
                    var list = [];

                    plugin.model.columnAnnotations.forEach(function(columnAnnotation)
                    {
                        list.push(columnAnnotation.name);
                    });

                    return list;
                }

                visibleColumnAnnotationNames: _availableColumnAnnotationNames

                elideLabelWidth:
                {
                    var newHeight = height * 0.25;
                    var quant = 20;
                    var quantised = Math.floor(newHeight / quant) * quant;

                    if(quantised < 40)
                        quantised = 0;

                    return quantised;
                }

                property bool scrollBarRequired: visibleHorizontalFraction < 1.0

                horizontalScrollPosition:
                {
                    return scrollView.flickableItem.contentX /
                        (scrollView.flickableItem.contentWidth - scrollView.viewport.width);
                }

                onRightClick: { plotContextMenu.popup(); }

                property bool _timedBusy: false

                Timer
                {
                    id: busyIndicationTimer
                    interval: 250
                    repeat: false
                    onTriggered:
                    {
                        if(plot.busy)
                            plot._timedBusy = true;
                    }
                }

                onBusyChanged:
                {
                    if(!plot.busy)
                    {
                        busyIndicationTimer.stop();
                        plot._timedBusy = false;
                    }
                    else
                        busyIndicationTimer.start();

                }

                BusyIndicator
                {
                    anchors.centerIn: parent
                    width: 64
                    height: 64

                    visible: plot._timedBusy
                }

            }

            ScrollView
            {
                id: scrollView
                visible: plot.scrollBarRequired
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                height: 15
                width: plot.width
                Item
                {
                    // This is a fake object to make native scrollbars appear
                    // Prevent Qt opengl texture overflow (2^14 pixels)
                    width: Math.min(plot.width / plot.visibleHorizontalFraction, 16383);
                    height: 1
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

    function initialise()
    {
        var columns = [];

        plugin.model.nodeAttributeTableModel.columnNames.forEach(function(columnName)
        {
            if(plugin.model.nodeAttributeTableModel.columnIsHiddenByDefault(columnName))
                columns = Utils.setAdd(columns, columnName);
        });

        tableView.hiddenColumns = columns;
    }

    function save()
    {
        var data =
        {
            "sideBySide": toggleUiOrientationAction.checked,
            "showColumnNames": toggleColumnNamesAction.checked,
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder,
            "hiddenColumns": tableView.hiddenColumns,

            "plotScaling": plot.plotScaleType,
            "plotAveraging": plot.plotAveragingType,
            "plotDispersion": plot.plotDispersionType,
            "plotDispersionVisual": plot.plotDispersionVisualType,
            "plotLegend": plot.showLegend,
            "plotGridLines": plot.showGridLines,
            "columnAnnotations": plot.visibleColumnAnnotationNames
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

        if(data.plotScaling !== undefined)              plot.plotScaleType = data.plotScaling;
        if(data.plotAveraging !== undefined)            plot.plotAveragingType = data.plotAveraging;
        if(data.plotDispersion !== undefined)           plot.plotDispersionType = data.plotDispersion;
        if(data.plotDispersionVisual !== undefined)     plot.plotDispersionVisualType = data.plotDispersionVisual;
        if(data.plotLegend !== undefined)               plot.showLegend = data.plotLegend;
        if(data.plotGridLines !== undefined)            plot.showGridLines = data.plotGridLines;
        if(data.columnAnnotations !== undefined)        plot.visibleColumnAnnotationNames = data.columnAnnotations;
    }
}
