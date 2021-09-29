/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs

import app.graphia 1.0

import "../../../../shared/ui/qml/Utils.js" as Utils
import "../../../../shared/ui/qml/Constants.js" as Constants

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
        iconName: "view-refresh"
        checkable: true
        checked: true

        onCheckedChanged: { root.saveRequired = true; }
    }

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("Resize Columns To Fit Contents")
        iconName: "auto-column-resize"
        onTriggered: tableView.resizeColumnsToContents();
    }

    Action
    {
        id: toggleColumnNamesAction
        text: qsTr("Show &Column Names")
        iconName: "format-text-bold"
        checkable: true
        checked: plot.showColumnNames

        onTriggered:
        {
            plot.showAllColumns = false;
            plot.showColumnNames = !plot.showColumnNames;
        }
    }

    Action
    {
        id: selectColumnsAction
        text: qsTr("Select Visible Columns")
        iconName: "column-select"
        checkable: true
        checked: tableView.columnSelectionMode

        onTriggered:
        {
            tableView.columnSelectionMode = !tableView.columnSelectionMode;
        }
    }

    MessageDialog
    {
        id: tooManyAnnotationsDialog
        visible: false
        title: qsTr("Too Many Annotations")
        text: qsTr("There is currently not enough physical space to display the " +
            "column annotations. Please increase the vertical size of the plot and " +
            "try again.")

        icon: StandardIcon.Critical
        standardButtons: StandardButton.Ok
    }

    Action
    {
        id: selectColumnAnnotationsAction
        text: qsTr("Select Visible Column Annotations")
        iconName: "column-annotations"

        enabled: plugin.model.columnAnnotationNames.length > 0

        checkable: true

        onTriggered:
        {
            if(!plot.canShowColumnAnnotationSelection)
            {
                tooManyAnnotationsDialog.open();
                checked = false;
                return;
            }

            plot.columnAnnotationSelectionModeEnabled = !plot.columnAnnotationSelectionModeEnabled;
            checked = plot.columnAnnotationSelectionModeEnabled;
        }
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

    Action
    {
        id: toggleIncludeYZero
        text: qsTr("Include Zero In Y Axis")
        checkable: true
        checked: plot.includeYZero

        onTriggered: { plot.includeYZero = !plot.includeYZero; }
    }

    Action
    {
        id: resetZoom
        text: qsTr("Reset Zoom")
        enabled: plot.zoomed

        onTriggered: { plot.resetZoom(); }
    }

    Action
    {
        id: toggleShowAllColumns
        text: qsTr("Show All Columns")
        checkable: true
        checked: plot.showAllColumns

        onTriggered:
        {
            plot.showColumnNames = false;
            plot.showAllColumns = !plot.showAllColumns;
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: rawScaling
            text: qsTr("None")
            checkable: true
            checked: plot.scaleType === PlotScaleType.Raw
            onTriggered: { plot.scaleType = PlotScaleType.Raw; }
        }

        Action
        {
            id: logScaling
            text:  qsTr("Log(x + ε)");
            checkable: true
            checked: plot.scaleType === PlotScaleType.Log
            onTriggered: { plot.scaleType = PlotScaleType.Log; }
        }

        Action
        {
            id: meanCentreScaling
            text: qsTr("Mean Centre Scaling")
            enabled: !plot.iqrStyle
            checkable: true
            checked: plot.scaleType === PlotScaleType.MeanCentre
            onTriggered: { plot.scaleType = PlotScaleType.MeanCentre; }
        }

        Action
        {
            id: unitVarianceScaling
            text: qsTr("Unit Variance Scaling")
            enabled: !plot.iqrStyle
            checkable: true
            checked: plot.scaleType === PlotScaleType.UnitVariance
            onTriggered: { plot.scaleType = PlotScaleType.UnitVariance; }
        }

        Action
        {
            id: paretoScaling
            text: qsTr("Pareto Scaling")
            enabled: !plot.iqrStyle
            checkable: true
            checked: plot.scaleType === PlotScaleType.Pareto
            onTriggered: { plot.scaleType = PlotScaleType.Pareto; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: barDeviationVisual
            enabled: plot.dispersionType !== PlotDispersionType.None
            text: qsTr("Error Bars")
            checkable: true
            checked: plot.dispersionVisualType === PlotDispersionVisualType.Bars
            onTriggered: { plot.dispersionVisualType = PlotDispersionVisualType.Bars; }
        }

        Action
        {
            id: graphDeviationVisual
            enabled: plot.dispersionType !== PlotDispersionType.None
            text: qsTr("Area")
            checkable: true
            checked: plot.dispersionVisualType === PlotDispersionVisualType.Area
            onTriggered: { plot.dispersionVisualType = PlotDispersionVisualType.Area; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: noDispersion
            text: qsTr("None")
            checkable: true
            checked: plot.dispersionType === PlotDispersionType.None
            onTriggered: { plot.dispersionType = PlotDispersionType.None; }
        }
        Action
        {
            id: stdDeviations
            text: qsTr("Standard Deviation")
            checkable: true
            checked: plot.dispersionType === PlotDispersionType.StdDev
            onTriggered: { plot.dispersionType = PlotDispersionType.StdDev; }
        }
        Action
        {
            id: stdErrorDeviations
            text: qsTr("Standard Error")
            checkable: true
            checked: plot.dispersionType === PlotDispersionType.StdErr
            onTriggered: { plot.dispersionType = PlotDispersionType.StdErr; }
        }
    }

    ExclusiveGroup
    {
        Action
        {
            id: individualLineAverage
            text: qsTr("Individual Line")
            checkable: true
            checked: plot.averagingType === PlotAveragingType.Individual
            onTriggered: { plot.averagingType = PlotAveragingType.Individual; }
        }

        Action
        {
            id: meanLineAverage
            text:  qsTr("Mean Line");
            checkable: true
            checked: plot.averagingType === PlotAveragingType.MeanLine
            onTriggered: { plot.averagingType = PlotAveragingType.MeanLine; }
        }

        Action
        {
            id: medianLineAverage
            text: qsTr("Median Line")
            checkable: true
            checked: plot.averagingType === PlotAveragingType.MedianLine
            onTriggered: { plot.averagingType = PlotAveragingType.MedianLine; }
        }

        Action
        {
            id: meanHistogramAverage
            text: qsTr("Mean Histogram")
            checkable: true
            checked: plot.averagingType === PlotAveragingType.MeanHistogram
            onTriggered: { plot.averagingType = PlotAveragingType.MeanHistogram; }
        }

        Action
        {
            id: iqrAverage
            text: qsTr("IQR")
            checkable: true
            checked: plot.averagingType === PlotAveragingType.IQR
            onTriggered: { plot.averagingType = PlotAveragingType.IQR; }
        }
    }

    Action
    {
        id: groupByAnnotation
        text: qsTr("Group By Annotation")
        enabled: plot.visibleColumnAnnotationNames.length > 0 || plot.columnAnnotationSelectionModeEnabled
        checkable: true
        checked: plot.groupByAnnotation

        onTriggered:
        {
            plot.groupByAnnotation = !plot.groupByAnnotation;

            if(plot.groupByAnnotation)
            {
                plot.scaleType = PlotScaleType.Raw;
                plot.averagingType = PlotAveragingType.Individual;
                plot.dispersionType = PlotDispersionType.None;
            }
        }
    }

    ExclusiveGroup
    {
        id: sortByExclusiveGroup
    }

    ExclusiveGroup
    {
        id: sharedValuesAttributeExclusiveGroup
    }

    ExclusiveGroup
    {
        id: scaleByAttributeExclusiveGroup
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
                text: plot.yAxisLabel
            }
        }

        onAccepted: plot.yAxisLabel = yAxisTextField.text;
    }

    Connections
    {
        target: plugin.model
        function onSharedValuesAttributeNamesChanged()
        {
            if(!plugin.model.sharedValuesAttributeNames.includes(
                plot.averagingAttributeName))
            {
                // The averaging attribute doesn't exist any more, so unset it
                plot.averagingAttributeName = "";
            }
        }

        function onNumericalAttributeNamesChanged()
        {
            if(!plugin.model.numericalAttributeNames.includes(
                plot.scaleByAttributeName))
            {
                // The scaling attribute doesn't exist any more, so unset it
                plot.scaleType = PlotScaleType.Raw;
                plot.scaleByAttributeName = "";
            }
        }
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

            let showAllColumnsMenu = menu.addItem("");
            showAllColumnsMenu.action = toggleShowAllColumns;
            showAllColumnsMenu.visible = Qt.binding(function() { return plot.isWide; });

            if(plugin.model.columnAnnotationNames.length > 0)
                menu.addItem("").action = selectColumnAnnotationsAction;

            menu.addItem("").action = toggleGridLines;
            menu.addItem("").action = togglePlotLegend;

            let axisLabels = menu.addMenu(qsTr("Axis Labels"));
            axisLabels.addItem("").action = setXAxisLabelAction;
            axisLabels.addItem("").action = setYAxisLabelAction;
            menu.addSeparator();

            if(plugin.model.numContinuousColumns > 0)
            {
                let scalingMenu = menu.addMenu(qsTr("Scaling"));
                scalingMenu.addItem("").action = rawScaling;
                scalingMenu.addItem("").action = logScaling;
                scalingMenu.addItem("").action = meanCentreScaling;
                scalingMenu.addItem("").action = unitVarianceScaling;
                scalingMenu.addItem("").action = paretoScaling;
                scalingMenu.enabled = Qt.binding(function()
                {
                    return plot.averagingType === PlotAveragingType.Individual || plot.iqrStyle;
                });

                scalingMenu.addSeparator();
                let scaleByAttributeMenu = scalingMenu.addMenu(qsTr("By Attribute"));
                scaleByAttributeMenu.enabled = Qt.binding(function() { return !plot.iqrStyle; });
                plugin.model.numericalAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = scaleByAttributeMenu.addItem(attributeName);

                    attributeMenuItem.exclusiveGroup = scaleByAttributeExclusiveGroup;
                    attributeMenuItem.checkable = true;
                    attributeMenuItem.checked = Qt.binding(function()
                    {
                        if(plot.scaleType !== PlotScaleType.ByAttribute)
                            return false;

                        return attributeName === plot.scaleByAttributeName;
                    });

                    attributeMenuItem.triggered.connect(function()
                    {
                        plot.scaleByAttributeName = attributeName;
                        plot.scaleType = PlotScaleType.ByAttribute;
                    });
                });

                let averagingMenu = menu.addMenu(qsTr("Averaging"));
                averagingMenu.addItem("").action = individualLineAverage;
                averagingMenu.addItem("").action = meanLineAverage;
                averagingMenu.addItem("").action = medianLineAverage;
                averagingMenu.addItem("").action = meanHistogramAverage;
                averagingMenu.addItem("").action = iqrAverage;
                averagingMenu.enabled = Qt.binding(function()
                {
                    return !plot.groupByAnnotation;
                });

                averagingMenu.addSeparator();

                let sharedValuesAttributesMenu = averagingMenu.addMenu(qsTr("By Attribute"));
                sharedValuesAttributesMenu.enabled = Qt.binding(function()
                {
                    return plot.averagingType !== PlotAveragingType.Individual &&
                        plot.averagingType !== PlotAveragingType.IQR &&
                        !plot.groupByAnnotation;
                });
                let allAttributesMenuItem = sharedValuesAttributesMenu.addItem(qsTr("All"));
                allAttributesMenuItem.exclusiveGroup = sharedValuesAttributeExclusiveGroup;
                allAttributesMenuItem.checkable = true;
                allAttributesMenuItem.checked = Qt.binding(function()
                {
                    return plot.averagingAttributeName.length === 0;
                });
                allAttributesMenuItem.triggered.connect(function()
                {
                    plot.averagingAttributeName = "";
                });

                sharedValuesAttributesMenu.addSeparator();

                plugin.model.sharedValuesAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = sharedValuesAttributesMenu.addItem(attributeName);

                    attributeMenuItem.exclusiveGroup = sharedValuesAttributeExclusiveGroup;
                    attributeMenuItem.checkable = true;
                    attributeMenuItem.checked = Qt.binding(function()
                    {
                        return attributeName === plot.averagingAttributeName;
                    });

                    attributeMenuItem.triggered.connect(function()
                    {
                        plot.averagingAttributeName = attributeName;
                    });
                });

                let dispersionMenu = menu.addMenu(qsTr("Dispersion"));
                dispersionMenu.addItem("").action = noDispersion;
                dispersionMenu.addItem("").action = stdDeviations;
                dispersionMenu.addItem("").action = stdErrorDeviations;
                dispersionMenu.addSeparator();
                dispersionMenu.addItem("").action = barDeviationVisual;
                dispersionMenu.addItem("").action = graphDeviationVisual;
                dispersionMenu.enabled = Qt.binding(function()
                {
                    return (plot.averagingType === PlotAveragingType.MeanLine ||
                        plot.averagingType === PlotAveragingType.MeanHistogram) &&
                        !plot.groupByAnnotation;
                });

                menu.addItem("").action = toggleIncludeYZero;
                menu.addItem("").action = resetZoom;

                menu.addSeparator();
            }

            let sortByMenu = menu.addMenu(qsTr("Sort Columns By"));
            root._availableplotColumnSortOptions.forEach(function(sortOption)
            {
                let sortByMenuItem = sortByMenu.addItem(sortOption.text);

                sortByMenuItem.exclusiveGroup = sortByExclusiveGroup;
                sortByMenuItem.checkable = true;
                sortByMenuItem.checked = Qt.binding(function()
                {
                    let columnSortType = PlotColumnSortType.Natural;
                    let columnSortAnnotation = "";
                    if(plot.columnSortOrders.length > 0)
                    {
                        columnSortType = plot.columnSortOrders[0].type;
                        columnSortAnnotation = plot.columnSortOrders[0].text;
                    }

                    if(sortOption.type === columnSortType &&
                        columnSortType === PlotColumnSortType.ColumnAnnotation)
                    {
                        return sortOption.text === columnSortAnnotation;
                    }

                    return sortOption.type === columnSortType;
                });

                sortByMenuItem.triggered.connect(function()
                {
                    plot.sortBy(sortOption.type, sortOption.text);
                });
            });

            if(plugin.model.columnAnnotationNames.length > 0)
                menu.addItem("").action = groupByAnnotation;

            menu.addSeparator();
            menu.addItem("").action = savePlotImageAction;

            Utils.cloneMenu(menu, plotContextMenu);
            return true;
        }

        return false;
    }

    property var _availableColumnAnnotationNames:
    {
        let list = [];

        plugin.model.columnAnnotationNames.forEach(function(columnAnnotationName)
        {
            list.push(columnAnnotationName);
        });

        return list;
    }

    property var _availableplotColumnSortOptions:
    {
        let options =
        [
            {type: PlotColumnSortType.Natural, text: qsTr("Natural Order")},
            {type: PlotColumnSortType.ColumnName, text: qsTr("Column Name")}
        ];

        root._availableColumnAnnotationNames.forEach(function(columnAnnotationName)
        {
            options.push({type: PlotColumnSortType.ColumnAnnotation, text: columnAnnotationName});
        });

        return options;
    }

    SystemPalette { id: systemPalette }

    toolStrip: ToolBar
    {
        RowLayout
        {
            anchors.fill: parent

            ToolButton { action: toggleUiOrientationAction }
            ToolBarSeparator {}
            ToolButton { action: resizeColumnsToContentsAction }
            ToolButton { action: selectColumnsAction }
            ToolButton { action: tableView.exportAction }
            ToolBarSeparator {}
            ToolButton { action: toggleColumnNamesAction }
            ToolButton
            {
                visible: plugin.model.columnAnnotationNames.length > 0
                action: selectColumnAnnotationsAction
            }

            Item { Layout.fillWidth: true }
        }
    }

    SplitView
    {
        id: splitView

        orientation: toggleUiOrientationAction.checked ? Qt.Horizontal : Qt.Vertical

        anchors.fill: parent

        handleDelegate: Rectangle
        {
            width: 8
            height: 8

            color: systemPalette.window

            Rectangle
            {
                anchors.centerIn: parent

                readonly property int maxDimension: 48
                readonly property int minDimension: 4

                width: splitView.orientation === Qt.Horizontal ? minDimension : maxDimension
                height: splitView.orientation === Qt.Horizontal ? maxDimension : minDimension
                radius: minDimension * 0.5
                color: systemPalette.midlight
            }
        }

        NodeAttributeTableView
        {
            id: tableView
            Layout.fillHeight: splitView.orientation === Qt.Vertical
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            Layout.minimumWidth: splitView.orientation === Qt.Horizontal ? 400 : -1

            model: plugin.model.nodeAttributeTableModel

            onSelectedRowsChanged:
            {
                // If the tableView's selection is less than complete, highlight
                // the corresponding nodes in the graph, otherwise highlight nothing
                plugin.model.highlightedRows = tableView.selectedRows.length < rowCount ?
                    tableView.selectedRows : [];
            }

            onSortIndicatorColumnChanged: { root.saveRequired = true; }
            onSortIndicatorOrderChanged: { root.saveRequired = true; }
        }

        CorrelationPlot
        {
            id: plot

            Layout.fillWidth: true
            Layout.fillHeight: splitView.orientation !== Qt.Vertical
            Layout.minimumHeight: splitView.orientation === Qt.Vertical ? minimumHeight : root.minimumHeight
            Layout.minimumWidth: 200

            model: plugin.model
            selectedRows: tableView.selectedRows

            onPlotOptionsChanged: { root.saveRequired = true; }
            onVisibleColumnAnnotationNamesChanged: { root.saveRequired = true; }
            onColumnSortOrdersChanged: { root.saveRequired = true; }

            elideLabelWidth:
            {
                let newHeight = height * 0.25;
                let quant = 20;
                let quantised = Math.floor(newHeight / quant) * quant;

                if(quantised < 40)
                    quantised = 0;

                return quantised;
            }

            property bool iqrStyle: groupByAnnotation || averagingType == PlotAveragingType.IQR

            property bool scrollBarRequired: visibleHorizontalFraction < 1.0
            xAxisPadding: Constants.padding + (scrollBarRequired ? scrollView.__horizontalScrollBar.height : 0)

            horizontalScrollPosition:
            {
                return scrollView.flickableItem.contentX /
                    (scrollView.flickableItem.contentWidth - scrollView.viewport.width);
            }

            onRightClick:
            {
                if(plotContextMenu.enabled)
                    plotContextMenu.popup();
            }

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

            FloatingButton
            {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 4
                visible: plot.columnAnnotationSelectionModeEnabled
                iconName: "emblem-unreadable"

                onClicked:
                {
                    plot.columnAnnotationSelectionModeEnabled =
                        selectColumnAnnotationsAction.checked = false;
                }
            }

            ScrollView
            {
                id: scrollView
                visible: plot.scrollBarRequired
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                anchors.fill: parent
                frameVisible: true

                contentItem: Item
                {
                    // This is a fake item to give the scrollbar the correct range
                    // The maximum size is to prevent OpenGL texture overflow (2^14 pixels)
                    width: Math.min(plot.width / plot.visibleHorizontalFraction, 16383);

                    // This needs to match the viewport height, otherwise the plot doesn't
                    // get vertical mouse wheel events (see the conditions required for
                    // QQuickWheelArea1::wheelEvent to call QWheelEvent::ignore())
                    height: scrollView.viewport.height
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

    function initialise()
    {
        tableView.initialise();
    }

    property bool saveRequired: false

    function save()
    {
        let data =
        {
            "sideBySide": toggleUiOrientationAction.checked,
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder,
            "hiddenColumns": tableView.hiddenColumns,
            "columnOrder": tableView.columnOrder,

            "showColumnNames": plot.showColumnNames,

            "plotScaling": plot.scaleType,
            "plotScaleAttributeName": plot.scaleByAttributeName,
            "plotAveraging": plot.averagingType,
            "plotAveragingAttributeName": plot.averagingAttributeName,
            "plotDispersion": plot.dispersionType,
            "plotDispersionVisual": plot.dispersionVisualType,
            "plotGroupByAnnotation": plot.groupByAnnotation,

            "plotIncludeYZero": plot.includeYZero,
            "plotShowAllColumns": plot.showAllColumns,
            "plotXAxisLabel": plot.xAxisLabel,
            "plotYAxisLabel": plot.yAxisLabel,

            "plotLegend": plot.showLegend,
            "plotGridLines": plot.showGridLines,

            "columnAnnotations": plot.visibleColumnAnnotationNames,
            "plotColumnSortOrders": plot.columnSortOrders
        };

        return data;
    }

    function load(data, version)
    {
        if(version < 6)
        {
            if(data.sortColumn !== undefined && Number.isInteger(data.sortColumn) && data.sortColumn >= 0)
                data.sortColumn = tableView.model.columnNameFor(data.sortColumn);
            else
                data.sortColumn = "";

            if(data.hiddenColumns !== undefined)
                data.hiddenColumns = data.hiddenColumns.map(c => tableView.model.columnNameFor(c));
        }

        if(data.sideBySide !== undefined)                   toggleUiOrientationAction.checked = data.sideBySide;
        if(data.sortColumn !== undefined)                   tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                    tableView.sortIndicatorOrder = data.sortOrder;
        if(data.hiddenColumns !== undefined)                tableView.setHiddenColumns(data.hiddenColumns);
        if(data.columnOrder !== undefined)                  tableView.columnOrder = data.columnOrder;

        if(data.showColumnNames !== undefined)              plot.showColumnNames = data.showColumnNames;

        if(data.plotScaling !== undefined)                  plot.scaleType = data.plotScaling;
        if(data.plotScaleAttributeName !== undefined)       plot.scaleByAttributeName = data.plotScaleAttributeName;
        if(data.plotAveraging !== undefined)                plot.averagingType = data.plotAveraging;
        if(data.plotAveragingAttributeName !== undefined)   plot.averagingAttributeName = data.plotAveragingAttributeName;
        if(data.plotDispersion !== undefined)               plot.dispersionType = data.plotDispersion;
        if(data.plotDispersionVisual !== undefined)         plot.dispersionVisualType = data.plotDispersionVisual;
        if(data.plotGroupByAnnotation !== undefined)        plot.groupByAnnotation = data.plotGroupByAnnotation;

        if(data.plotIncludeYZero !== undefined)             plot.includeYZero = data.plotIncludeYZero;
        if(data.plotShowAllColumns !== undefined)           plot.showAllColumns = data.plotShowAllColumns;
        if(data.plotXAxisLabel !== undefined)               plot.xAxisLabel = data.plotXAxisLabel;
        if(data.plotYAxisLabel !== undefined)               plot.yAxisLabel = data.plotYAxisLabel;

        if(data.plotLegend !== undefined)                   plot.showLegend = data.plotLegend;
        if(data.plotGridLines !== undefined)                plot.showGridLines = data.plotGridLines;

        if(data.columnAnnotations !== undefined)            plot.visibleColumnAnnotationNames = data.columnAnnotations;

        if(data.plotColumnSortType !== undefined)
        {
            // Handle older file format
            let columnSortOrder = {"order": Qt.AscendingOrder, "text": ""};
            columnSortOrder.type = data.plotColumnSortType;

            if(data.plotColumnSortAnnotation !== undefined)
                columnSortOrder.text = data.plotColumnSortAnnotation;

            plot.columnSortOrders.push(columnSortOrder);
        }
        else

        if(data.plotColumnSortOrders !== undefined)         plot.columnSortOrders = data.plotColumnSortOrders;
    }
}
