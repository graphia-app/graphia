/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

    Action
    {
        id: selectColumnAnnotationsAction
        text: qsTr("Select Visible Column Annotations")
        iconName: "column-annotations"

        enabled: plot.canShowColumnAnnotationSelection &&
            plugin.model.columnAnnotationNames.length > 0

        checkable: true
        checked: plot.columnAnnotationSelectionModeEnabled

        onTriggered:
        {
            plot.columnAnnotationSelectionModeEnabled = !plot.columnAnnotationSelectionModeEnabled;
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

    ExclusiveGroup
    {
        id: sortByExclusiveGroup
    }

    ExclusiveGroup
    {
        id: sharedValuesAttributeExclusiveGroup
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
                plot.plotAveragingAttributeName))
            {
                // The averaging attribute doesn't exist any more, so unset it
                plot.plotAveragingAttributeName = "";
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

            var showAllColumnsMenu = menu.addItem("");
            showAllColumnsMenu.action = toggleShowAllColumns;
            showAllColumnsMenu.visible = Qt.binding(function() { return plot.isWide; });

            if(plugin.model.columnAnnotationNames.length > 0)
                menu.addItem("").action = selectColumnAnnotationsAction;

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
                return plot.plotAveragingType === PlotAveragingType.Individual
            });

            var averagingMenu = menu.addMenu(qsTr("Averaging"));
            averagingMenu.addItem("").action = individualLineAverage;
            averagingMenu.addItem("").action = meanLineAverage;
            averagingMenu.addItem("").action = medianLineAverage;
            averagingMenu.addItem("").action = meanHistogramAverage;
            averagingMenu.addItem("").action = iqrAverage;

            averagingMenu.addSeparator();

            var sharedValuesAttributesMenu = averagingMenu.addMenu(qsTr("By Attribute"));
            sharedValuesAttributesMenu.enabled = Qt.binding(function()
            {
                return plot.plotAveragingType !== PlotAveragingType.Individual &&
                    plot.plotAveragingType !== PlotAveragingType.IQRPlot;
            });
            var allAttributesMenuItem = sharedValuesAttributesMenu.addItem(qsTr("All"));
            allAttributesMenuItem.exclusiveGroup = sharedValuesAttributeExclusiveGroup;
            allAttributesMenuItem.checkable = true;
            allAttributesMenuItem.checked = Qt.binding(function()
            {
                return plot.plotAveragingAttributeName.length === 0;
            });
            allAttributesMenuItem.triggered.connect(function()
            {
                plot.plotAveragingAttributeName = "";
            });

            sharedValuesAttributesMenu.addSeparator();

            plugin.model.sharedValuesAttributeNames.forEach(function(attributeName)
            {
                var attributeMenuItem = sharedValuesAttributesMenu.addItem(attributeName);

                attributeMenuItem.exclusiveGroup = sharedValuesAttributeExclusiveGroup;
                attributeMenuItem.checkable = true;
                attributeMenuItem.checked = Qt.binding(function()
                {
                    return attributeName === plot.plotAveragingAttributeName;
                });

                attributeMenuItem.triggered.connect(function()
                {
                    plot.plotAveragingAttributeName = attributeName;
                });
            });

            var dispersionMenu = menu.addMenu(qsTr("Dispersion"));
            dispersionMenu.addItem("").action = noDispersion;
            dispersionMenu.addItem("").action = stdDeviations;
            dispersionMenu.addItem("").action = stdErrorDeviations;
            dispersionMenu.addSeparator();
            dispersionMenu.addItem("").action = barDeviationVisual;
            dispersionMenu.addItem("").action = graphDeviationVisual;
            dispersionMenu.enabled = Qt.binding(function()
            {
                return plot.plotAveragingType === PlotAveragingType.MeanLine ||
                        plot.plotAveragingType === PlotAveragingType.MeanHistogram
            });

            menu.addItem("").action = toggleIncludeYZero;

            menu.addSeparator();
            var sortByMenu = menu.addMenu(qsTr("Sort Columns By"));
            root._availableplotColumnSortOptions.forEach(function(sortOption)
            {
                var sortByMenuItem = sortByMenu.addItem(sortOption.text);

                sortByMenuItem.exclusiveGroup = sortByExclusiveGroup;
                sortByMenuItem.checkable = true;
                sortByMenuItem.checked = Qt.binding(function()
                {
                    if(sortOption.type === plot.columnSortType &&
                        sortOption.type === PlotColumnSortType.ColumnAnnotation)
                    {
                        return sortOption.text === plot.columnSortAnnotation;
                    }

                    return sortOption.type === plot.columnSortType;
                });

                sortByMenuItem.triggered.connect(function()
                {
                    plot.columnSortAnnotation =
                        sortOption.type === PlotColumnSortType.ColumnAnnotation ?
                        sortOption.text : "";

                    plot.columnSortType = sortOption.type;
                });
            });

            menu.addSeparator();
            menu.addItem("").action = savePlotImageAction;

            Utils.cloneMenu(menu, plotContextMenu);
            return true;
        }

        return false;
    }

    property var _availableColumnAnnotationNames:
    {
        var list = [];

        plugin.model.columnAnnotationNames.forEach(function(columnAnnotationName)
        {
            list.push(columnAnnotationName);
        });

        return list;
    }

    property var _availableplotColumnSortOptions:
    {
        var options =
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
            onColumnSortTypeChanged: { root.saveRequired = true; }
            onColumnSortAnnotationChanged: { root.saveRequired = true; }

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

            // On platforms where the scrollbar doesn't auto-hide or appear transparent,
            // we need to provide a little room for it to be visible
            xAxisPadding: Qt.platform.os !== "osx" && scrollBarRequired ? 15 : 0

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

            FloatingButton
            {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 4
                visible: plot.columnAnnotationSelectionModeEnabled
                iconName: "emblem-unreadable"

                onClicked: { plot.columnAnnotationSelectionModeEnabled = false; }
            }

            ScrollView
            {
                id: scrollView
                visible: plot.scrollBarRequired
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                anchors.fill: parent
                frameVisible: true

                Item
                {
                    // This is a fake item to give the scrollbar the correct range
                    // The maximum size is to prevent OpenGL texture overflow (2^14 pixels)
                    width: Math.min(plot.width / plot.visibleHorizontalFraction, 16383);
                    height: 0
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
        tableView.defaultColumnVisibility();
    }

    function save()
    {
        var data =
        {
            "sideBySide": toggleUiOrientationAction.checked,
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder,
            "hiddenColumns": tableView.hiddenColumns,

            "showColumnNames": plot.showColumnNames,

            "plotScaling": plot.plotScaleType,
            "plotAveraging": plot.plotAveragingType,
            "plotAveragingAttributeName": plot.plotAveragingAttributeName,
            "plotDispersion": plot.plotDispersionType,
            "plotDispersionVisual": plot.plotDispersionVisualType,

            "plotIncludeYZero": plot.includeYZero,
            "plotShowAllColumns": plot.showAllColumns,

            "plotLegend": plot.showLegend,
            "plotGridLines": plot.showGridLines,

            "columnAnnotations": plot.visibleColumnAnnotationNames,
            "plotColumnSortType": plot.columnSortType,
            "plotColumnSortAnnotation": plot.columnSortAnnotation
        };

        return data;
    }

    function load(data, version)
    {
        if(data.sideBySide !== undefined)                   toggleUiOrientationAction.checked = data.sideBySide;
        if(data.sortColumn !== undefined)                   tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                    tableView.sortIndicatorOrder = data.sortOrder;
        if(data.hiddenColumns !== undefined)                tableView.setHiddenColumns(data.hiddenColumns);

        if(data.showColumnNames !== undefined)              plot.showColumnNames = data.showColumnNames;

        if(data.plotScaling !== undefined)                  plot.plotScaleType = data.plotScaling;
        if(data.plotAveraging !== undefined)                plot.plotAveragingType = data.plotAveraging;
        if(data.plotAveragingAttributeName !== undefined)   plot.plotAveragingAttributeName = data.plotAveragingAttributeName;
        if(data.plotDispersion !== undefined)               plot.plotDispersionType = data.plotDispersion;
        if(data.plotDispersionVisual !== undefined)         plot.plotDispersionVisualType = data.plotDispersionVisual;

        if(data.plotIncludeYZero !== undefined)             plot.includeYZero = data.plotIncludeYZero;
        if(data.plotShowAllColumns !== undefined)           plot.showAllColumns = data.plotShowAllColumns;

        if(data.plotLegend !== undefined)                   plot.showLegend = data.plotLegend;
        if(data.plotGridLines !== undefined)                plot.showGridLines = data.plotGridLines;

        if(data.columnAnnotations !== undefined)            plot.visibleColumnAnnotationNames = data.columnAnnotations;
        if(data.plotColumnSortType !== undefined)           plot.columnSortType = data.plotColumnSortType;
        if(data.plotColumnSortAnnotation !== undefined)     plot.columnSortAnnotation = data.plotColumnSortAnnotation;
    }
}
