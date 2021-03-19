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

import QtQuick.Controls 1.5
import QtQuick 2.14
import QtQml 2.12
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import app.graphia 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "Controls"

BaseParameterDialog
{
    id: root
    //FIXME These should be set automatically by Wizard
    minimumWidth: 700
    minimumHeight: 500

    modality: Qt.ApplicationModal

    Preferences
    {
        section: "correlation"
        property alias advancedParameters: advancedCheckBox.checked
    }

    function isInsideRect(x, y, rect)
    {
        return  x >= rect.x &&
                x < rect.x + rect.width &&
                y >= rect.y &&
                y < rect.y + rect.height;
    }

    CorrelationTabularDataParser
    {
        id: tabularDataParser

        minimumCorrelation: minimumCorrelationSpinBox.value
        correlationType: { return algorithm.model.get(algorithm.currentIndex).value; }
        correlationDataType: { return dataType.model.get(dataType.currentIndex).value; }
        correlationPolarity: { return polarity.model.get(polarity.currentIndex).value; }
        scalingType: { return scaling.model.get(scaling.currentIndex).value; }
        normaliseType: { return normalise.model.get(normalise.currentIndex).value; }
        missingDataType: { return missingDataType.model.get(missingDataType.currentIndex).value; }
        replacementValue: replacementConstant.text

        onDataRectChanged:
        {
            parameters.dataRect = dataRect.asQRect;

            if(dataRect.hasDiscreteValues)
                dataType.setDiscrete();
            else if(dataRect.appearsToBeContinuous)
                dataType.setContinuous();

            dataRectView.scrollToDataRect();
        }

        onDataLoaded:
        {
            tabularDataParser.autoDetectDataRectangle();
            parameters.data = tabularDataParser.data;
        }
    }

    ColumnLayout
    {
        id: loadingInfo

        anchors.fill: parent
        visible: !tabularDataParser.complete || tabularDataParser.failed

        Item { Layout.fillHeight: true }

        Text
        {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            text: tabularDataParser.failed ?
                qsTr("Failed to Load ") + QmlUtils.baseFileNameForUrl(fileUrl) + "." :
                qsTr("Loading ") + QmlUtils.baseFileNameForUrl(fileUrl) + "…"
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            ProgressBar
            {
                visible: !tabularDataParser.failed
                value: tabularDataParser.progress >= 0.0 ? tabularDataParser.progress / 100.0 : 0.0
                indeterminate: tabularDataParser.progress < 0.0
            }

            Button
            {
                text: tabularDataParser.failed ? qsTr("Close") : qsTr("Cancel")
                onClicked:
                {
                    if(!tabularDataParser.failed)
                        tabularDataParser.cancelParse();

                    root.close();
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    ListTabView
    {
        id: listTabView
        anchors.fill: parent
        visible: !loadingInfo.visible

        onListTabChanged:
        {
            if(currentItem == dataRectPage)
                dataTable.forceLayout();
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
                        text: qsTr("The correlation plugin creates graphs based on the similarity between variables in a dataset.<br>" +
                                   "<br>" +
                                   "A ") + QmlUtils.redirectLink("correlation_coef", qsTr("correlation coefficient")) + qsTr(" " +
                                   "provides this similarity metric. " +
                                   "If specified, the input data can be scaled and normalised, after which correlation scores are " +
                                   "used to determine whether or not an edge is created between the nodes representing rows of data.<br>" +
                                   "<br>" +
                                   "The edges may be filtered using transforms once the graph has been created.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true

                        PointingCursorOnHoverLink {}
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

            ColumnLayout
            {
                anchors.fill: parent

                Text
                {
                    text: qsTr("<h2>Data Selection - Select and Adjust</h2>")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                }

                Text
                {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("A dataframe has been automatically selected for your dataset. " +
                        "If you would like to adjust it, select the new starting cell below. " +
                        "Note that the dataframe always extends to the bottom rightmost cell of the dataset.")
                }

                RowLayout
                {
                    CheckBox
                    {
                        id: transposeCheckBox

                        text: qsTr("Transpose")
                        enabled: !dataRectPage._busy
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

                RowLayout
                {
                    Text
                    {
                        text: qsTr("Data Type:")
                        Layout.alignment: Qt.AlignRight
                    }

                    ComboBox
                    {
                        id: dataType
                        enabled: tabularDataParser.dataHasNumericalRect && !dataRectPage._busy

                        model: ListModel
                        {
                            ListElement { text: qsTr("Continuous"); value: CorrelationDataType.Continuous }
                            ListElement { text: qsTr("Discrete");   value: CorrelationDataType.Discrete }
                        }
                        textRole: "text"

                        property bool _setting: false
                        onCurrentIndexChanged:
                        {
                            parameters.correlationDataType = model.get(currentIndex).value;

                            if(_setting)
                                return;

                            let newValue = model.get(currentIndex).value;
                            if(newValue === CorrelationDataType.Continuous && tabularDataParser.dataRect.hasDiscreteValues)
                                tabularDataParser.autoDetectDataRectangle();
                        }

                        function indexForDataType(type)
                        {
                            for(let index = 0; index < model.count; index++)
                            {
                                if(model.get(index).value === type)
                                    return index;
                            }

                            return -1;
                        }

                        function setContinuous()
                        {
                            _setting = true;
                            currentIndex = indexForDataType(CorrelationDataType.Continuous);
                            _setting = false;
                        }

                        function setDiscrete()
                        {
                            _setting = true;
                            currentIndex = indexForDataType(CorrelationDataType.Discrete);
                            _setting = false;
                        }

                        property int value: { return model.get(currentIndex).value; }
                    }

                    HelpTooltip
                    {
                        Layout.rightMargin: Constants.spacing * 2

                        title: qsTr("Data Type")
                        GridLayout
                        {
                            columns: 2
                            Text
                            {
                                text: qsTr("<b>Continuous:</b>")
                                textFormat: Text.StyledText
                                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            }

                            Text
                            {
                                text: qsTr("Interpret the selected data as " +
                                    "rows of continuous data. A numerical " +
                                    "correlation measure will be used between rows.");
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

                            Text
                            {
                                text: qsTr("<b>Discrete:</b>")
                                textFormat: Text.StyledText
                                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            }

                            Text
                            {
                                text: qsTr("Interpret the selected data as " +
                                    "rows of discrete values. The values may be numerical, " +
                                    "but in the normal case they will be textual. A discrete " +
                                    "correlation measure will be used between rows.");
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }

                Rectangle
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: 1
                    border.color: sysPalette.dark

                    ColumnLayout
                    {
                        id: dataTable

                        anchors.margins: 1
                        anchors.fill: parent

                        function forceLayout()
                        {
                            dataRectView.forceLayout();
                            columnHeaderView.forceLayout();
                        }

                        TableView
                        {
                            property var headerPadding: 4
                            property var headerWidthPadding: 10
                            id: columnHeaderView

                            model: dataRectView.model
                            height: headerFontMetrics.height + columnHeaderView.headerPadding
                            Layout.fillWidth: true
                            interactive: false
                            clip: true

                            rowHeightProvider: function(row)
                            {
                                return row > 1 ? 0 : -1;
                            }

                            columnWidthProvider: dataRectView.columnWidthProvider

                            FontMetrics
                            {
                                id: headerFontMetrics
                            }

                            delegate: Item
                            {
                                property var modelIndex: index
                                implicitWidth: headerLabel.contentWidth + columnHeaderView.headerWidthPadding
                                implicitHeight: headerFontMetrics.height + columnHeaderView.headerPadding

                                id: headerDelegate
                                Rectangle
                                {
                                    anchors.fill: parent
                                    color: sysPalette.light
                                }
                                Label
                                {
                                    id: headerLabel
                                    clip: true
                                    maximumLineCount: 1
                                    width: parent.width
                                    text:
                                    {
                                        let headerIndex = tabularDataParser.model.index(0, model.column);
                                        if(!headerIndex.valid)
                                            return "";

                                        return tabularDataParser.model.data(headerIndex);
                                    }

                                    color: sysPalette.text
                                    padding: columnHeaderView.headerPadding
                                    renderType: Text.NativeRendering
                                }

                                Rectangle
                                {
                                    anchors.right: parent.right
                                    height: parent.height
                                    width: 1
                                    color: sysPalette.midlight

                                    MouseArea
                                    {
                                        cursorShape: Qt.SizeHorCursor
                                        width: 5
                                        height: parent.height
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        drag.target: parent
                                        drag.axis: Drag.XAxis

                                        onMouseXChanged:
                                        {
                                            if(drag.active)
                                            {
                                                let currentWidth = dataRectView.userColumnWidths[model.column];
                                                if(currentWidth === undefined)
                                                    currentWidth = headerDelegate.implicitWidth;

                                                dataRectView.userColumnWidths[model.column] = Math.max(30, currentWidth + mouseX);
                                                dataTable.forceLayout();
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        TableView
                        {
                            property var delegateHeight: headerFontMetrics.height
                            id: dataRectView
                            syncDirection: Qt.Horizontal
                            syncView: columnHeaderView

                            clip: true
                            QQC2.ScrollBar.vertical: QQC2.ScrollBar {}
                            QQC2.ScrollBar.horizontal: QQC2.ScrollBar {}
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            anchors.margins: 1
                            model: tabularDataParser.model
                            enabled: !dataRectPage._busy

                            rowHeightProvider: function(row)
                            {
                                if(row === 0)
                                    return 0;

                                return -1;
                            }

                            property var userColumnWidths: []

                            columnWidthProvider: function(column)
                            {
                                let userWidth = userColumnWidths[column];
                                if(userWidth !== undefined)
                                    return userWidth;

                                let headerIndex = tabularDataParser.model.index(0, column);
                                let headerText = tabularDataParser.model.data(headerIndex);
                                let textWidth = headerFontMetrics.advanceWidth(headerText);

                                return textWidth + (2 * columnHeaderView.headerPadding);
                            }

                            SystemPalette
                            {
                                id: sysPalette
                            }

                            delegate: Item
                            {
                                // Based on Qt source for BaseTableView delegate
                                implicitHeight: Math.max(16, label.implicitHeight)
                                onImplicitHeightChanged:
                                {
                                    dataRectView.delegateHeight = implicitHeight;
                                }

                                implicitWidth: label.implicitWidth + 16
                                visible: model.row > 0

                                clip: true

                                property var modelColumn: model.column
                                property var modelRow: model.row
                                property var modelIndex: model.index

                                SystemPalette { id: systemPalette }

                                Rectangle
                                {
                                    width: parent.width

                                    anchors.centerIn: parent
                                    height: parent.height
                                    color:
                                    {
                                        if(isInsideRect(model.column, model.row, tabularDataParser.dataRect))
                                            return systemPalette.highlight;

                                        return model.row % 2 ? sysPalette.window : sysPalette.alternateBase;
                                    }

                                    Text
                                    {
                                        id: label
                                        objectName: "label"
                                        elide: Text.ElideRight
                                        width: parent.width
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.leftMargin: 10
                                        color: QmlUtils.contrastingColor(parent.color)

                                        text: modelData
                                        renderType: Text.NativeRendering
                                    }

                                    MouseArea
                                    {
                                        anchors.fill: parent
                                        onClicked:
                                        {
                                            tabularDataParser.setDataRectangle(model.column, model.row);
                                        }
                                    }
                                }
                            }

                            function firstDataCellVisible()
                            {
                                let firstDataColumnPosition = 0;
                                for(let column = 0; column < tabularDataParser.dataRect.x; column++)
                                    firstDataColumnPosition += columnWidthProvider(column);

                                let firstDataRowPosition = 0;
                                for(let row = 0; row < tabularDataParser.dataRect.y; row++)
                                    firstDataRowPosition += delegateHeight;

                                let x = firstDataColumnPosition - contentX;
                                let y = (firstDataRowPosition - contentY) - delegateHeight;

                                return x >= 0.0 && x < width && y >= 0.0 && y < height;
                            }

                            function scrollToDataRect()
                            {
                                if(dataRectView.firstDataCellVisible())
                                    return;

                                let columnPosition = -topMargin;
                                for(let column = 0; column < tabularDataParser.dataRect.x - 1; column++)
                                    columnPosition += columnWidthProvider(column);

                                let rowPosition = delegateHeight * ((tabularDataParser.dataRect.y - 2) - 1);
                                if(rowPosition < 0)
                                    rowPosition = -topMargin;

                                scrollXAnimation.to = columnPosition;
                                scrollXAnimation.running = true;
                                scrollYAnimation.to = rowPosition;
                                scrollYAnimation.running = true;
                            }

                            PropertyAnimation
                            {
                                id: scrollXAnimation
                                target: dataRectView
                                property: "contentX"
                                duration: 750
                                easing.type: Easing.OutQuad
                            }

                            PropertyAnimation
                            {
                                id: scrollYAnimation
                                target: dataRectView
                                property: "contentY"
                                duration: 750
                                easing.type: Easing.OutQuad
                            }
                        }
                    }

                    BusyIndicator
                    {
                        id: busyIndicator
                        anchors.centerIn: parent
                        running: dataRectPage._busy
                    }
                }

                Text
                {
                    id: warningText
                    visible: text.length > 0

                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap

                    text:
                    {
                        if(dataType.value === CorrelationDataType.Discrete && tabularDataParser.dataRect.appearsToBeContinuous)
                        {
                            return qsTr("<font color=\"red\">WARNING: the selected dataframe appears " +
                                "to contain contiguous data; interpreting it as discrete may " +
                                "result in only a low level of correlation.</font>");
                        }

                        return "";
                    }
                }
            }
        }

        ListTab
        {
            title: qsTr("Correlation")
            ColumnLayout
            {
                anchors.fill: parent

                RowLayout
                {
                    Layout.fillWidth: true
                    Text
                    {
                        text: qsTr("<h2>Correlation - Adjust Parameters</h2>")
                        textFormat: Text.StyledText
                    }

                    Item { Layout.fillWidth: true }

                    CheckBox
                    {
                        id: advancedCheckBox

                        Layout.alignment: Qt.AlignTop
                        text: qsTr("Advanced")
                    }
                }

                ColumnLayout
                {
                    Layout.fillHeight: true

                    spacing: Constants.spacing * 2

                    Text
                    {
                        Layout.fillWidth: true
                        visible: !advancedCheckBox.checked

                        text: qsTr("Correlation values of 1 represent perfectly correlated rows whereas " +
                            "0 indicates no correlation. " +
                            "All absolute values below the selected minimum correlation threshold are " +
                            "discarded and will not be used to create edges. " +
                            "By default a transform is added which will remove edges for all " +
                            "absolute values below the initial correlation threshold." +
                            "<br><br>" +
                            "For more control over the parameters used, check the <b>Advanced</b> option " +
                            "on the top right.")
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                    }

                    ColumnLayout
                    {
                        Layout.fillWidth: true
                        visible: advancedCheckBox.checked

                        GridLayout
                        {
                            columns: 6
                            Layout.fillWidth: true

                            Text
                            {
                                text: qsTr("Algorithm:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: algorithm

                                model: ListModel
                                {
                                    ListElement { text: qsTr("Pearson");        value: CorrelationType.Pearson }
                                    ListElement { text: qsTr("Spearman Rank");  value: CorrelationType.SpearmanRank }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.correlationType = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                Layout.rightMargin: Constants.spacing * 2

                                title: qsTr("Correlation Algorithm")
                                GridLayout
                                {
                                    columns: 2
                                    Text
                                    {
                                        text: qsTr("<b>Pearson:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }

                                    Text
                                    {
                                        text: qsTr("The Pearson correlation coefficient is a measure " +
                                            "of the linear correlation between two variables.");
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    Text
                                    {
                                        text: qsTr("<b>Spearman Rank:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }

                                    Text
                                    {
                                        text: qsTr("Spearman's rank correlation coefficient is a " +
                                            "nonparametric measure of the statistical dependence between " +
                                            "the rankings of two variables. It assesses how well the " +
                                            "relationship between two variables can be described using a " +
                                            "monotonic function.");
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }

                            Text
                            {
                                text: qsTr("Polarity:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: polarity

                                model: ListModel
                                {
                                    ListElement { text: qsTr("Positive");   value: CorrelationPolarity.Positive }
                                    ListElement { text: qsTr("Negative");   value: CorrelationPolarity.Negative }
                                    ListElement { text: qsTr("Both");       value: CorrelationPolarity.Both }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.correlationPolarity = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                title: qsTr("Polarity")
                                Text
                                {
                                    wrapMode: Text.WordWrap
                                    text: qsTr("By default only positive correlations are considered when creating " +
                                               "the graph. In most cases this is the correct setting, " +
                                               "but for some data sources it may make more sense to take " +
                                               "account of the polarity of the correlation.")
                                }
                            }

                            Text
                            {
                                visible: tabularDataParser.dataRect.hasMissingValues
                                text: qsTr("Imputation:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: missingDataType
                                visible: tabularDataParser.dataRect.hasMissingValues

                                model: ListModel
                                {
                                    ListElement { text: qsTr("Constant");           value: MissingDataType.Constant }
                                    ListElement { text: qsTr("Row Interpolate");    value: MissingDataType.RowInterpolation }
                                    ListElement { text: qsTr("Column Mean");        value: MissingDataType.ColumnAverage }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.missingDataType = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            RowLayout
                            {
                                Layout.columnSpan: 4
                                visible: tabularDataParser.dataRect.hasMissingValues

                                TextField
                                {
                                    id: replacementConstant
                                    visible: missingDataType.currentText === qsTr("Constant")

                                    validator: DoubleValidator{}

                                    onTextChanged: { parameters.missingDataValue = text; }

                                    text: "0.0"
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
                            }

                            Text
                            {
                                text: qsTr("Scaling:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: scaling

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

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                title: qsTr("Scaling Types")
                                GridLayout
                                {
                                    columns: 2
                                    rowSpacing: Constants.spacing

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
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: normalise

                                model: ListModel
                                {
                                    ListElement { text: qsTr("None");               value: NormaliseType.None }
                                    ListElement { text: qsTr("Min/Max");            value: NormaliseType.MinMax }
                                    ListElement { text: qsTr("Mean");               value: NormaliseType.Mean }
                                    ListElement { text: qsTr("Standardisation");    value: NormaliseType.Standarisation }
                                    ListElement { text: qsTr("Unit Scaling");       value: NormaliseType.UnitScaling }
                                    ListElement { text: qsTr("Quantile");           value: NormaliseType.Quantile }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.normalise = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                title: qsTr("Normalisation Types")
                                GridLayout
                                {
                                    columns: 2
                                    Text
                                    {
                                        text: qsTr("<b>Min/Max:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }
                                    Text
                                    {
                                        text: qsTr("Rescale the data so that its values " +
                                                   "lie within the [0, 1] range: " +
                                                   "(<i>x</i>-min(<i>x</i>))/(max(<i>x</i>)-min(<i>x</i>)). " +
                                                   "This is useful if the columns in the dataset " +
                                                   "have differing scales or units. " +
                                                   "Note: If all elements in a column have the same value " +
                                                   "this will rescale the values to 0.0.");
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    Text
                                    {
                                        text: qsTr("<b>Mean:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }
                                    Text
                                    {
                                        text: qsTr("Similar to Min/Max scaling except the resultant " +
                                                   "values are centred around their column mean: " +
                                                   "(<i>x</i>-µ)/(max(<i>x</i>)-min(<i>x</i>)). " +
                                                   "Note: If all elements in a column have the same value " +
                                                   "this will rescale the values to 0.0.");
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    Text
                                    {
                                        text: qsTr("<b>Standardisation:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }
                                    Text
                                    {
                                        text: qsTr("Also known as Z-score normalisation, this method " +
                                                   "centres the value around the mean and scales by " +
                                                   "the standard deviation: (<i>x</i>-µ)/σ.");
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    Text
                                    {
                                        text: qsTr("<b>Unit Scaling:</b>")
                                        textFormat: Text.StyledText
                                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                    }
                                    Text
                                    {
                                        text: qsTr("Values are divided by the euclidean length of " +
                                                   "the vector formed by the column of data: <i>x</i>/‖<i>x</i>‖.");
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
                                let range = maximumValue - value;
                                let adjustedInitial = value + (range * 0.1);

                                if(initialCorrelationSpinBox.value <= adjustedInitial)
                                    initialCorrelationSpinBox.value = adjustedInitial;
                            }
                        }

                        Slider
                        {
                            id: minimumSlider

                            Layout.fillWidth: true
                            Layout.minimumWidth: 50
                            Layout.maximumWidth: 150

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
                            Layout.rightMargin: Constants.spacing * 2

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

                        visible: tabularDataParser.graphSizeEstimate.keys !== undefined || _timedBusy
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

                            function onGraphSizeEstimateInProgressChanged()
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
            title: qsTr("Initial Transforms")
            ColumnLayout
            {
                spacing: Constants.spacing * 2

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
                        model: ListModel
                        {
                            ListElement { text: qsTr("None");       value: ClusteringType.None }
                            ListElement { text: qsTr("Louvain");    value: ClusteringType.Louvain }
                            ListElement { text: qsTr("MCL");        value: ClusteringType.MCL }
                        }
                        textRole: "text"

                        onCurrentIndexChanged:
                        {
                            parameters.clusteringType = model.get(currentIndex).value;
                        }

                        property int value: { return model.get(currentIndex).value; }
                    }

                    HelpTooltip
                    {
                        title: qsTr("Clustering")
                        GridLayout
                        {
                            columns: 2

                            Text
                            {
                                text: qsTr("<b>Louvain:</b>")
                                textFormat: Text.StyledText
                                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            }

                            Text
                            {
                                text: qsTr("Louvain modularity is a method for finding clusters " +
                                           "by measuring edge density from within communities to " +
                                           "neighbouring communities. It is often a good choice " +
                                           "when used in conjunction with an edge reduction " +
                                           "transform.");
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }

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
                        model: ListModel
                        {
                            ListElement { text: qsTr("None");   value: EdgeReductionType.None }
                            ListElement { text: qsTr("k-NN");   value: EdgeReductionType.KNN }
                            ListElement { text: qsTr("%-NN");   value: EdgeReductionType.PercentNN }
                        }
                        textRole: "text"

                        onCurrentIndexChanged:
                        {
                            parameters.edgeReductionType = model.get(currentIndex).value;
                        }

                        property int value: { return model.get(currentIndex).value; }
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

                                Text
                                {
                                    text: qsTr("<b>%-NN:</b>")
                                    textFormat: Text.StyledText
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                }

                                Text
                                {
                                    text: qsTr("Like k-nearest neighbours, but instead of choosing " +
                                        "the top <i>k</i> edges, choose a percentage of the highest " +
                                        "ranking edges.");
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
                    text: qsTr("A graph will be created using the following parameters:")
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
                        let indentString = "&nbsp;&nbsp;&nbsp;";
                        let summaryString = "";

                        if(transposeCheckBox.checked)
                            summaryString += qsTr("Transposed<br>");

                        summaryString += qsTr("Data Frame:") +
                            qsTr(" [ Column: ") + tabularDataParser.dataRect.x +
                            qsTr(" Row: ") + tabularDataParser.dataRect.y +
                            qsTr(" Width: ") + tabularDataParser.dataRect.width +
                            qsTr(" Height: ") + tabularDataParser.dataRect.height + " ]<br>";

                        summaryString += qsTr("Correlation Metric: ") + algorithm.currentText + "<br>";
                        summaryString += qsTr("Correlation Polarity: ") + polarity.currentText + "<br>";
                        summaryString += qsTr("Minimum Correlation Value: ") + minimumCorrelationSpinBox.value + "<br>";
                        summaryString += qsTr("Initial Correlation Threshold: ") + initialCorrelationSpinBox.value + "<br>";

                        if(scaling.value !== ScalingType.None)
                            summaryString += qsTr("Scaling: ") + scaling.currentText + "<br>";

                        if(normalise.value !== NormaliseType.None)
                            summaryString += qsTr("Normalisation: ") + normalise.currentText + "<br>";

                        if(tabularDataParser.dataRect.hasMissingValues)
                        {
                            summaryString += qsTr("Imputation: ") + missingDataType.currentText;

                            if(missingDataType.value === MissingDataType.Constant)
                                summaryString += qsTr(" (") + replacementConstant.text + qsTr(")");

                            summaryString += "<br>";
                        }

                        let transformString = ""
                        if(clustering.value !== ClusteringType.None)
                        {
                            transformString += indentString + qsTr("• Clustering (") +
                                clustering.currentText + ")<br>";
                        }

                        if(edgeReduction.value !== EdgeReductionType.None)
                        {
                            transformString += indentString + qsTr("• Edge Reduction (") +
                                edgeReduction.currentText + ")<br>";
                        }

                        if(transformString.length > 0)
                            summaryString += qsTr("Initial Transforms:<br>") + transformString;

                        let normalFont = "<font>";
                        let warningFont = "<font color=\"red\">";

                        if(tabularDataParser.graphSizeEstimate.keys !== undefined)
                        {
                            summaryString += "<br>" + qsTr("Estimated Pre-Transform Graph Size: ");

                            let warningThreshold = 5e6;

                            let numNodes = tabularDataParser.graphSizeEstimate.numNodes[0];
                            let numEdges = tabularDataParser.graphSizeEstimate.numEdges[0];

                            let nodesFont = normalFont;
                            if(numNodes > warningThreshold)
                                nodesFont = warningFont;

                            let edgesFont = normalFont;
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

        onRejected: { root.close(); }
    }

    Component.onCompleted: { initialise(); }
    function initialise()
    {
        let DEFAULT_MINIMUM_CORRELATION = 0.7;
        let DEFAULT_INITIAL_CORRELATION = DEFAULT_MINIMUM_CORRELATION +
                ((1.0 - DEFAULT_MINIMUM_CORRELATION) * 0.5);

        parameters = { minimumCorrelation: DEFAULT_MINIMUM_CORRELATION,
            initialThreshold: DEFAULT_INITIAL_CORRELATION, transpose: false,
            correlationType: CorrelationType.Pearson,
            correlationDataType: CorrelationDataType.Continuous,
            correlationPolarity: CorrelationPolarity.Positive,
            scaling: ScalingType.None, normalise: NormaliseType.None,
            missingDataType: MissingDataType.Constant };

        minimumCorrelationSpinBox.value = DEFAULT_MINIMUM_CORRELATION;
        initialCorrelationSpinBox.value = DEFAULT_INITIAL_CORRELATION;
        transposeCheckBox.checked = false;
        dataType.currentIndex = 0;
    }

    onVisibleChanged:
    {
        if(visible)
        {
            if(root.fileUrl.length !== 0 && root.fileType.length !== 0)
                tabularDataParser.parse(root.fileUrl, root.fileType);
            else
                console.log("ERROR: fileUrl or fileType is empty");
        }
    }
}
