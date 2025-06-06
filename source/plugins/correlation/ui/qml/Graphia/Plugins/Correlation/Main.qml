/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import Graphia.Controls
import Graphia.Plugins
import Graphia.Utils

PluginContent
{
    id: root

    anchors.fill: parent
    minimumHeight: 320

    Preferences
    {
        id: misc
        section: "misc"

        property var fileOpenInitialFolder
    }

    Action
    {
        id: toggleUiOrientationAction
        text: qsTr("Display UI &Side By Side")
        icon.name: "view-refresh"
        checkable: true
        checked: true

        onCheckedChanged: function(checked) { root.saveRequired = true; }
    }

    Action
    {
        id: toggleColumnNamesAction
        text: qsTr("Show &Column Names")
        icon.name: "format-text-bold"
        checkable: true
        checked: plot.showColumnNames

        onTriggered: function(source)
        {
            plot.showAllColumns = false;
            plot.showColumnNames = !plot.showColumnNames;
        }
    }

    Action
    {
        id: findRowsOfInterestAction
        text: qsTr("Find Rows Of Interest")
        icon.name: "edit-find"

        enabled: plugin.model.numContinuousColumns > 0

        checkable: true

        onTriggered: function(source)
        {
            root.togglePlotMode(PlotMode.RowsOfInterestColumnSelection);
        }
    }

    Action
    {
        id: selectColumnAnnotationsAction
        text: qsTr("Select Visible Column Annotations")
        icon.name: "column-annotations"

        enabled: plugin.model.columnAnnotationNames.length > 0

        checkable: true

        onTriggered: function(source)
        {
            root.togglePlotMode(PlotMode.ColumnAnnotationSelection);
        }
    }

    function setPlotMode(mode)
    {
        let hideControls = (plot.plotMode !== PlotMode.Normal) && (mode === PlotMode.Normal);

        plot.plotMode = mode;

        selectColumnAnnotationsAction.checked =
            (plot.plotMode === PlotMode.ColumnAnnotationSelection);
        findRowsOfInterestAction.checked =
            (plot.plotMode === PlotMode.RowsOfInterestColumnSelection);

        if(hideControls)
            modalControls.hide();
        else
            modalControls.show();
    }

    function togglePlotMode(mode)
    {
        if(plot.plotMode === mode)
        {
            root.setPlotMode(PlotMode.Normal);
            return;
        }

        root.setPlotMode(mode);
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

        onTriggered: function(source)
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
            id: antiLogScaling
            text:  qsTr("AntiLog");
            checkable: true
            checked: plot.scaleType === PlotScaleType.AntiLog
            onTriggered: { plot.scaleType = PlotScaleType.AntiLog; }
        }

        Action
        {
            id: meanCentreScaling
            text: qsTr("Mean Centre Scaling")
            enabled: plot.complexScalingEnabled
            checkable: true
            checked: plot.scaleType === PlotScaleType.MeanCentre
            onTriggered: { plot.scaleType = PlotScaleType.MeanCentre; }
        }

        Action
        {
            id: unitVarianceScaling
            text: qsTr("Unit Variance Scaling")
            enabled: plot.complexScalingEnabled
            checkable: true
            checked: plot.scaleType === PlotScaleType.UnitVariance
            onTriggered: { plot.scaleType = PlotScaleType.UnitVariance; }
        }

        Action
        {
            id: paretoScaling
            text: qsTr("Pareto Scaling")
            enabled: plot.complexScalingEnabled
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

        onTriggered: function(source)
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

    Component
    {
        id: importAnnotationsDialog
        ImportAnnotationsDialog { pluginModel: plugin.model }
    }

    OpenFileDialog
    {
        id: importAnnotationsFileOpenDialog
        nameFilters:
        [
            "All Files (*.csv *.tsv *.ssv *.xlsx)",
            "CSV Files (*.csv)",
            "TSV Files (*.tsv)",
            "SSV Files (*.ssv)",
            "Excel Files (*.xlsx)"
        ]

        onAccepted:
        {
            misc.fileOpenInitialFolder = currentFolder.toString();
            let instance = Utils.createWindow(root, importAnnotationsDialog, {}, false);
            Utils.centreWindow(instance);
            instance.open(selectedFile);
        }
    }

    Action
    {
        id: importAnnotationsAction
        text: qsTr("Import Annotations From Table…")
        onTriggered: function(source)
        {
            if(misc.fileOpenInitialFolder !== undefined)
                importAnnotationsFileOpenDialog.currentFolder = misc.fileOpenInitialFolder;

            importAnnotationsFileOpenDialog.open();
        }
    }

    Action
    {
        id: savePlotImageAction
        enabled: plot.selectedRows.length > 0

        function showSaveImageDialog(onAcceptedFn)
        {
            let folder = screenshot.path !== undefined ? screenshot.path : "";
            let path = Utils.format(qsTr("{0}/{1}-correlation-plot"),
                NativeUtils.fileNameForUrl(folder),
                root.baseFileNameNoExtension);

            savePlotImageFileDialog.currentFolder = folder;
            savePlotImageFileDialog.selectedFile = NativeUtils.urlForFileName(path);
            savePlotImageFileDialog.onAcceptedFn = onAcceptedFn;
            savePlotImageFileDialog.open();
        }

        text: qsTr("Save As &Image…")
        icon.name: "camera-photo"
        onTriggered:
        {
            showSaveImageDialog(function(filename, extension)
            {
                plot.savePlotImage(filename, extension);
            });
        }
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

        standardButtons: Dialog.Ok | Dialog.Cancel

        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: qsTr("Please enter the X Axis label:")
                color: palette.buttonText
            }
            TextField
            {
                id: xAxisTextField
                Layout.bottomMargin: 12
                implicitWidth: 150
                selectByMouse: true
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

        standardButtons: Dialog.Ok | Dialog.Cancel

        RowLayout
        {
            Text
            {
                Layout.bottomMargin: 12
                text: qsTr("Please enter the Y Axis label:")
                color: palette.buttonText
            }
            TextField
            {
                id: yAxisTextField
                Layout.bottomMargin: 12
                implicitWidth: 150
                selectByMouse: true
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

        function onColumnAnnotationNamesChanged(added, removed)
        {
            if(added.length > 0)
                plot.showColumnAnnotations(added);

            if(removed.length > 0)
                plot.hideColumnAnnotations(removed);
        }

        function onColumnAnnotationValuesChanged()
        {
            plot.refreshColumnAnnotations();
        }
    }

    property var _plotMenuStateUpdateFns: []

    function addCheckedStateFnFor(checkable, isChecked)
    {
        _plotMenuStateUpdateFns.push(function() { checkable.checked = isChecked(); });
    }

    function updatePlotMenuState()
    {
        for(let plotMenuStateUpdateFn of _plotMenuStateUpdateFns)
            plotMenuStateUpdateFn();
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            tableView.populateTableMenu(menu);
            return true;

        case 1:
            root._plotMenuStateUpdateFns = [];

            menu.title = qsTr("&Plot");
            MenuUtils.addActionTo(menu, toggleColumnNamesAction);

            let showAllColumnsMenuItem = MenuUtils.addActionTo(menu, toggleShowAllColumns);
            showAllColumnsMenuItem.hidden = Qt.binding(() => !plot.isWide);

            if(plugin.model.numContinuousColumns > 0)
                MenuUtils.addActionTo(menu, findRowsOfInterestAction);

            if(plugin.model.columnAnnotationNames.length > 0)
                MenuUtils.addActionTo(menu, selectColumnAnnotationsAction);

            MenuUtils.addActionTo(menu, toggleGridLines);
            MenuUtils.addActionTo(menu, togglePlotLegend);

            let axisLabelsMenu = MenuUtils.addSubMenuTo(menu, qsTr("Axis Labels"));
            MenuUtils.addActionTo(axisLabelsMenu, setXAxisLabelAction);
            MenuUtils.addActionTo(axisLabelsMenu, setYAxisLabelAction);
            MenuUtils.addSeparatorTo(menu);

            if(plugin.model.numContinuousColumns > 0)
            {
                let scalingMenu = MenuUtils.addSubMenuTo(menu, qsTr("Scaling"));
                MenuUtils.addActionTo(scalingMenu, rawScaling);
                MenuUtils.addActionTo(scalingMenu, logScaling);
                MenuUtils.addActionTo(scalingMenu, antiLogScaling);
                MenuUtils.addActionTo(scalingMenu, meanCentreScaling);
                MenuUtils.addActionTo(scalingMenu, unitVarianceScaling);
                MenuUtils.addActionTo(scalingMenu, paretoScaling);

                MenuUtils.addSeparatorTo(scalingMenu);

                let scaleByAttributeMenu = MenuUtils.addSubMenuTo(scalingMenu, qsTr("By Attribute"));
                scaleByAttributeMenu.enabled = Qt.binding(() => plot.complexScalingEnabled);
                plugin.model.numericalAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = MenuUtils.addItemTo(scaleByAttributeMenu, attributeName);

                    attributeMenuItem.checkable = true;
                    root.addCheckedStateFnFor(attributeMenuItem, () =>
                        plot.scaleType === PlotScaleType.ByAttribute &&
                        attributeName === plot.scaleByAttributeName);

                    attributeMenuItem.triggered.connect(function()
                    {
                        plot.scaleByAttributeName = attributeName;
                        plot.scaleType = PlotScaleType.ByAttribute;
                    });
                });

                let averagingMenu = MenuUtils.addSubMenuTo(menu, qsTr("Averaging"));
                MenuUtils.addActionTo(averagingMenu, individualLineAverage);
                MenuUtils.addActionTo(averagingMenu, meanLineAverage);
                MenuUtils.addActionTo(averagingMenu, medianLineAverage);
                MenuUtils.addActionTo(averagingMenu, meanHistogramAverage);
                MenuUtils.addActionTo(averagingMenu, iqrAverage);
                averagingMenu.enabled = Qt.binding(() => !plot.groupByAnnotation);

                MenuUtils.addSeparatorTo(averagingMenu);

                let sharedValuesAttributesMenu = MenuUtils.addSubMenuTo(averagingMenu, qsTr("By Attribute"));
                sharedValuesAttributesMenu.enabled = Qt.binding(() =>
                    plot.averagingType !== PlotAveragingType.Individual &&
                    plot.averagingType !== PlotAveragingType.IQR &&
                    !plot.groupByAnnotation);
                let allAttributesMenuItem = MenuUtils.addItemTo(sharedValuesAttributesMenu, qsTr("All"));
                allAttributesMenuItem.checkable = true;
                root.addCheckedStateFnFor(allAttributesMenuItem, () =>
                    plot.averagingAttributeName.length === 0);
                allAttributesMenuItem.triggered.connect(function()
                {
                    plot.averagingAttributeName = "";
                });

                MenuUtils.addSeparatorTo(sharedValuesAttributesMenu);

                plugin.model.sharedValuesAttributeNames.forEach(function(attributeName)
                {
                    let attributeMenuItem = MenuUtils.addItemTo(sharedValuesAttributesMenu, attributeName);

                    attributeMenuItem.checkable = true;
                    root.addCheckedStateFnFor(attributeMenuItem, () =>
                        attributeName === plot.averagingAttributeName);

                    attributeMenuItem.triggered.connect(function()
                    {
                        plot.averagingAttributeName = attributeName;
                    });
                });

                let dispersionMenu = MenuUtils.addSubMenuTo(menu, qsTr("Dispersion"));
                MenuUtils.addActionTo(dispersionMenu, noDispersion);
                MenuUtils.addActionTo(dispersionMenu, stdDeviations);
                MenuUtils.addActionTo(dispersionMenu, stdErrorDeviations);
                MenuUtils.addSeparatorTo(dispersionMenu);
                MenuUtils.addActionTo(dispersionMenu, barDeviationVisual);
                MenuUtils.addActionTo(dispersionMenu, graphDeviationVisual);
                dispersionMenu.enabled = Qt.binding(() =>
                    (plot.averagingType === PlotAveragingType.MeanLine ||
                        plot.averagingType === PlotAveragingType.MeanHistogram) &&
                    !plot.groupByAnnotation);

                MenuUtils.addActionTo(menu, toggleIncludeYZero);

                if(plot.iqrStyle)
                    MenuUtils.addActionTo(menu, toggleShowOutliers);

                MenuUtils.addActionTo(menu, resetZoom);

                MenuUtils.addSeparatorTo(menu);
            }

            let sortByMenu = MenuUtils.addSubMenuTo(menu, qsTr("Sort Columns By"));
            let addedSortBySeparator = false;

            root._availableplotColumnSortOptions.forEach(function(sortOption)
            {
                if(sortOption.type === PlotColumnSortType.ColumnAnnotation && !addedSortBySeparator)
                {
                    MenuUtils.addSeparatorTo(sortByMenu);
                    addedSortBySeparator = true;
                }

                let sortByMenuItem = MenuUtils.addItemTo(sortByMenu, sortOption.text);

                sortByMenuItem.checkable = true;
                root.addCheckedStateFnFor(sortByMenuItem, function()
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
                    sortByMenuItem.hidden = Qt.binding(() => plot.groupByAnnotation);

                let sortFn = function()
                {
                    plot.sortBy(sortOption.type,
                        sortOption.type === PlotColumnSortType.ColumnAnnotation ?
                        sortOption.text : "");
                };

                if(sortOption.type === PlotColumnSortType.HierarchicalClustering)
                {
                    sortByMenuItem.triggered.connect(function()
                    {
                        // Reset the menu state (and thus uncheck the clustering item)
                        // in case the clustering is cancelled by the user
                        root.updatePlotMenuState();

                        plugin.model.computeHierarchicalClustering();
                    });

                    plugin.model.hierarchicalClusteringComplete.connect(sortFn);
                }
                else
                    sortByMenuItem.triggered.connect(sortFn);
            });

            if(plugin.model.columnAnnotationNames.length > 0)
            {
                MenuUtils.addActionTo(menu, groupByAnnotationAction);

                let colorGroupsByMenu = MenuUtils.addSubMenuTo(menu, qsTr("Colour Groups By"));
                colorGroupsByMenu.hidden = Qt.binding(() => !plot.groupByAnnotation);

                let colorByNoGroupMenuItem = MenuUtils.addItemTo(colorGroupsByMenu, qsTr("None"));
                colorByNoGroupMenuItem.checkable = true;
                root.addCheckedStateFnFor(colorByNoGroupMenuItem, () =>
                    plot.colorGroupByAnnotationName.length === 0);

                colorByNoGroupMenuItem.triggered.connect(function()
                {
                    plot.colorGroupByAnnotationName = "";
                });

                plot.visibleColumnAnnotationNames.forEach(function(columnAnnotationName)
                {
                    let colorGroupsByMenuItem = MenuUtils.addItemTo(colorGroupsByMenu, columnAnnotationName);

                    colorGroupsByMenuItem.checkable = true;
                    root.addCheckedStateFnFor(colorGroupsByMenuItem, () =>
                        columnAnnotationName === plot.colorGroupByAnnotationName);

                    colorGroupsByMenuItem.triggered.connect(function()
                    {
                        plot.colorGroupByAnnotationName = columnAnnotationName;
                    });
                });
            }

            MenuUtils.addActionTo(menu, importAnnotationsAction);

            MenuUtils.addSeparatorTo(menu);
            MenuUtils.addActionTo(menu, savePlotImageAction);

            let savePlotImageAttributeMenu = MenuUtils.addSubMenuTo(menu, qsTr("Save As Images By…"));
            savePlotImageAttributeMenu.enabled = Qt.binding(() => plot.selectedRows.length > 0);

            let savePlotImageByRowMenuItem = MenuUtils.addItemTo(savePlotImageAttributeMenu, qsTr("Row"));
            savePlotImageByRowMenuItem.triggered.connect(function()
            {
                savePlotImageAction.showSaveImageDialog(function(filename, extension)
                {
                    plot.savePlotImageByRow(filename, extension);
                });
            });

            if(plugin.model.sharedValuesAttributeNames.length > 0)
            {
                MenuUtils.addSeparatorTo(savePlotImageAttributeMenu);
                plugin.model.sharedValuesAttributeNames.forEach(function(attributeName)
                {
                    let savePlotImageByAttributeMenuItem = MenuUtils.addItemTo(savePlotImageAttributeMenu, attributeName);

                    savePlotImageByAttributeMenuItem.triggered.connect(function()
                    {
                        savePlotImageAction.showSaveImageDialog(function(filename, extension)
                        {
                            plot.savePlotImageByAttribute(filename, extension, attributeName);
                        });
                    });
                });
            }

            // Initial update
            root.updatePlotMenuState();

            MenuUtils.clone(menu, plotContextMenu);
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
        ];

        let continuousOptions =
        [
            {type: PlotColumnSortType.DataValue, text: qsTr("Data Value")},
            {type: PlotColumnSortType.HierarchicalClustering, text: qsTr("Hierarchical Clustering")}
        ];

        if(plugin.model.numContinuousColumns > 0)
            options = options.concat(continuousOptions);

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

            ToolBarButton { action: toggleUiOrientationAction }
            ToolBarSeparator {}
            ToolBarButton { action: tableView.resizeColumnsAction }
            ToolBarButton { action: tableView.selectAction }
            ToolBarButton { action: tableView.exportAction }
            ToolBarButton { action: tableView.focusAction }
            ToolBarSeparator {}
            ToolBarButton { action: toggleColumnNamesAction }
            ToolBarButton
            {
                visible: findRowsOfInterestAction.enabled
                action: findRowsOfInterestAction
            }
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

        handle: SplitViewHandle {}

        NodeAttributeTableView
        {
            id: tableView
            SplitView.fillHeight: splitView.orientation === Qt.Vertical
            SplitView.minimumHeight: splitView.orientation === Qt.Vertical ? 100 : -1
            SplitView.minimumWidth: splitView.orientation === Qt.Horizontal ? 400 : -1

            model: plugin.model.nodeAttributeTableModel
            pluginContent: root

            exportBaseFileName: root.baseFileNameNoExtension + "-attributes"

            onVisibleRowsChanged:
            {
                if(plot.plotMode === PlotMode.RowsOfInterestColumnSelection && plot.selectedColumns.length > 0)
                    plot.selectRowsOfInterest();
                else
                    tableView.selectAll();
            }

            onSelectedRowsChanged:
            {
                // If the tableView's selection is less than complete, highlight
                // the corresponding nodes in the graph, otherwise highlight nothing
                plugin.model.highlightedRows = tableView.selectedRows.length < tableView.visibleRows.length ?
                    tableView.selectedRows : [];
            }

            onAboutToCrop: { root.setPlotMode(PlotMode.Normal); }

            onSortIndicatorColumnChanged: { root.saveRequired = true; }
            onSortIndicatorOrderChanged: { root.saveRequired = true; }
            onHiddenColumnsChanged: { root.saveRequired = true; }
        }

        Item
        {
            // This Item exists solely as a parent for the Flickable's vertical scrollbar
            // See https://doc.qt.io/qt-6/qml-qtquick-controls2-scrollbar.html#attaching-scrollbar-to-a-flickable

            SplitView.fillWidth: true
            SplitView.fillHeight: splitView.orientation !== Qt.Vertical
            SplitView.minimumHeight: 150 // Should be <= the minimum that CorrelationPlot::minimumHeight returns
            SplitView.minimumWidth: 200

            Flickable
            {
                id: plotFlickable
                anchors.fill: parent

                contentHeight: Math.max(height, plot.minimumHeight)
                clip: true
                interactive: false
                boundsBehavior: Flickable.StopAtBounds

                MouseArea
                {
                    anchors.fill: parent
                    onWheel: function(wheel)
                    {
                        plotFlickable.flick(0.0, wheel.angleDelta.y * 4.0);

                        if(wheel.angleDelta.x < 0.0)
                            horizontalPlotScrollBar.increase();
                        else if(wheel.angleDelta.x > 0.0)
                            horizontalPlotScrollBar.decrease();
                    }
                }

                ScrollBar.vertical: ScrollBar
                {
                    id: verticalPlotScrollBar

                    parent: plotFlickable.parent
                    anchors.top: plotFlickable.top
                    anchors.bottom: plotFlickable.bottom
                    anchors.bottomMargin: plotFlickable.horizontalScrollBarHeight
                    anchors.right: plotFlickable.right

                    policy: ScrollBar.AsNeeded
                }

                readonly property int horizontalScrollBarHeight:
                    horizontalPlotScrollBar.size < 1 ? horizontalPlotScrollBar.height : 0
                readonly property int verticalScrollBarWidth:
                    ScrollBar.vertical.size < 1 ? ScrollBar.vertical.width : 0

                CorrelationPlot
                {
                    id: plot

                    anchors.fill: parent

                    rightPadding: plotFlickable.verticalScrollBarWidth
                    bottomPadding: plotFlickable.horizontalScrollBarHeight

                    model: plugin.model
                    selectedRows: tableView.selectedRows

                    onPlotOptionsChanged:
                    {
                        root.updatePlotMenuState();
                        root.saveRequired = true;
                    }

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

                    property bool complexScalingEnabled: plot.averagingType === PlotAveragingType.Individual

                    horizontalScrollPosition: horizontalPlotScrollBar.position / (1.0 - horizontalPlotScrollBar.size)

                    property var _lastSelectedRowsOfInterest: []
                    property bool selectedRowsDifferFromRowsOfInterest:
                        plot.plotMode === PlotMode.RowsOfInterestColumnSelection &&
                        plot._lastSelectedRowsOfInterest.length > 0 &&
                        !Utils.arraysMatch(tableView.selectedRows, plot._lastSelectedRowsOfInterest)

                    function selectRowsOfInterest()
                    {
                        _lastSelectedRowsOfInterest = [];

                        if(plot.plotMode !== PlotMode.RowsOfInterestColumnSelection || plot.selectedColumns.length === 0)
                            return;

                        let rows = plugin.model.rowsOfInterestByColumns(plot.selectedColumns,
                            tableView.visibleRows, modalControls.roiPercentile, modalControls.roiWeight);

                        tableView.setRowOrder(rows);
                        tableView.clearAndSelectRows(rows);
                        _lastSelectedRowsOfInterest = rows;
                    }

                    onPlotModeChanged:
                    {
                        if(plot.plotMode !== PlotMode.RowsOfInterestColumnSelection)
                            _lastSelectedRowsOfInterest = [];
                    }

                    onSelectedColumnsChanged: { plot.selectRowsOfInterest(); }

                    onRightClick:
                    {
                        if(plotContextMenu.enabled)
                            plotContextMenu.popup();
                    }

                    DelayedBusyIndicator
                    {
                        visible: parent.visible
                        anchors.centerIn: parent
                        width: 64
                        height: 64

                        delayedRunning: plot.busy
                    }

                    Rectangle
                    {
                        implicitWidth: roiInstructionsLayout.implicitWidth
                        implicitHeight: roiInstructionsLayout.implicitHeight

                        color: ControlColors.background
                        border.width: 1
                        border.color: ControlColors.outline
                        radius: 5

                        Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                        opacity: plot.plotMode === PlotMode.RowsOfInterestColumnSelection &&
                            plot.selectedColumns.length === 0 ? 1.0 : 0.0

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 48

                        ColumnLayout
                        {
                            id: roiInstructionsLayout
                            anchors.centerIn: parent

                            Text
                            {
                                Layout.margins: Constants.margin
                                Layout.preferredWidth: 256
                                wrapMode: Text.WordWrap
                                color: palette.buttonText

                                text: qsTr("Click on a column or a column annotation " +
                                    "to select. Hold <i>Ctrl</i> and click to select multiple columns.")
                            }
                        }
                    }

                    SlidingPanel
                    {
                        id: modalControlsPanel

                        alignment: Qt.AlignTop|Qt.AlignLeft

                        anchors.left: parent.left
                        anchors.top: parent.top

                        // Have the button move sync with scrolling, so it's always visible
                        anchors.topMargin: plotFlickable.contentY

                        initiallyOpen: false
                        disableItemWhenClosed: false

                        item: PlotModalControls
                        {
                            id: modalControls

                            plot: plot

                            onShown: { modalControlsPanel.show(); }
                            onHidden: { modalControlsPanel.hide(); }
                            onClosed: { root.setPlotMode(PlotMode.Normal); }
                        }
                    }

                    onColumnSortOrderCanBePinnedChanged:
                    {
                        pinnedButton.checked = false;

                        if(columnSortOrderCanBePinned)
                            pinningPanel.show();
                        else
                            pinningPanel.hide();
                    }

                    columnSortOrderPinned: pinnedButton.checked

                    SlidingPanel
                    {
                        id: pinningPanel

                        alignment: Qt.AlignTop|Qt.AlignRight

                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: Constants.margin

                        initiallyOpen: false
                        disableItemWhenClosed: false

                        item: FloatingButton
                        {
                            id: pinnedButton

                            checkable: true
                            icon.name: "pin"

                            ToolTip.visible: hovered
                            ToolTip.delay: Constants.toolTipDelay
                            ToolTip.text: qsTr("Fix Data Value Sort Order")
                        }
                    }

                    ScrollBar
                    {
                        id: horizontalPlotScrollBar

                        parent: plotFlickable
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: plotFlickable.verticalScrollBarWidth
                        anchors.bottom: parent.bottom

                        policy: plot.visibleHorizontalFraction < 1.0 ?
                            ScrollBar.AsNeeded : ScrollBar.AlwaysOff

                        hoverEnabled: true
                        active: hovered || pressed
                        orientation: Qt.Horizontal
                        size: plot.visibleHorizontalFraction
                        stepSize: size * 0.1
                    }
                }
            }

            ScrollBarCornerFiller
            {
                horizontalScrollBar: horizontalPlotScrollBar
                verticalScrollBar: verticalPlotScrollBar
            }
        }
    }

    PlatformMenu { id: plotContextMenu }

    SaveFileDialog
    {
        id: savePlotImageFileDialog

        title: qsTr("Save Plot As Image")
        nameFilters: [qsTr("PNG Image (*.png)"), qsTr("JPEG Image (*.jpg *.jpeg)"), qsTr("PDF Document (*.pdf)")]

        property var onAcceptedFn: null

        onAccepted:
        {
            screenshot.path = savePlotImageFileDialog.currentFolder.toString();
            savePlotImageFileDialog.onAcceptedFn(savePlotImageFileDialog.selectedFile,
                savePlotImageFileDialog.selectedNameFilter.extensions[0]);
        }
    }

    Preferences
    {
        id: screenshot
        section: "screenshot"

        property var path
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
