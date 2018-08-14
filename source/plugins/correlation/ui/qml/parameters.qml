import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import com.kajeka 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants
import "Controls"

ListTabDialog
{
    id: root
    //FIXME These should be set automatically by Wizard
    minimumWidth: 700
    minimumHeight: 500
    property int selectedRow: -1
    property int selectedCol: -1
    property bool _clickedCell: false

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
                                          dataRectView.flickableItem.contentWidth -
                                          dataRectView.flickableItem.width);
        // Pre-2017 ECMA doesn't have Math.clamp...
        dataFrameAnimationX.to = Math.max(dataFrameAnimationX.to, 0);

        // Only animate if we need to
        if(dataRectView.flickableItem.contentX !== dataFrameAnimationX.to)
            dataFrameAnimationX.running = true;

        dataFrameAnimationY.to = Math.min(runningHeight,
                                          dataRectView.flickableItem.contentHeight -
                                          dataRectView.flickableItem.height);
        dataFrameAnimationY.to = Math.max(dataFrameAnimationY.to, 0);

        if(dataRectView.flickableItem.contentY !== dataFrameAnimationY.to)
            dataFrameAnimationY.running = true;
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
                    text: qsTr("The correlation plugin creates graphs based on how similar row profiles are in a dataset.<br>" +
                               "<br>" +
                               "If specified, the input data will be scaled and normalised and a Pearson Correlation will be performed. " +
                               "The <a href=\"https://en.wikipedia.org/wiki/Pearson_correlation_coefficient\">Pearson Correlation coefficient</a> " +
                               "is effectively a measure of similarity between rows of data. It is used to determine " +
                               "whether or not an edge is created between rows.<br>" +
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
        property bool _busy: preParser.isRunning || root.animating

        Connections
        {
            target: root
            onAnimatingChanged:
            {
                if(!root.animating && root.currentItem === dataRectPage)
                {
                    if(root.fileUrl !== "" && root.fileType !== "" && preParser.model.rowCount() === 0)
                        preParser.parse();
                }
            }
        }

        CorrelationPreParser
        {
            id: preParser
            fileType: root.fileType
            fileUrl: root.fileUrl
            onDataRectChanged:
            {
                parameters.dataFrame = dataRect;
                if(!isInsideRect(selectedCol, selectedRow, dataRect) &&
                        selectedCol >= 0 && selectedRow >= 0)
                {
                    scrollToCell(dataRectView, dataRect.x, dataRect.y)
                    tooltipNonNumerical.visible = _clickedCell;
                    _clickedCell = false;
                }
            }
            onDataLoaded:
            {
                repopulateTableView();
                Qt.callLater(scrollToCell, dataRectView, preParser.dataRect.x, preParser.dataRect.y);
            }
        }

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
                        preParser.transposed = checked;
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
                // Hack to prevent TableView crawling when it adds a large number of columns
                // Should be fixed with new tableview?
                property int maxColumns: 200
                id: dataRectView
                headerVisible: false
                Layout.fillHeight: true
                Layout.fillWidth: true
                model: preParser.model
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
                        return isInsideRect(styleData.column, styleData.row, preParser.dataRect);
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
                                if(styleData.column === dataRectView.maxColumns)
                                    return;
                                tooltipNonNumerical.visible = false;
                                nonNumericalTimer.stop();
                                selectedCol = styleData.column;
                                selectedRow = styleData.row;
                                _clickedCell = true;
                                preParser.autoDetectDataRectangle(styleData.column, styleData.row);
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
                                if(styleData.column >= dataRectView.maxColumns)
                                {
                                    if(styleData.row === 0)
                                    {
                                        return (preParser.model.columnCount() - dataRectView.maxColumns) +
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
                target: preParser.model

                onModelReset:
                {
                    repopulateTableView();
                    selectedCol = 0;
                    selectedRow = 0;
                    preParser.autoDetectDataRectangle();
                }
            }
        }
    }

    function repopulateTableView()
    {
        while(dataRectView.columnCount > 0)
            dataRectView.removeColumn(0);

        dataRectView.model = null;
        dataRectView.model = preParser.model;
        for(var i = 0; i < preParser.model.columnCount(); i++)
        {
            dataRectView.addColumn(columnComponent.createObject(dataRectView,
                                                                {"role": i}));

            // Cap the column count since a huge number of columns causes a large slowdown
            if(i == dataRectView.maxColumns - 1)
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
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Correlation - Adjust Thresholds</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            ColumnLayout
            {
                spacing: 20

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

                GridLayout
                {
                    columns: 4
                    Text
                    {
                        text: qsTr("Minimum Correlation:")
                        Layout.alignment: Qt.AlignRight
                    }

                    SpinBox
                    {
                        id: minimumCorrelationSpinBox

                        Layout.alignment: Qt.AlignLeft
                        implicitWidth: 70

                        minimumValue: 0.0
                        maximumValue: 1.0

                        decimals: 3
                        stepSize: (maximumValue - minimumValue) / 100.0

                        onValueChanged:
                        {
                            parameters.minimumCorrelation = value;
                        }
                    }

                    Slider
                    {
                        id: minimumSlider
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
                        title: qsTr("Minimum Correlation")
                        Text
                        {
                            wrapMode: Text.WordWrap
                            text: qsTr("The minimum correlation value above which an edge " +
                                       "will be created in the graph. Using a lower minimum value will " +
                                       "increase the computation time.")
                        }
                    }

                    Text
                    {
                        text: qsTr("Initial Correlation Threshold:")
                        Layout.alignment: Qt.AlignRight
                    }

                    SpinBox
                    {
                        id: initialCorrelationSpinBox

                        Layout.alignment: Qt.AlignLeft
                        implicitWidth: 70

                        maximumValue: 1.0

                        decimals: 3
                        stepSize: (maximumValue - minimumValue) / 100.0

                        onValueChanged:
                        {
                            parameters.initialThreshold = value;
                            initialSlider.value = value;
                        }
                    }

                    Slider
                    {
                        id: initialSlider
                        minimumValue: minimumCorrelationSpinBox.value
                        maximumValue: 1.0
                        onValueChanged:
                        {
                            initialCorrelationSpinBox.value = value;
                        }
                    }

                    HelpTooltip
                    {
                        title: qsTr("Correlation Threshold")
                        Text
                        {
                            wrapMode: Text.WordWrap
                            text: qsTr("Sets the initial correlation threshold, below which edges in the graph are filtered. " +
                                       "A lower value filters fewer, whereas a higher value filters more. " +
                                       "Its value can be changed later, after the graph has been created.")
                        }
                    }
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
                               "will occur in the order it is displayed below.")
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
                        id: missingDataMethod
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
                        visible: missingDataMethod.currentText === qsTr("Constant")

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
                           "the graph from here.")
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
                            text: qsTr("Markov clustering algorithm simulates stochastic flow within the generated graph to identify " +
                                       "distinct clusters. ");
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
                text:
                {
                    var summaryString = "";
                    summaryString += qsTr("Minimum Correlation: ") + minimumCorrelationSpinBox.value + "\n";
                    summaryString += qsTr("Initial Correlation Threshold: ") + initialCorrelationSpinBox.value + "\n";
                    if(preParser.dataRect != Qt.rect(0,0,0,0))
                    {
                        summaryString += qsTr("Data Frame:") +
                                qsTr(" [ Column: ") + preParser.dataRect.x +
                                qsTr(" Row: ") + preParser.dataRect.y +
                                qsTr(" Width: ") + preParser.dataRect.width +
                                qsTr(" Height: ") + preParser.dataRect.height + " ]\n";
                    }
                    else
                    {
                        summaryString += qsTr("Data Frame: Automatic\n");
                    }
                    summaryString += qsTr("Data Transpose: ") + transposeCheckBox.checked + "\n";
                    summaryString += qsTr("Data Scaling: ") + scaling.currentText + "\n";
                    summaryString += qsTr("Data Normalise: ") + normalise.currentText + "\n";
                    summaryString += qsTr("Missing Data Replacement: ") + missingDataMethod.currentText + "\n";
                    if(missingDataMethod.model.get(missingDataMethod.currentIndex).value === MissingDataType.Constant)
                        summaryString += qsTr("Replacement Constant: ") + replacementConstant.text + "\n";
                    var transformString = ""
                    if(clustering.model.get(clustering.currentIndex).value !== ClusteringType.None)
                        transformString += qsTr("Clustering (") + clustering.currentText + ")\n";
                    if(edgeReduction.model.get(edgeReduction.currentIndex).value !== EdgeReductionType.None)
                        transformString += qsTr("Edge Reduction (") + edgeReduction.currentText + ")\n";
                    if(!transformString)
                        transformString = qsTr("None")
                    summaryString += qsTr("Initial Transforms: ") + transformString;
                    return summaryString;
                }
            }
        }
    }

    Component.onCompleted: initialise();
    function initialise()
    {
        var DEFAULT_MINIMUM_CORRELATION = 0.7;
        var DEFAULT_INITIAL_CORRELATION = DEFAULT_MINIMUM_CORRELATION +
                ((1.0 - DEFAULT_MINIMUM_CORRELATION) * 0.5);

        parameters = { minimumCorrelation: DEFAULT_MINIMUM_CORRELATION, initialThreshold: DEFAULT_INITIAL_CORRELATION, transpose: false,
            scaling: ScalingType.None, normalise: NormaliseType.None,
            missingDataType: MissingDataType.None };

        minimumCorrelationSpinBox.value = DEFAULT_MINIMUM_CORRELATION;
        initialCorrelationSpinBox.value = DEFAULT_INITIAL_CORRELATION;
        transposeCheckBox.checked = false;
    }
}
