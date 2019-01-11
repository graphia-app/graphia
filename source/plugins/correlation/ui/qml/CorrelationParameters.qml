import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import com.kajeka 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "Controls"

BaseParameterDialog
{
    id: root
    //FIXME These should be set automatically by Wizard
    minimumWidth: 700
    minimumHeight: 500
    property int selectedRow: -1
    property int selectedCol: -1
    property bool _clickedCell: false

    modality: Qt.ApplicationModal

    // Work around for QTBUG-58594
    function resizeColumnsToContentsBugWorkaround(tableView)
    {
        for(var i = 0; i < tableView.columnCount; ++i)
        {
            var col = tableView.getColumn(i);
            var header = tableView.__listView.headerItem.headerRepeater.itemAt(i);
            if(col)
            {
                col.__index = i;
                col.resizeToContents();
                if(col.width < header.implicitWidth)
                    col.width = header.implicitWidth;
            }
        }
    }

    function isInsideRect(x, y, rect)
    {
        return  x >= rect.x &&
                x < rect.x + rect.width &&
                y >= rect.y &&
                y < rect.y + rect.height;
    }

    function scrollToCell(tableView, x, y)
    {
        if(x > 0)
            x--;
        if(y > 0)
            y--;

        var runningWidth = 0;
        for(var i = 0; i < x; ++i)
        {
            var col = tableView.getColumn(i);
            var header = tableView.__listView.headerItem.headerRepeater.itemAt(i);
            if(col !== null)
                runningWidth += col.width;
        }
        var runningHeight = 0;
        for(i = 0; i < y; ++i)
            runningHeight += dataRectView.__listView.contentItem.children[1].height;
        dataFrameAnimationX.to = Math.min(runningWidth,
                                          tableView.flickableItem.contentWidth -
                                          tableView.flickableItem.width);
        // Pre-2017 ECMA doesn't have Math.clamp...
        dataFrameAnimationX.to = Math.max(dataFrameAnimationX.to, 0);

        // Only animate if we need to
        if(tableView.flickableItem.contentX !== dataFrameAnimationX.to)
            dataFrameAnimationX.running = true;

        dataFrameAnimationY.to = Math.min(runningHeight,
                                          tableView.flickableItem.contentHeight -
                                          tableView.flickableItem.height);
        dataFrameAnimationY.to = Math.max(dataFrameAnimationY.to, 0);

        if(tableView.flickableItem.contentY !== dataFrameAnimationY.to)
            dataFrameAnimationY.running = true;
    }

    function scrollToDataRect()
    {
        if(listTabView.currentItem === dataRectPage)
        {
            Qt.callLater(scrollToCell, dataRectView,
                tabularDataParser.dataRect.x, tabularDataParser.dataRect.y);
        }
    }

    TabularDataParser
    {
        id: tabularDataParser

        minimumCorrelation: minimumCorrelationSpinBox.value
        scalingType: { return scaling.model.get(scaling.currentIndex).value; }
        normaliseType: { return normalise.model.get(normalise.currentIndex).value; }
        missingDataType: { return missingDataType.model.get(missingDataType.currentIndex).value; }
        replacementValue: replacementConstant.text

        onDataRectChanged:
        {
            parameters.dataFrame = dataRect;
            if(!isInsideRect(selectedCol, selectedRow, dataRect) &&
                selectedCol >= 0 && selectedRow >= 0)
            {
                scrollToDataRect();

                tooltipNonNumerical.visible = _clickedCell;
                _clickedCell = false;
            }
        }

        onDataLoaded:
        {
            listTabView.repopulateTableView();
            parameters.data = tabularDataParser.data;
        }
    }

    ColumnLayout
    {
        anchors.centerIn: parent
        visible: !tabularDataParser.complete

        Text { text: qsTr("Loading ") + QmlUtils.baseFileNameForUrl(fileUrl) + "…" }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            ProgressBar
            {
                value: tabularDataParser.progress >= 0.0 ? tabularDataParser.progress / 100.0 : 0.0
                indeterminate: tabularDataParser.progress < 0.0
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked:
                {
                    tabularDataParser.cancelParse();
                    root.close();
                }
            }
        }
    }

    ListTabView
    {
        id: listTabView
        anchors.fill: parent
        visible: tabularDataParser.complete

        onListTabChanged:
        {
            if(currentItem == dataRectPage)
                scrollToDataRect();
        }

        ListTab
        {
            title: qsTr("Introduction")
            ColumnLayout
            {
                width: parent.width
                anchors.left: parent.left
                anchors.right: parent.right

                Text
                {
                    text: qsTr("<h2>Correlation Graph Analysis</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                RowLayout
                {
                    Text
                    {
                        text: qsTr("The correlation plugin creates graphs based on the similarity between variables in a numerical dataset.<br>" +
                                   "<br>" +
                                   "The <a href=\"https://en.wikipedia.org/wiki/Pearson_correlation_coefficient\">Pearson Correlation coefficient</a> " +
                                   "provides this similarity metric. " +
                                   "If specified, the input data can be scaled and normalised, after which correlation scores are " +
                                   "used to determine whether or not an edge is created between the nodes representing rows of data.<br>" +
                                   "<br>" +
                                   "The edges may be filtered using transforms once the graph has been created.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true

                        onLinkActivated: Qt.openUrlExternally(link);
                    }

                    Image
                    {
                        Layout.minimumWidth: 100
                        Layout.minimumHeight: 100
                        sourceSize.width: 100
                        sourceSize.height: 100
                        source: "../plots.svg"
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Data Selection")
            id: dataRectPage
            property bool _busy: tabularDataParser.busy || listTabView.animating

            Component
            {
                id: columnComponent
                TableViewColumn { width: 200 }
            }

            ColumnLayout
            {
                anchors.fill: parent

                Text
                {
                    text: qsTr("<h2>Data Selection - Select and adjust</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("A contiguous numerical dataframe will automatically be selected from your dataset. " +
                               "If you would like to adjust the dataframe, select the new starting cell below.")
                }

                RowLayout
                {
                    CheckBox
                    {
                        id: transposeCheckBox

                        text: qsTr("Transpose Dataset")
                        onCheckedChanged:
                        {
                            parameters.transpose = checked;
                            tabularDataParser.transposed = checked;
                        }
                    }
                    HelpTooltip
                    {
                        title: qsTr("Transpose")
                        Text
                        {
                            wrapMode: Text.WordWrap
                            text: qsTr("Transpose will flip the data over its diagonal. " +
                                       "Moving rows to columns and vice versa.")
                        }
                    }
                }

                Text
                {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("<b>Note:</b> Dataframes will always end at the last cell of the input.")
                }

                TableView
                {
                    id: dataRectView
                    headerVisible: false
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: tabularDataParser.model
                    selectionMode: SelectionMode.NoSelection
                    enabled: !dataRectPage._busy

                    PropertyAnimation
                    {
                        id: dataFrameAnimationX
                        target: dataRectView.flickableItem
                        easing.type: Easing.InOutQuad
                        property: "contentX"
                        to: 0
                        duration: 750
                        onRunningChanged:
                        {
                            if(running)
                                dataRectView.enabled = false;
                            else if (!dataFrameAnimationY.running && !dataFrameAnimationX.running)
                                dataRectView.enabled = true;
                        }
                    }

                    PropertyAnimation
                    {
                        id: dataFrameAnimationY
                        target: dataRectView.flickableItem
                        easing.type: Easing.InOutQuad
                        property: "contentY"
                        to: 0
                        duration: 750
                        onRunningChanged:
                        {
                            if(running)
                                dataRectView.enabled = false;
                            else if (!dataFrameAnimationY.running && !dataFrameAnimationX.running)
                                dataRectView.enabled = true;
                        }
                    }

                    Rectangle
                    {
                        id: tooltipNonNumerical
                        color: sysPalette.light
                        border.color: sysPalette.mid
                        border.width: 1
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 25
                        anchors.horizontalCenter: parent.horizontalCenter
                        implicitWidth: messageText.width + 5
                        implicitHeight: messageText.height + 5
                        onVisibleChanged:
                        {
                            if(visible)
                                nonNumericalTimer.start();
                        }
                        visible: false

                        Timer
                        {
                            id: nonNumericalTimer
                            interval: 5000
                            onTriggered: { tooltipNonNumerical.visible = false; }
                        }

                        Text
                        {
                            anchors.centerIn: parent
                            id: messageText
                            text: qsTr("Selected frame contains non-numerical data. " +
                                       "Next availaible frame selected.");
                        }
                    }

                    BusyIndicator
                    {
                        id: busyIndicator
                        anchors.centerIn: parent
                        running: dataRectPage._busy
                    }

                    SystemPalette
                    {
                        id: sysPalette
                    }

                    itemDelegate: Item
                    {
                        // Based on Qt source for BaseTableView delegate
                        height: Math.max(16, label.implicitHeight)
                        property int implicitWidth: label.implicitWidth + 16
                        clip: true

                        property var isInDataFrame:
                        {
                            return isInsideRect(styleData.column, styleData.row, tabularDataParser.dataRect);
                        }

                        Rectangle
                        {
                            Rectangle
                            {
                                anchors.right: parent.right
                                height: parent.height
                                width: 1
                                color: isInDataFrame ? sysPalette.light : sysPalette.mid
                            }

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    if(styleData.column === tabularDataParser.model.MAX_COLUMNS)
                                        return;
                                    tooltipNonNumerical.visible = false;
                                    nonNumericalTimer.stop();
                                    selectedCol = styleData.column;
                                    selectedRow = styleData.row;
                                    _clickedCell = true;
                                    tabularDataParser.autoDetectDataRectangle(styleData.column, styleData.row);
                                }
                            }

                            width: parent.width
                            anchors.centerIn: parent
                            height: parent.height
                            color: isInDataFrame ? "lightblue" : "transparent"

                            Text
                            {
                                id: label
                                objectName: "label"
                                width: parent.width
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: styleData.hasOwnProperty("depth") && styleData.column === 0 ? 0 :
                                                     horizontalAlignment === Text.AlignRight ? 1 : 8
                                anchors.rightMargin: (styleData.hasOwnProperty("depth") && styleData.column === 0)
                                                     || horizontalAlignment !== Text.AlignRight ? 1 : 8
                                horizontalAlignment: styleData.textAlignment
                                anchors.verticalCenter: parent.verticalCenter
                                elide: styleData.elideMode

                                text:
                                {
                                    if(styleData.column >= tabularDataParser.model.MAX_COLUMNS)
                                    {
                                        if(styleData.row === 0)
                                        {
                                            return (tabularDataParser.model.columnCount() - tabularDataParser.model.MAX_COLUMNS) +
                                                    qsTr(" more columns…");
                                        }
                                        else
                                        {
                                            return "…";
                                        }
                                    }

                                    if(styleData.value === undefined)
                                        return "";

                                    return styleData.value;
                                }

                                color: styleData.textColor
                                renderType: Text.NativeRendering
                            }
                        }
                    }
                }

                Connections
                {
                    target: tabularDataParser.model

                    onModelReset:
                    {
                        listTabView.repopulateTableView();
                        selectedCol = 0;
                        selectedRow = 0;
                        tabularDataParser.autoDetectDataRectangle();
                    }
                }
            }
        }

        function repopulateTableView()
        {
            while(dataRectView.columnCount > 0)
                dataRectView.removeColumn(dataRectView.columnCount - 1);

            dataRectView.model = null;
            dataRectView.model = tabularDataParser.model;
            for(var i = 0; i < tabularDataParser.model.columnCount(); i++)
            {
                dataRectView.addColumn(columnComponent.createObject(dataRectView,
                                                                    {"role": i}));

                // FIXME - TY QT TABLEVIEW 1.
                // https://bugreports.qt.io/browse/QTBUG-70069
                // Cap the column count since a huge number of columns causes a large slowdown
                if(i == tabularDataParser.model.MAX_COLUMNS - 1)
                {
                    // Add a blank
                    dataRectView.addColumn(columnComponent.createObject(dataRectView));
                    break;
                }
            }
            // Qt.callLater is used as the TableView will not generate the columns until
            // next loop has passed and there's no way to reliably listen for the change
            // (Thanks TableView)
            Qt.callLater(resizeColumnsToContentsBugWorkaround, dataRectView);
        }

        ListTab
        {
            title: qsTr("Correlation")
            ColumnLayout
            {
                anchors.fill: parent

                Text
                {
                    text: qsTr("<h2>Correlation - Adjust Thresholds</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                ColumnLayout
                {
                    Layout.fillHeight: true

                    spacing: 10

                    Text
                    {
                        text: qsTr("A Pearson Correlation will be performed on the dataset to provide a measure of correlation between rows of data. " +
                                   "1.0 represents highly correlated rows and 0.0 represents no correlation. Negative correlation values are discarded. " +
                                   "All values below the minimum correlation value will also be discarded and will not be included in the generated graph.<br>" +
                                   "<br>" +
                                   "By default a transform is created which will create edges for all values above the initial correlation threshold. " +
                                   "Is is not possible to create edges using values below the minimum correlation value.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                    }

                    RowLayout
                    {
                        Layout.fillWidth: true

                        Text { text: qsTr("Minimum:") }

                        SpinBox
                        {
                            id: minimumCorrelationSpinBox

                            implicitWidth: 70

                            minimumValue: 0.0
                            maximumValue: 1.0

                            decimals: 3
                            stepSize: Utils.incrementForRange(minimumValue, maximumValue);

                            onValueChanged:
                            {
                                parameters.minimumCorrelation = value;

                                // When the minimum value is increased beyond the initial
                                // value, the latter can get (visually) lost against the extreme
                                // left of the plot, so just punt it over a bit
                                var range = maximumValue - value;
                                var adjustedInitial = value + (range * 0.1);

                                if(initialCorrelationSpinBox.value <= adjustedInitial)
                                    initialCorrelationSpinBox.value = adjustedInitial;
                            }
                        }

                        Slider
                        {
                            id: minimumSlider

                            Layout.fillWidth: true
                            Layout.minimumWidth: 50
                            Layout.maximumWidth: 175

                            minimumValue: 0.0
                            maximumValue: 1.0
                            value: minimumCorrelationSpinBox.value
                            onValueChanged:
                            {
                                minimumCorrelationSpinBox.value = value;
                            }
                        }

                        HelpTooltip
                        {
                            title: qsTr("Minimum Correlation Value")
                            Text
                            {
                                wrapMode: Text.WordWrap
                                text: qsTr("The minimum correlation value above which an edge " +
                                           "will be created in the graph. Using a lower minimum value will " +
                                           "increase the compute and memory requirements.")
                            }
                        }

                        Text { text: qsTr("Initial:") }

                        SpinBox
                        {
                            id: initialCorrelationSpinBox

                            implicitWidth: 70

                            minimumValue:
                            {
                                if(tabularDataParser.graphSizeEstimate.keys !== undefined)
                                    return tabularDataParser.graphSizeEstimate.keys[0];

                                return minimumCorrelationSpinBox.value;
                            }

                            maximumValue: 1.0

                            decimals: 3
                            stepSize: Utils.incrementForRange(minimumValue, maximumValue);

                            onValueChanged:
                            {
                                parameters.initialThreshold = value;
                                graphSizeEstimatePlot.threshold = value;
                            }
                        }

                        HelpTooltip
                        {
                            title: qsTr("Correlation Threshold")
                            Text
                            {
                                wrapMode: Text.WordWrap
                                text: qsTr("The initial correlation threshold determines the size of the resultant graph. " +
                                           "A lower value filters fewer edges, and results in a larger graph. " +
                                           "This value can be changed later, after the graph has been created.")
                            }
                        }
                    }

                    GraphSizeEstimatePlot
                    {
                        id: graphSizeEstimatePlot

                        visible: tabularDataParser.graphSizeEstimate.keys !== undefined
                        graphSizeEstimate: tabularDataParser.graphSizeEstimate

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onThresholdChanged:
                        {
                            initialCorrelationSpinBox.value = threshold;
                        }

                        property bool _timedBusy: false

                        Timer
                        {
                            id: busyIndicationTimer
                            interval: 250
                            repeat: false
                            onTriggered:
                            {
                                if(tabularDataParser.graphSizeEstimateInProgress)
                                    graphSizeEstimatePlot._timedBusy = true;
                            }
                        }

                        Connections
                        {
                            target: tabularDataParser

                            onGraphSizeEstimateInProgressChanged:
                            {
                                if(!tabularDataParser.graphSizeEstimateInProgress)
                                {
                                    busyIndicationTimer.stop();
                                    graphSizeEstimatePlot._timedBusy = false;
                                }
                                else
                                    busyIndicationTimer.start();
                            }
                        }

                        BusyIndicator
                        {
                            anchors.centerIn: parent
                            width: { return Math.min(64, parent.width); }
                            height: { return Math.min(64, parent.height); }

                            visible: graphSizeEstimatePlot._timedBusy
                        }
                    }

                    Text
                    {
                        visible: !graphSizeEstimatePlot.visible

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: qsTr("Empty Graph")
                        font.italic: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Data Manipulation")
            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right

                Text
                {
                    text: qsTr("<h2>Data Manipulation</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }
                ColumnLayout
                {
                    spacing: 10

                    Text
                    {
                        text: qsTr("Please select the required data manipulation. The manipulation " +
                                   "will occur in the order it is displayed below. Resultant changes " +
                                   "in the estimated graph size may be observed on the Correlation tab.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    GridLayout
                    {
                        columns: 3
                        Text
                        {
                            text: qsTr("Imputation:")
                            Layout.alignment: Qt.AlignLeft
                        }

                        ComboBox
                        {
                            id: missingDataType
                            Layout.alignment: Qt.AlignRight
                            model: ListModel
                            {
                                ListElement { text: qsTr("None");               value: MissingDataType.None }
                                ListElement { text: qsTr("Constant");           value: MissingDataType.Constant }
                                ListElement { text: qsTr("Row Interpolate");    value: MissingDataType.RowInterpolation }
                                ListElement { text: qsTr("Column Mean");        value: MissingDataType.ColumnAverage }
                            }
                            textRole: "text"

                            onCurrentIndexChanged:
                            {
                                parameters.missingDataType = model.get(currentIndex).value;
                            }
                        }

                        HelpTooltip
                        {
                            title: qsTr("Imputation")
                            GridLayout
                            {
                                columns: 2
                                Text
                                {
                                    text: qsTr("<b>None:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("All empty or missing values will be treated as zeroes.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>Constant:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Replace all missing values with a constant.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>Row Interpolate:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Linearly interpolate missing values using the nearest surrounding" +
                                               " values in the row and their relative positions. If only one surrounding value" +
                                               " is available the value will be set to match." +
                                               " The value will be set to zero if the row is empty.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>Column Mean:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Replace missing values with the mean value of their column, excluding missing values.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        RowLayout
                        {
                            Layout.columnSpan: 3
                            Layout.alignment: Qt.AlignHCenter
                            visible: missingDataType.currentText === qsTr("Constant")

                            Text
                            {
                                text: qsTr("Value:")
                                Layout.alignment: Qt.AlignLeft
                            }

                            TextField
                            {
                                id: replacementConstant
                                validator: DoubleValidator{}

                                onTextChanged:
                                {
                                    parameters.missingDataValue = text;
                                }

                                text: "0.0"
                            }
                        }

                        Text
                        {
                            text: qsTr("Scaling:")
                            Layout.alignment: Qt.AlignLeft
                        }

                        ComboBox
                        {
                            id: scaling
                            Layout.alignment: Qt.AlignRight
                            model: ListModel
                            {
                                ListElement { text: qsTr("None");          value: ScalingType.None }
                                ListElement { text: qsTr("Log2(x + ε)");   value: ScalingType.Log2 }
                                ListElement { text: qsTr("Log10(x + ε)");  value: ScalingType.Log10 }
                                ListElement { text: qsTr("AntiLog2(x)");   value: ScalingType.AntiLog2 }
                                ListElement { text: qsTr("AntiLog10(x)");  value: ScalingType.AntiLog10 }
                                ListElement { text: qsTr("Arcsin(x)");     value: ScalingType.ArcSin }
                            }
                            textRole: "text"

                            onCurrentIndexChanged:
                            {
                                parameters.scaling = model.get(currentIndex).value;
                            }
                        }

                        HelpTooltip
                        {
                            title: qsTr("Scaling Types")
                            GridLayout
                            {
                                columns: 2
                                rowSpacing: 10

                                Text
                                {
                                    text: qsTr("<b>Log</b><i>b</i>(<i>x</i> + ε):")
                                    Layout.alignment: Qt.AlignTop
                                    textFormat: Text.StyledText
                                }

                                Text
                                {
                                    text: qsTr("Perform a Log of <i>x</i> + ε to base <i>b</i>, where <i>x</i> is the input data and ε is a very small constant.");
                                    wrapMode: Text.WordWrap
                                    Layout.alignment: Qt.AlignTop
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>AntiLog</b><i>b</i>(<i>x</i>):")
                                    Layout.alignment: Qt.AlignTop
                                    textFormat: Text.StyledText
                                }

                                Text
                                {
                                    text: qsTr("Raise <i>x</i> to the power of <i>b</i>, where <i>x</i> is the input data.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>Arcsin</b>(<i>x</i>):")
                                    Layout.alignment: Qt.AlignTop
                                    textFormat: Text.StyledText
                                }

                                Text
                                {
                                    text: qsTr("Perform an inverse sine function of <i>x</i>, where <i>x</i> is the input data. This is useful when " +
                                        "you require a log transform but the dataset contains negative numbers or zeros.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }

                        }

                        Text
                        {
                            text: qsTr("Normalisation:")
                            Layout.alignment: Qt.AlignLeft
                        }

                        ComboBox
                        {
                            id: normalise
                            Layout.alignment: Qt.AlignRight
                            model: ListModel
                            {
                                ListElement { text: qsTr("None");       value: NormaliseType.None }
                                ListElement { text: qsTr("MinMax");     value: NormaliseType.MinMax }
                                ListElement { text: qsTr("Quantile");   value: NormaliseType.Quantile }
                            }
                            textRole: "text"

                            onCurrentIndexChanged:
                            {
                                parameters.normalise = model.get(currentIndex).value;
                            }
                        }

                        HelpTooltip
                        {
                            title: qsTr("Normalisation Types")
                            GridLayout
                            {
                                columns: 2
                                Text
                                {
                                    text: qsTr("<b>MinMax:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Normalise the data so 1.0 is the maximum value " +
                                               "of that column and 0.0 the minimum. " +
                                               "This is useful if the columns in the dataset " +
                                               "have differing scales or units. " +
                                               "Note: If all elements in a column have the same value " +
                                               "this will rescale the values to 0.0.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text
                                {
                                    text: qsTr("<b>Quantile:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Normalise the data so that the columns have equal distributions.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Initial Transforms")
            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                Text
                {
                    text: qsTr("<h2>Graph Transforms</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text: qsTr("Commonly used transforms can be automatically added to " +
                               "the graph here.")
                    Layout.alignment: Qt.AlignLeft
                    wrapMode: Text.WordWrap
                }

                GridLayout
                {
                    columns: 3
                    Text
                    {
                        text: qsTr("Clustering:")
                        Layout.alignment: Qt.AlignLeft
                    }

                    ComboBox
                    {
                        id: clustering
                        Layout.alignment: Qt.AlignRight
                        model: ListModel
                        {
                            ListElement { text: qsTr("None");  value: ClusteringType.None }
                            ListElement { text: qsTr("MCL");   value: ClusteringType.MCL }
                        }
                        textRole: "text"

                        onCurrentIndexChanged:
                        {
                            parameters.clusteringType = model.get(currentIndex).value;
                        }
                    }

                    HelpTooltip
                    {
                        title: qsTr("Clustering")
                        GridLayout
                        {
                            columns: 2
                            Text
                            {
                                text: qsTr("<b>MCL:</b>")
                                textFormat: Text.StyledText
                                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            }

                            Text
                            {
                                text: qsTr("The Markov Clustering Algorithm simulates stochastic " +
                                           "flow within the generated graph to identify distinct " +
                                           "clusters of potentially related data points.");
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    Text
                    {
                        text: qsTr("Edge Reduction:")
                        Layout.alignment: Qt.AlignLeft
                    }

                    ComboBox
                    {
                        id: edgeReduction
                        Layout.alignment: Qt.AlignRight
                        model: ListModel
                        {
                            ListElement { text: qsTr("None");  value: EdgeReductionType.None }
                            ListElement { text: qsTr("k-NN");   value: EdgeReductionType.KNN }
                        }
                        textRole: "text"

                        onCurrentIndexChanged:
                        {
                            parameters.edgeReductionType = model.get(currentIndex).value;
                        }
                    }

                    HelpTooltip
                    {
                        title: qsTr("Edge Reduction")
                        ColumnLayout
                        {
                            Text
                            {
                                text: qsTr("Edge-reduction attempts to reduce unnecessary or non-useful edges")
                                textFormat: Text.StyledText
                                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            }

                            GridLayout
                            {
                                columns: 2
                                Text
                                {
                                    text: qsTr("<b>k-NN:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("k-nearest neighbours ranks node edges and only " +
                                        "keeps <i>k</i> number of edges per node.");
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Summary")
            ColumnLayout
            {
                anchors.fill: parent

                Text
                {
                    text: qsTr("<h2>Summary</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    text: qsTr("A graph will be created with the following parameters:")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                TextArea
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readOnly: true
                    textFormat: TextEdit.RichText

                    text:
                    {
                        var summaryString = "";
                        summaryString += qsTr("Minimum Correlation Value: ") + minimumCorrelationSpinBox.value + "<br>";
                        summaryString += qsTr("Initial Correlation Threshold: ") + initialCorrelationSpinBox.value + "<br>";

                        if(!tabularDataParser.dataRect !== Qt.rect(0, 0, 0, 0))
                        {
                            summaryString += qsTr("Data Frame:") +
                                qsTr(" [ Column: ") + tabularDataParser.dataRect.x +
                                qsTr(" Row: ") + tabularDataParser.dataRect.y +
                                qsTr(" Width: ") + tabularDataParser.dataRect.width +
                                qsTr(" Height: ") + tabularDataParser.dataRect.height + " ]<br>";
                        }
                        else
                        {
                            summaryString += qsTr("Data Frame: Automatic<br>");
                        }

                        summaryString += qsTr("Data Transpose: ") + transposeCheckBox.checked + "<br>";
                        summaryString += qsTr("Data Scaling: ") + scaling.currentText + "<br>";
                        summaryString += qsTr("Data Normalise: ") + normalise.currentText + "<br>";
                        summaryString += qsTr("Missing Data Replacement: ") + missingDataType.currentText + "<br>";

                        if(missingDataType.model.get(missingDataType.currentIndex).value === MissingDataType.Constant)
                            summaryString += qsTr("Replacement Constant: ") + replacementConstant.text + "<br>";

                        var transformString = ""
                        if(clustering.model.get(clustering.currentIndex).value !== ClusteringType.None)
                            transformString += qsTr("Clustering (") + clustering.currentText + ")<br>";

                        if(edgeReduction.model.get(edgeReduction.currentIndex).value !== EdgeReductionType.None)
                            transformString += qsTr("Edge Reduction (") + edgeReduction.currentText + ")<br>";

                        if(!transformString)
                            transformString = qsTr("None")

                        summaryString += qsTr("Initial Transforms: ") + transformString;

                        var normalFont = "<font>";
                        var warningFont = "<font color=\"red\">";

                        if(tabularDataParser.graphSizeEstimate.keys !== undefined)
                        {
                            summaryString += "<br><br>" + qsTr("Estimated Pre-Transform Graph Size: ");

                            var warningThreshold = 5e6;

                            var numNodes = tabularDataParser.graphSizeEstimate.numNodes[0];
                            var numEdges = tabularDataParser.graphSizeEstimate.numEdges[0];

                            var nodesFont = normalFont;
                            if(numNodes > warningThreshold)
                                nodesFont = warningFont;

                            var edgesFont = normalFont;
                            if(numEdges > warningThreshold)
                                edgesFont = warningFont;

                            summaryString +=
                                nodesFont + QmlUtils.formatNumberSIPostfix(numNodes) + qsTr(" Nodes") + "</font>" +
                                ", " +
                                edgesFont + QmlUtils.formatNumberSIPostfix(numEdges) + qsTr(" Edges") + "</font>";

                            if(numNodes > warningThreshold || numEdges > warningThreshold)
                            {
                                summaryString += "<br><br>" + warningFont +
                                    qsTr("WARNING: This is a very large graph which has the potential " +
                                    "to exhaust system resources and lead to instability " +
                                    "or freezes. Increasing the Minimum Correlation Value will " +
                                    "usually reduce the graph size.") + "</font>";
                            }
                        }
                        else if(!tabularDataParser.graphSizeEstimateInProgress)
                        {
                            summaryString += "<br><br>" + warningFont +
                                qsTr("WARNING: It is likely that the generated graph will be empty.") + "</font>";
                        }

                        return summaryString;
                    }

                    enabled: !tabularDataParser.graphSizeEstimateInProgress
                    BusyIndicator
                    {
                        anchors.centerIn: parent
                        running: tabularDataParser.graphSizeEstimateInProgress
                    }
                }
            }
        }

        finishEnabled: !tabularDataParser.graphSizeEstimateInProgress

        onAccepted:
        {
            root.accepted();
            root.close();
        }

        onCancel: { root.close(); }
    }

    Component.onCompleted: { initialise(); }
    function initialise()
    {
        var DEFAULT_MINIMUM_CORRELATION = 0.7;
        var DEFAULT_INITIAL_CORRELATION = DEFAULT_MINIMUM_CORRELATION +
                ((1.0 - DEFAULT_MINIMUM_CORRELATION) * 0.5);

        parameters = { minimumCorrelation: DEFAULT_MINIMUM_CORRELATION,
            initialThreshold: DEFAULT_INITIAL_CORRELATION, transpose: false,
            scaling: ScalingType.None, normalise: NormaliseType.None,
            missingDataType: MissingDataType.None };

        minimumCorrelationSpinBox.value = DEFAULT_MINIMUM_CORRELATION;
        initialCorrelationSpinBox.value = DEFAULT_INITIAL_CORRELATION;
        transposeCheckBox.checked = false;
    }

    onVisibleChanged:
    {
        if(visible)
        {
            if(root.fileUrl.length !== 0 && root.fileType.length !== 0)
                tabularDataParser.parse(root.fileUrl, root.fileType);
            else
                console.log("ERROR: fileUrl or fileType are empty");
        }
    }
}
