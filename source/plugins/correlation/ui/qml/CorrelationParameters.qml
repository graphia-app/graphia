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

import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

BaseParameterDialog
{
    id: root
    //FIXME These should be set automatically by Wizard
    minimumWidth: 750
    minimumHeight: 500

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

        threshold: minimumCorrelationSpinBox.value
        correlationDataType: dataTypeComboBox.value
        continuousCorrelationType: continuousAlgorithmComboBox.value
        correlationPolarity: polarityComboBox.value
        discreteCorrelationType: discreteAlgorithmComboBox.value
        scalingType: scalingComboBox.value
        normaliseType: normalisationComboBox.value
        missingDataType: missingDataTypeComboBox.value
        replacementValue: replacementConstantText.text.length > 0 ? replacementConstantText.text : 0.0
        clippingType: clippingTypeComboBox.value
        clippingValue:
        {
            switch(clippingTypeComboBox.value)
            {
            case ClippingType.Constant:
                return clippingValueText.text.length > 0 ? parseFloat(clippingValueText.text) : 0.0;
            case ClippingType.Winsorization:
                return winsoriztionValueSpinBox.value;
            }

            return 0.0;
        }

        treatAsBinary: treatAsBinaryCheckbox.checked

        onDataRectChanged:
        {
            parameters.dataRect = dataRect.asQRect;

            if(dataRect.hasDiscreteValues)
                dataTypeComboBox.setDiscrete();
            else if(dataRect.appearsToBeContinuous)
                dataTypeComboBox.setContinuous();

            clippingValueText.text = QmlUtils.formatNumber(dataRect.maxValue);
        }

        onDataLoaded:
        {
            tabularDataParser.autoDetectDataRectangle();
            parameters.data = tabularDataParser.data;
        }

        property double minimumInitialCorrelationValue:
        {
            if(tabularDataParser.graphSizeEstimate.keys !== undefined)
                return tabularDataParser.graphSizeEstimate.keys[0];

            return minimumCorrelationSpinBox.value;
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

            text: Utils.format(tabularDataParser.failed ?
                qsTr("Failed to Load {0}.") : qsTr("Loading {0}…"),
                QmlUtils.baseFileNameForUrl(url))
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
                onClicked: function(mouse)
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
                        text: Utils.format(qsTr("The correlation plugin creates graphs based on the similarity between variables in a dataset.<br>" +
                            "<br>" +
                            "A {0} provides this similarity metric. " +
                            "If specified, the input data can be scaled and normalised, after which correlation scores are " +
                            "used to determine whether or not an edge is created between the nodes representing rows of data.<br>" +
                            "<br>" +
                            "The edges may be filtered using transforms once the graph has been created."),
                            QmlUtils.redirectLink("correlation_coef", qsTr("correlation coefficient")))

                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true

                        PointingCursorOnHoverLink {}
                        onLinkActivated: function(link) { Qt.openUrlExternally(link); }
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
                        Layout.rightMargin: Constants.spacing * 2

                        title: qsTr("Transpose")
                        Text
                        {
                            wrapMode: Text.WordWrap
                            text: qsTr("Transpose will flip the data over its diagonal. " +
                                       "Moving rows to columns and vice versa.")
                        }
                    }

                    Text
                    {
                        text: qsTr("Data Type:")
                        Layout.alignment: Qt.AlignRight
                    }

                    ComboBox
                    {
                        id: dataTypeComboBox
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

                    CheckBox
                    {
                        id: treatAsBinaryCheckbox

                        visible: dataTypeComboBox.value === CorrelationDataType.Discrete
                        text: qsTr("Treat As Binary")

                        onCheckedChanged:
                        {
                            parameters.treatAsBinary = checked;
                        }
                    }

                    HelpTooltip
                    {
                        visible: dataTypeComboBox.value === CorrelationDataType.Discrete
                        title: qsTr("Treat As Binary")
                        Text
                        {
                            wrapMode: Text.WordWrap
                            text: qsTr("If this is enabled, for the purposes of the " +
                                "correlation algorithm empty cells or cells containing " +
                                "the values <i>0</i> or <i>false</i> are considered to " +
                                "be false, while all other values are considered to be true.")
                        }
                    }
                }

                DataTable
                {
                    id: dataTable
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: tabularDataParser.model

                    highlightedProvider: (column, row) => isInsideRect(column, row, tabularDataParser.dataRect)
                    onClicked: function(column, row, mouse) { tabularDataParser.setDataRectangle(column, row); }

                    BusyIndicator
                    {
                        anchors.centerIn: parent
                        visible: dataRectPage._busy
                    }

                    NamedIcon
                    {
                        visible: tabularDataParser.dataRect.x < dataTable.leftColumn
                        anchors { verticalCenter: parent.verticalCenter; left: parent.left; margins: Constants.margin }
                        iconName: "go-previous"
                    }

                    NamedIcon
                    {
                        visible: tabularDataParser.dataRect.x > dataTable.rightColumn
                        anchors { verticalCenter: parent.verticalCenter; right: parent.right; margins: Constants.margin }
                        iconName: "go-next"
                    }

                    NamedIcon
                    {
                        visible: tabularDataParser.dataRect.y < dataTable.topRow
                        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; margins: Constants.margin + parent.headerHeight }
                        iconName: "go-up"
                    }

                    NamedIcon
                    {
                        visible: tabularDataParser.dataRect.y > dataTable.bottomRow
                        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; margins: Constants.margin }
                        iconName: "go-down"
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
                        if(dataTypeComboBox.value === CorrelationDataType.Discrete && tabularDataParser.dataRect.appearsToBeContinuous)
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
                enabled: !dataRectPage._busy

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
                                visible: dataTypeComboBox.value === CorrelationDataType.Discrete
                                text: qsTr("Algorithm:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: discreteAlgorithmComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Discrete
                                Layout.preferredWidth: 160

                                model: ListModel
                                {
                                    ListElement { text: qsTr("Jaccard"); value: CorrelationType.Jaccard }
                                    ListElement { text: qsTr("SMC"); value: CorrelationType.SMC }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.discreteCorrelationType = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Discrete
                                Layout.rightMargin: Constants.spacing * 2

                                title: qsTr("Correlation Algorithm")
                                GridLayout
                                {
                                    id: discreteAlgorithmTooltip
                                    columns: 2
                                }
                            }

                            Text
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                text: qsTr("Algorithm:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: continuousAlgorithmComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.preferredWidth: 160

                                model: ListModel
                                {
                                    ListElement { text: qsTr("Pearson");                value: CorrelationType.Pearson }
                                    ListElement { text: qsTr("Spearman Rank");          value: CorrelationType.SpearmanRank }
                                    ListElement { text: qsTr("Euclidean Similarity");   value: CorrelationType.EuclideanSimilarity }
                                    ListElement { text: qsTr("Cosine Similarity");      value: CorrelationType.CosineSimilarity }
                                    ListElement { text: qsTr("Bicor");                  value: CorrelationType.Bicor }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.continuousCorrelationType = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            HelpTooltip
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.rightMargin: Constants.spacing * 2

                                title: qsTr("Correlation Algorithm")
                                GridLayout
                                {
                                    id: continuousAlgorithmTooltip
                                    columns: 2
                                }
                            }

                            Text
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                text: qsTr("Polarity:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: polarityComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.preferredWidth: 160

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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                title: qsTr("Polarity")
                                Text
                                {
                                    wrapMode: Text.WordWrap
                                    text: qsTr("By default only positive correlations are considered when creating " +
                                        "the graph. In almost all cases this is the correct setting, " +
                                        "but for some data sources it may make more sense to take " +
                                        "account of the polarity of the correlation. Note that not all " +
                                        "correlation algorithms are capable of finding negative relationships.")
                                }
                            }

                            Text
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous &&
                                    tabularDataParser.dataRect.hasMissingValues
                                text: qsTr("Imputation:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: missingDataTypeComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous &&
                                    tabularDataParser.dataRect.hasMissingValues
                                Layout.preferredWidth: 160

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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous &&
                                    tabularDataParser.dataRect.hasMissingValues

                                TextField
                                {
                                    id: replacementConstantText
                                    visible: missingDataTypeComboBox.value === MissingDataType.Constant
                                    selectByMouse: true

                                    validator: DoubleValidator{}

                                    onTextChanged: { if(text.length > 0) parameters.missingDataValue = text; }

                                    text: "0.0"
                                }

                                HelpTooltip
                                {
                                    visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                    title: qsTr("Imputation")
                                    GridLayout
                                    {
                                        columns: 2
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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                text: qsTr("Clipping:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: clippingTypeComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.preferredWidth: 160

                                model: ListModel
                                {
                                    ListElement { text: qsTr("None");               value: ClippingType.None }
                                    ListElement { text: qsTr("Constant");           value: ClippingType.Constant }
                                    ListElement { text: qsTr("Winsorization");      value: ClippingType.Winsorization }
                                }
                                textRole: "text"

                                onCurrentIndexChanged:
                                {
                                    parameters.clippingType = model.get(currentIndex).value;
                                }

                                property int value: { return model.get(currentIndex).value; }
                            }

                            RowLayout
                            {
                                Layout.columnSpan: 4
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous

                                TextField
                                {
                                    id: clippingValueText
                                    visible: clippingTypeComboBox.value === ClippingType.Constant
                                    selectByMouse: true

                                    validator: DoubleValidator{}

                                    onVisibleChanged: { update(); }
                                    onTextChanged: { update(); }

                                    function update()
                                    {
                                        if(visible && text.length > 0)
                                            parameters.clippingValue = text;
                                    }

                                    text: "0.0"
                                }

                                SpinBox
                                {
                                    id: winsoriztionValueSpinBox
                                    Layout.preferredWidth: 80
                                    visible: clippingTypeComboBox.value === ClippingType.Winsorization

                                    editable: true
                                    Component.onCompleted: { contentItem.selectByMouse = true; }

                                    from: 0
                                    to: 100
                                    value: 100

                                    onVisibleChanged: { update(); }
                                    onValueChanged: { update(); }

                                    function update()
                                    {
                                        if(visible)
                                            parameters.clippingValue = value;
                                    }
                                }

                                Text
                                {
                                    visible: clippingTypeComboBox.value === ClippingType.Winsorization
                                    text: qsTr("percentile")
                                }

                                HelpTooltip
                                {
                                    visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                    title: qsTr("Clipping")
                                    GridLayout
                                    {
                                        columns: 2
                                        Text
                                        {
                                            text: qsTr("<b>Constant:</b>")
                                            textFormat: Text.StyledText
                                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                        }

                                        Text
                                        {
                                            text: qsTr("Limit the maximum data table value to a specified constant.");
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }

                                        Text
                                        {
                                            text: qsTr("<b>Winsorization:</b>")
                                            textFormat: Text.StyledText
                                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                                        }

                                        Text
                                        {
                                            text: qsTr("Limit the maximum data value to that at a specified percentile, " +
                                                "on a per-row basis. For example if the 95th percentile is chosen, the " +
                                                "top 5th percentile of values will be set to that of the 95th percentile.");
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }

                            Text
                            {
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                text: qsTr("Scaling:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: scalingComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.preferredWidth: 160

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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                text: qsTr("Normalisation:")
                                Layout.alignment: Qt.AlignRight
                            }

                            ComboBox
                            {
                                id: normalisationComboBox
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
                                Layout.preferredWidth: 160

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
                                visible: dataTypeComboBox.value === CorrelationDataType.Continuous
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

                        DoubleSpinBox
                        {
                            id: minimumCorrelationSpinBox

                            implicitWidth: 70

                            from: 0.0
                            to: 1.0

                            decimals: 3
                            stepSize: Utils.incrementForRange(from, to);
                            editable: true

                            onValueChanged:
                            {
                                parameters.threshold = value;

                                // When the minimum value is increased beyond the initial
                                // value, the latter can get (visually) lost against the extreme
                                // left of the plot, so just punt it over a bit
                                let range = to - value;
                                let adjustedInitial = value + (range * 0.1);

                                if(initialCorrelationSpinBox.value <= adjustedInitial)
                                    initialCorrelationSpinBox.value = adjustedInitial;
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

                        RangeSlider
                        {
                            id: minimumSlider

                            Layout.fillWidth: true
                            Layout.minimumWidth: 50
                            Layout.maximumWidth: 250

                            from: 0.0
                            to: 1.0

                            first.value: minimumCorrelationSpinBox.value
                            first.onValueChanged:
                            {
                                minimumCorrelationSpinBox.value = first.value;
                            }

                            second.value: initialCorrelationSpinBox.value
                            second.onValueChanged:
                            {
                                initialCorrelationSpinBox.value = second.value;
                            }
                        }

                        Text { text: qsTr("Initial:") }

                        DoubleSpinBox
                        {
                            id: initialCorrelationSpinBox

                            implicitWidth: 70

                            from: tabularDataParser.minimumInitialCorrelationValue
                            to: 1.0

                            decimals: 3
                            stepSize: Utils.incrementForRange(from, to);
                            editable: true

                            onFromChanged:
                            {
                                // Reset the value if the lower bound changes
                                if(value < from)
                                    value = Utils.lerp(from, to, 0.5);
                            }

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

                        DelayedBusyIndicator
                        {
                            visible: parent.visible
                            anchors.centerIn: parent
                            width: { return Math.min(64, parent.width); }
                            height: { return Math.min(64, parent.height); }

                            delayedRunning: tabularDataParser.graphSizeEstimateInProgress
                        }
                    }

                    Text
                    {
                        visible: !graphSizeEstimatePlot.visible

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: tabularDataParser.busy || tabularDataParser.graphSizeEstimateInProgress ?
                            qsTr("Working…") : qsTr("Empty Graph")
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
                enabled: !dataRectPage._busy

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
                        id: clusteringComboBox
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
                        id: edgeReductionComboBox
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

                ScrollableTextArea
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

                        summaryString += Utils.format(qsTr("Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]<br>"),
                            tabularDataParser.dataRect.x, tabularDataParser.dataRect.y,
                            tabularDataParser.dataRect.width, tabularDataParser.dataRect.height);

                        if(dataTypeComboBox.value === CorrelationDataType.Continuous)
                        {
                            summaryString += Utils.format(qsTr("Continuous Correlation Metric: {0}<br>"), continuousAlgorithmComboBox.currentText);
                            summaryString += Utils.format(qsTr("Correlation Polarity: {0}<br>"), polarityComboBox.currentText);
                            summaryString += Utils.format(qsTr("Minimum Correlation Value: {0}<br>"), QmlUtils.formatNumberScientific(minimumCorrelationSpinBox.value));
                            summaryString += Utils.format(qsTr("Initial Correlation Threshold: {0}<br>"), QmlUtils.formatNumberScientific(initialCorrelationSpinBox.value));

                            if(tabularDataParser.dataRect.hasMissingValues)
                            {
                                summaryString += Utils.format(qsTr("Imputation: {0}"), missingDataTypeComboBox.currentText);

                                if(missingDataTypeComboBox.value === MissingDataType.Constant)
                                    summaryString += Utils.format(qsTr(" ({0})"), replacementConstantText.text);

                                summaryString += "<br>";
                            }

                            if(clippingTypeComboBox.value !== ClippingType.None)
                            {
                                summaryString += Utils.format(qsTr("Clipping: {0}"), clippingTypeComboBox.currentText);

                                if(clippingTypeComboBox.value === ClippingType.Constant)
                                    summaryString += Utils.format(qsTr(" ({0})"), clippingValueText.text);
                                else if(clippingTypeComboBox.value === ClippingType.Winsorization)
                                    summaryString += Utils.format(qsTr(" ({0} percentile)"), winsoriztionValueSpinBox.value);

                                summaryString += "<br>";
                            }

                            if(scalingComboBox.value !== ScalingType.None)
                                summaryString += Utils.format(qsTr("Scaling: {0}<br>"), scalingComboBox.currentText);

                            if(normalisationComboBox.value !== NormaliseType.None)
                                summaryString += Utils.format(qsTr("Normalisation: {0}<br>"), normalisationComboBox.currentText);
                        }
                        else if(dataTypeComboBox.value === CorrelationDataType.Discrete)
                        {
                            summaryString += Utils.format(qsTr("Discrete Correlation Metric: {0}<br>"), discreteAlgorithmComboBox.currentText);
                            summaryString += Utils.format(qsTr("Minimum Correlation Value: {0}<br>"), QmlUtils.formatNumberScientific(minimumCorrelationSpinBox.value));
                            summaryString += Utils.format(qsTr("Initial Correlation Threshold: {0}<br>"), QmlUtils.formatNumberScientific(initialCorrelationSpinBox.value));
                        }

                        let transformString = ""
                        if(clusteringComboBox.value !== ClusteringType.None)
                        {
                            transformString += indentString +
                                Utils.format(qsTr("• Clustering ({0})<br>"), clusteringComboBox.currentText);
                        }

                        if(edgeReductionComboBox.value !== EdgeReductionType.None)
                        {
                            transformString += indentString +
                                Utils.format(qsTr("• Edge Reduction ({0})<br>"), edgeReductionComboBox.currentText);
                        }

                        if(transformString.length > 0)
                            summaryString += Utils.format(qsTr("Initial Transforms:<br>{0}"), transformString);

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

                            summaryString += Utils.format(qsTr("{0}{3} Nodes{2}, {1}{4} Edges{2}"), nodesFont, edgesFont, "</font>",
                                QmlUtils.formatNumberSIPostfix(numNodes), QmlUtils.formatNumberSIPostfix(numEdges));

                            if(numNodes > warningThreshold || numEdges > warningThreshold)
                            {
                                summaryString += Utils.format(qsTr("<br><br>{0}" +
                                    "WARNING: This is a very large graph which has the potential " +
                                    "to exhaust system resources and lead to instability " +
                                    "or freezes. Increasing the Minimum Correlation Value will " +
                                    "usually reduce the graph size.{1}"), warningFont, "</font>");
                            }
                        }
                        else if(!tabularDataParser.graphSizeEstimateInProgress && !dataRectPage._busy)
                        {
                            summaryString += Utils.format(qsTr("<br>{0}" +
                                "WARNING: It is likely that the generated graph will be empty.{1}"),
                                warningFont, "</font>");
                        }
                        else
                            summaryString += "<br>Estimate in progress…";

                        return summaryString;
                    }

                    enabled: !tabularDataParser.graphSizeEstimateInProgress && !dataRectPage._busy
                    BusyIndicator
                    {
                        visible: parent.visible
                        anchors.centerIn: parent
                        running: tabularDataParser.graphSizeEstimateInProgress || dataRectPage._busy
                    }
                }
            }
        }

        finishEnabled: !tabularDataParser.graphSizeEstimateInProgress && !dataRectPage._busy

        onAccepted:
        {
            root.accepted();
            root.close();
        }

        onRejected: { root.close(); }
    }

    Component
    {
        id: correlationNameComponent

        Text
        {
            textFormat: Text.StyledText
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        }
    }

    Component
    {
        id: correlationDescriptionComponent

        Text
        {
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    function populateCorrelationAlgorithmTooltip(model, layout)
    {
        for(let i = 0; i < model.count; i++)
        {
            let item = model.get(i);
            let correlation = root.plugin.correlationInfoFor(item.value);

            correlationNameComponent.createObject(layout, {text: qsTr("<b>" + correlation.name + ":</b>")});
            correlationDescriptionComponent.createObject(layout, {text: qsTr(correlation.description)});
        }
    }

    onInitialised:
    {
        let DEFAULT_MINIMUM_CORRELATION = 0.7;
        let DEFAULT_INITIAL_CORRELATION = DEFAULT_MINIMUM_CORRELATION +
                ((1.0 - DEFAULT_MINIMUM_CORRELATION) * 0.5);

        parameters =
        {
            threshold: DEFAULT_MINIMUM_CORRELATION,
            initialThreshold: DEFAULT_INITIAL_CORRELATION,
            transpose: false,
            correlationDataType: CorrelationDataType.Continuous,
            continuousCorrelationType: CorrelationType.Pearson,
            correlationPolarity: CorrelationPolarity.Positive,
            discreteCorrelationType: CorrelationType.Jaccard,
            scaling: ScalingType.None, normalise: NormaliseType.None,
            missingDataType: MissingDataType.Constant,
            clippingType: ClippingType.None
        };

        minimumCorrelationSpinBox.value = DEFAULT_MINIMUM_CORRELATION;
        initialCorrelationSpinBox.value = DEFAULT_INITIAL_CORRELATION;
        transposeCheckBox.checked = false;
        dataTypeComboBox.currentIndex = 0;

        populateCorrelationAlgorithmTooltip(discreteAlgorithmComboBox.model, discreteAlgorithmTooltip);
        populateCorrelationAlgorithmTooltip(continuousAlgorithmComboBox.model, continuousAlgorithmTooltip);
    }

    onVisibleChanged:
    {
        if(visible)
        {
            if(QmlUtils.urlIsValid(root.url) && root.type.length !== 0)
                tabularDataParser.parse(root.url, root.type);
            else
                console.log("ERROR: url or type is empty");
        }
    }
}
