/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15

import Qt.labs.platform 1.0 as Labs

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

PluginContent
{
    id: root

    anchors.fill: parent
    minimumHeight: 320

    Action
    {
        id: toggleUiOrientationAction
        text: qsTr("Display UI &Side By Side")
        icon.name: "view-refresh"
        checkable: true
        checked: true

        onCheckedChanged: { root.saveRequired = true; }
    }

    Action
    {
        id: toggleColumnNamesAction
        text: qsTr("Show &Column Names")
        icon.name: "format-text-bold"
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
        id: selectColumnAnnotationsAction
        text: qsTr("Select Visible Column Annotations")
        icon.name: "column-annotations"

        enabled: plugin.model.columnAnnotationNames.length > 0

        checkable: true

        onTriggered:
        {
            plot.columnAnnotationSelectionModeEnabled = !plot.columnAnnotationSelectionModeEnabled;
            checked = plot.columnAnnotationSelectionModeEnabled;
        }
    }

    Action
    {
        id: toggleGridLines
        text: qsTr("Grid Lines")
        icon.name: "grid-lines"

        checkable: true
        checked: plot.showGridLines

        onTriggered: { plot.showGridLines = !plot.showGridLines; }
    }

    Action
    {
        id: togglePlotLegend
        text: qsTr("Legend")
        icon.name: "format-justify-left"

        checkable: true
        checked: plot.showLegend

        onTriggered: { plot.showLegend = !plot.showLegend; }
    }

    Action
    {
        id: toggleIncludeYZero
        text: qsTr("Include Zero In Y Axis")
        icon.name: "include-zero"

        checkable: true
        checked: plot.includeYZero

        onTriggered: { plot.includeYZero = !plot.includeYZero; }
    }

    Action
    {
        id: toggleShowOutliers
        text: qsTr("Show Outliers")
        icon.name: "show-outliers"

        enabled: plot.iqrStyle

        checkable: true
        checked: plot.showIqrOutliers

        onTriggered: { plot.showIqrOutliers = !plot.showIqrOutliers; }
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

    ActionGroup
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

    ActionGroup
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

    ActionGroup
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

    ActionGroup
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
        id: groupByAnnotationAction
        text: qsTr("Group By Annotation")
        icon.name: "group-by"

        enabled: plot.visibleColumnAnnotationNames.length > 0
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
            else
                plot.colorGroupByAnnotationName = "";
        }
    }

    ButtonGroup { id: sortByButtonGroup }
    ButtonGroup { id: colorByButtonGroup }
    ButtonGroup { id: sharedValuesAttributeButtonGroup }
    ButtonGroup { id: scaleByAttributeButtonGroup }

    Action
    {
        id: savePlotImageAction
        text: qsTr("Save As &Image…")
        icon.name: "camera-photo"
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
        modal: true
        anchors.centerIn: parent
        title: qsTr("X Axis Label")

        standardButtons: StandardButton.Ok | StandardButton.Cancel

        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: qsTr("Please enter the X Axis label:")
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
        modal: true
        anchors.centerIn: parent
        title: qsTr("Y Axis Label")

        standardButtons: StandardButton.Ok | StandardButton.Cancel

        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: qsTr("Please enter the Y Axis label:")
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
            Utils.addActionTo(menu, toggleColumnNamesAction);

            let showAllColumnsMenuItem = Utils.addActionTo(menu, toggleShowAllColumns);
            Utils.setMenuItemVisibleFunction(showAllColumnsMenuItem, () => plot.isWide);

            if(plugin.model.columnAnnotationNames.length > 0)
                Utils.addActionTo(menu, selectColumnAnnotationsAction);

            Utils.addActionTo(menu, toggleGridLines);
            Utils.addActionTo(menu, togglePlotLegend);

            let axisLabelsMenu = Utils.addSubMenuTo(menu, qsTr("Axis Labels"));
            Utils.addActionTo(axisLabelsMenu, setXAxisLabelAction);
            Utils.addActionTo(axisLabelsMenu, setYAxisLabelAction);
            Utils.addSeparatorTo(menu);

            if(plugin.model.numContinuousColumns > 0)
            {
                let scalingMenu = Utils.addSubMenuTo(menu, qsTr("Scaling"));
                Utils.addActionTo(scalingMenu, rawScaling);
                Utils.addActionTo(scalingMenu, logScaling);
                Utils.addActionTo(scalingMenu, meanCentreScaling);
                Utils.addActionTo(scalingMenu, unitVarianceScaling);
                Utils.addActionTo(scalingMenu, paretoScaling);
                scalingMenu.enabled = Qt.binding(() =>
                    plot.averagingType === PlotAveragingType.Individual || plot.iqrStyle);

                Utils.addSeparatorTo(scalingMenu);

                let scaleByAttributeMenu = Utils.addSubMenuTo(scalingMenu, qsTr("By Attribute"));
                scaleByAttributeMenu.enabled = Qt.binding(() => !plot.iqrStyle);
                plugin.model.numericalAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = Utils.addItemTo(scaleByAttributeMenu, attributeName);

                    attributeMenuItem.ButtonGroup.group = scaleByAttributeButtonGroup;
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

                let averagingMenu = Utils.addSubMenuTo(menu, qsTr("Averaging"));
                Utils.addActionTo(averagingMenu, individualLineAverage);
                Utils.addActionTo(averagingMenu, meanLineAverage);
                Utils.addActionTo(averagingMenu, medianLineAverage);
                Utils.addActionTo(averagingMenu, meanHistogramAverage);
                Utils.addActionTo(averagingMenu, iqrAverage);
                averagingMenu.enabled = Qt.binding(() => !plot.groupByAnnotation);

                Utils.addSeparatorTo(averagingMenu);

                let sharedValuesAttributesMenu = Utils.addSubMenuTo(averagingMenu, qsTr("By Attribute"));
                sharedValuesAttributesMenu.enabled = Qt.binding(() =>
                    plot.averagingType !== PlotAveragingType.Individual &&
                    plot.averagingType !== PlotAveragingType.IQR &&
                    !plot.groupByAnnotation);
                let allAttributesMenuItem = Utils.addItemTo(sharedValuesAttributesMenu, qsTr("All"));
                allAttributesMenuItem.ButtonGroup.group = sharedValuesAttributeButtonGroup;
                allAttributesMenuItem.checkable = true;
                allAttributesMenuItem.checked = Qt.binding(() => plot.averagingAttributeName.length === 0);
                allAttributesMenuItem.triggered.connect(function()
                {
                    plot.averagingAttributeName = "";
                });

                Utils.addSeparatorTo(sharedValuesAttributesMenu);

                plugin.model.sharedValuesAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = Utils.addItemTo(sharedValuesAttributesMenu, attributeName);

                    attributeMenuItem.ButtonGroup.group = sharedValuesAttributeButtonGroup;
                    attributeMenuItem.checkable = true;
                    attributeMenuItem.checked = Qt.binding(() => attributeName === plot.averagingAttributeName);

                    attributeMenuItem.triggered.connect(function()
                    {
                        plot.averagingAttributeName = attributeName;
                    });
                });

                let dispersionMenu = Utils.addSubMenuTo(menu, qsTr("Dispersion"));
                Utils.addActionTo(dispersionMenu, noDispersion);
                Utils.addActionTo(dispersionMenu, stdDeviations);
                Utils.addActionTo(dispersionMenu, stdErrorDeviations);
                Utils.addSeparatorTo(dispersionMenu);
                Utils.addActionTo(dispersionMenu, barDeviationVisual);
                Utils.addActionTo(dispersionMenu, graphDeviationVisual);
                dispersionMenu.enabled = Qt.binding(() =>
                    (plot.averagingType === PlotAveragingType.MeanLine ||
                        plot.averagingType === PlotAveragingType.MeanHistogram) &&
                    !plot.groupByAnnotation);

                Utils.addActionTo(menu, toggleIncludeYZero);

                if(plot.iqrStyle)
                    Utils.addActionTo(menu, toggleShowOutliers);

                Utils.addActionTo(menu, resetZoom);

                Utils.addSeparatorTo(menu);
            }

            let sortByMenu = Utils.addSubMenuTo(menu, qsTr("Sort Columns By"));
            root._availableplotColumnSortOptions.forEach(function(sortOption)
            {
                let sortByMenuItem = Utils.addItemTo(sortByMenu, sortOption.text);

                sortByMenuItem.ButtonGroup.group = sortByButtonGroup;
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

                if(sortOption.type === PlotColumnSortType.DataValue)
                    Utils.setMenuItemVisibleFunction(sortByMenuItem, () => !plot.groupByAnnotation);

                sortByMenuItem.triggered.connect(function()
                {
                    plot.sortBy(sortOption.type, sortOption.text);
                });
            });

            if(plugin.model.columnAnnotationNames.length > 0)
            {
                Utils.addActionTo(menu, groupByAnnotationAction);

                let colorGroupsByMenu = Utils.addSubMenuTo(menu, qsTr("Colour Groups By"));
                colorGroupsByMenu.visible = Qt.binding(() => groupByAnnotationAction.checked);

                let colorByNoGroupMenuItem = Utils.addItemTo(colorGroupsByMenu, qsTr("None"));
                colorByNoGroupMenuItem.ButtonGroup.group = colorByButtonGroup;
                colorByNoGroupMenuItem.checkable = true;
                colorByNoGroupMenuItem.checked = Qt.binding(() => plot.colorGroupByAnnotationName.length === 0);

                colorByNoGroupMenuItem.triggered.connect(function()
                {
                    plot.colorGroupByAnnotationName = "";
                });

                plot.visibleColumnAnnotationNames.forEach(function(columnAnnotationName)
                {
                    let colorGroupsByMenuItem = Utils.addItemTo(colorGroupsByMenu, columnAnnotationName);

                    colorGroupsByMenuItem.ButtonGroup.group = colorByButtonGroup;
                    colorGroupsByMenuItem.checkable = true;
                    colorGroupsByMenuItem.checked = Qt.binding(() => columnAnnotationName === plot.colorGroupByAnnotationName);

                    colorGroupsByMenuItem.triggered.connect(function()
                    {
                        plot.colorGroupByAnnotationName = columnAnnotationName;
                    });
                });
            }

            Utils.addSeparatorTo(menu);
            Utils.addActionTo(menu, savePlotImageAction);

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
            {type: PlotColumnSortType.ColumnName, text: qsTr("Column Name")},
            {type: PlotColumnSortType.DataValue, text: qsTr("Data Value")}
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

            ToolBarButton { action: toggleUiOrientationAction }
            ToolBarSeparator {}
            ToolBarButton { action: tableView.resizeColumnsAction }
            ToolBarButton { action: tableView.selectAction }
            ToolBarButton { action: tableView.exportAction }
            ToolBarSeparator {}
            ToolBarButton { action: toggleColumnNamesAction }
            ToolBarButton
            {
                visible: selectColumnAnnotationsAction.enabled
                action: selectColumnAnnotationsAction
            }
            ToolBarButton { action: toggleGridLines }
            ToolBarButton { action: togglePlotLegend }
            ToolBarButton { action: toggleIncludeYZero }
            ToolBarButton
            {
                visible: groupByAnnotationAction.enabled
                action: groupByAnnotationAction
            }
            ToolBarButton
            {
                visible: toggleShowOutliers.enabled
                action: toggleShowOutliers
            }
            ToolBarButton { action: savePlotImageAction }

            Item { Layout.fillWidth: true }
        }
    }

    SplitView
    {
        id: splitView

        orientation: toggleUiOrientationAction.checked ? Qt.Horizontal : Qt.Vertical

        anchors.fill: parent

        handle: Rectangle
        {
            implicitWidth: 8
            implicitHeight: 8

            color: systemPalette.window

            Rectangle
            {
                anchors.centerIn: parent

                readonly property int maxDimension: 48
                readonly property int minDimension: 4

                implicitWidth: splitView.orientation === Qt.Horizontal ? minDimension : maxDimension
                implicitHeight: splitView.orientation === Qt.Horizontal ? maxDimension : minDimension
                radius: minDimension * 0.5
                color: systemPalette.midlight
            }
        }


        NodeAttributeTableView
        {
            id: tableView
            SplitView.fillHeight: splitView.orientation === Qt.Vertical
            SplitView.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            SplitView.minimumWidth: splitView.orientation === Qt.Horizontal ? 400 : -1

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

        Flickable
        {
            id: plotFlickable

            SplitView.fillWidth: true
            SplitView.fillHeight: splitView.orientation !== Qt.Vertical
            SplitView.minimumHeight: 150 // Should be <= the minimum that CorrelationPlot::minimumHeight returns
            SplitView.minimumWidth: 200

            contentHeight: Math.max(height, plot.minimumHeight)
            clip: true

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            CorrelationPlot
            {
                id: plot

                anchors.fill: parent

                model: plugin.model
                selectedRows: tableView.selectedRows

                onPlotOptionsChanged: { root.saveRequired = true; }

                onVisibleColumnAnnotationNamesChanged:
                {
                    root.saveRequired = true;

                    if(plot.visibleColumnAnnotationNames.indexOf(plot.colorGroupByAnnotationName) < 0)
                        plot.colorGroupByAnnotationName = "";

                    if(plot.groupByAnnotation && plot.visibleColumnAnnotationNames.length === 0)
                        groupByAnnotationAction.trigger();

                    updateMenu();
                }

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

                property bool iqrStyle: plot.groupByAnnotation || plot.averagingType === PlotAveragingType.IQR
                onIqrStyleChanged: { updateMenu(); }

                horizontalScrollPosition: horizontalPlotScrollBar.position / (1.0 - horizontalPlotScrollBar.size)

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

                    // Have the button move sync with scrolling, so it's always visible
                    anchors.topMargin: 4 + plotFlickable.contentY
                    anchors.margins: 4

                    visible: plot.columnAnnotationSelectionModeEnabled
                    iconName: "emblem-unreadable"

                    onClicked:
                    {
                        plot.columnAnnotationSelectionModeEnabled =
                            selectColumnAnnotationsAction.checked = false;
                    }
                }

                ScrollBar
                {
                    id: horizontalPlotScrollBar
                    parent: plotFlickable
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    policy: plot.visibleHorizontalFraction < 1.0 ?
                        ScrollBar.AsNeeded : ScrollBar.AlwaysOff

                    hoverEnabled: true
                    active: hovered || pressed
                    orientation: Qt.Horizontal
                    size: plot.visibleHorizontalFraction
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
            "plotColorGroupByAnnotationName": plot.colorGroupByAnnotationName,

            "plotIncludeYZero": plot.includeYZero,
            "plotShowIqrOutliers": plot.showIqrOutliers,
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

        if(data.sideBySide !== undefined)                       toggleUiOrientationAction.checked = data.sideBySide;
        if(data.sortColumn !== undefined)                       tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                        tableView.sortIndicatorOrder = data.sortOrder;
        if(data.hiddenColumns !== undefined)                    tableView.setHiddenColumns(data.hiddenColumns);
        if(data.columnOrder !== undefined)                      tableView.columnOrder = data.columnOrder;

        if(data.showColumnNames !== undefined)                  plot.showColumnNames = data.showColumnNames;

        if(data.plotScaling !== undefined)                      plot.scaleType = data.plotScaling;
        if(data.plotScaleAttributeName !== undefined)           plot.scaleByAttributeName = data.plotScaleAttributeName;
        if(data.plotAveraging !== undefined)                    plot.averagingType = data.plotAveraging;
        if(data.plotAveragingAttributeName !== undefined)       plot.averagingAttributeName = data.plotAveragingAttributeName;
        if(data.plotDispersion !== undefined)                   plot.dispersionType = data.plotDispersion;
        if(data.plotDispersionVisual !== undefined)             plot.dispersionVisualType = data.plotDispersionVisual;
        if(data.plotGroupByAnnotation !== undefined)            plot.groupByAnnotation = data.plotGroupByAnnotation;
        if(data.plotColorGroupByAnnotationName !== undefined)   plot.colorGroupByAnnotationName = data.plotColorGroupByAnnotationName;

        if(data.plotIncludeYZero !== undefined)                 plot.includeYZero = data.plotIncludeYZero;
        if(data.plotShowIqrOutliers !== undefined)              plot.showIqrOutliers = data.plotShowIqrOutliers;
        if(data.plotShowAllColumns !== undefined)               plot.showAllColumns = data.plotShowAllColumns;
        if(data.plotXAxisLabel !== undefined)                   plot.xAxisLabel = data.plotXAxisLabel;
        if(data.plotYAxisLabel !== undefined)                   plot.yAxisLabel = data.plotYAxisLabel;

        if(data.plotLegend !== undefined)                       plot.showLegend = data.plotLegend;
        if(data.plotGridLines !== undefined)                    plot.showGridLines = data.plotGridLines;

        if(data.columnAnnotations !== undefined)                plot.visibleColumnAnnotationNames = data.columnAnnotations;

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

        if(data.plotColumnSortOrders !== undefined)             plot.columnSortOrders = data.plotColumnSortOrders;
    }
}
