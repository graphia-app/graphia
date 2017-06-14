import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import "Constants.js" as Constants
import "Controls"

Wizard
{
    //FIXME These should be set automatically by Wizard
    minimumWidth: 640
    minimumHeight: 400

    Item
    {
        anchors.fill: parent
        ColumnLayout
        {
            width: parent.width
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Correlation</h2>")
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
                    anchors.top: parent.top
                    Layout.minimumWidth: 100
                    Layout.minimumHeight: 100
                    sourceSize.width: 100
                    sourceSize.height: 100
                    source: "../plots.svg"
                }
            }
        }
    }

    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 20

            Text
            {
                text: qsTr("<h2>Pearson Correlation</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            Text
            {
                text: qsTr("A Pearson Correlation will be performed on the dataset to provide a measure of correlation between rows of data. " +
                           "1.0 represents highly correlated rows and 0.0 represents no correlation. Negative correlation values are discarded. " +
                           "All values below the Minimum correlation value will also be discarded and will not create edges in the generated graph.<br>" +
                           "<br>" +
                           "By default a transform is created which will create edges for all values above the minimum correlation threshold. " +
                           "Is is not possible to create edges using values below the minimum correlation value.")
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                Layout.fillWidth: true
            }

            RowLayout
            {
                Text
                {
                    text: qsTr("Minimum Correlation:")
                    Layout.alignment: Qt.AlignRight
                }

                Item { Layout.fillWidth: true }

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
                        slider.value = value;
                    }
                }

                Slider
                {
                    id: slider
                    minimumValue: 0.0
                    maximumValue: 1.0
                    onValueChanged:
                    {
                        minimumCorrelationSpinBox.value = value;
                    }
                }
            }
        }
    }

    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Data Transpose and Scaling</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 20

                Text
                {
                    text: qsTr("Please select if the data should be transposed and the required method to " +
                               "scale the data input. This will occur before normalisation.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                CheckBox
                {
                    id: transposeCheckBox

                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("Transpose Dataset")
                    onCheckedChanged:
                    {
                        parameters.transpose = checked;
                    }
                }

                RowLayout
                {
                    Text
                    {
                        text: qsTr("Scaling:")
                        Layout.alignment: Qt.AlignLeft
                    }

                    ComboBox
                    {
                        id: scaling
                        Layout.alignment: Qt.AlignRight
                        model: [ qsTr("None"), qsTr("Log2(𝒙 + ε)"), qsTr("Log10(𝒙 + ε)"),
                            qsTr("AntiLog2(𝒙)"), qsTr("AntiLog10(𝒙)"), qsTr("Arcsin(𝒙)")]

                        onCurrentIndexChanged:
                        {
                            parameters.scaling = scalingStringToEnum(currentText);
                        }
                    }
                }

                GridLayout
                {
                    columns: 2
                    rowSpacing: 20

                    Text
                    {
                        text: "<b>Log</b>𝒃(𝒙 + ε):"
                        Layout.alignment: Qt.AlignTop
                        textFormat: Text.StyledText
                    }

                    Text
                    {
                        text: qsTr("Will perform a Log of 𝒙 + ε to base 𝒃, where 𝒙 is the input data and ε is a very small constant.");
                        wrapMode: Text.WordWrap
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true
                    }

                    Text
                    {
                        text: "<b>AntiLog</b>𝒃(𝒙):"
                        Layout.alignment: Qt.AlignTop
                        textFormat: Text.StyledText
                    }

                    Text
                    {
                        text: qsTr("Will raise x to the power of 𝒃, where 𝒙 is the input data.");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text
                    {
                        text: "<b>Arcsin</b>(𝒙):"
                        Layout.alignment: Qt.AlignTop
                        textFormat: Text.StyledText
                    }

                    Text
                    {
                        text: qsTr("Will perform an inverse sine function of 𝒙, where 𝒙 is the input data. This is useful when " +
                                   "you require a log transform but the dataset contains negative numbers or zeros.");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Data Normalisation</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }
            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 20;

                Text
                {
                    text: qsTr("Please select the required method to normalise the data input. " +
                               "This will occur after scaling.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                RowLayout
                {
                    Text
                    {
                        text: qsTr("Normalisation:")
                        Layout.alignment: Qt.AlignLeft
                    }

                    ComboBox
                    {
                        id: normalise
                        Layout.alignment: Qt.AlignRight
                        model: [ qsTr("None"), qsTr("MinMax"), qsTr("Quantile")]

                        onCurrentIndexChanged:
                        {
                            parameters.normalise = normaliseStringToEnum(currentText);
                        }
                    }
                }

                GridLayout
                {
                    columns: 2
                    Text
                    {
                        text: "<b>MinMax:</b>"
                        textFormat: Text.StyledText
                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    }

                    Text
                    {
                        text: qsTr("Normalise the data so 1.0 is the maximum value of that column and 0.0 the minimum. " +
                                   "This is useful if the columns in the dataset have differing scales or units. " +
                                   "Note: If all elements in a column have the same value this will rescale the values to 0.0.");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text
                    {
                        text: "<b>Quantile:</b>"
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

    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Handling Missing Data</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 20;

                Text
                {
                    text: qsTr("Select how you would like to handle any missing data within " +
                               "your dataset. By default missing values are zeroed.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                RowLayout
                {
                    Text
                    {
                        text: qsTr("Replacement:")
                        Layout.alignment: Qt.AlignLeft
                    }

                    ComboBox
                    {
                        id: missingDataMethod
                        Layout.alignment: Qt.AlignRight
                        model: [ qsTr("None"), qsTr("Constant") ]

                        onCurrentIndexChanged:
                        {
                            parameters.missingDataType = missingDataStringToEnum(currentText);
                        }
                    }

                    RowLayout
                    {
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
                }

                GridLayout
                {
                    columns: 2

                    Text
                    {
                        text: "<b>Constant:</b>"
                        textFormat: Text.StyledText
                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    }

                    Text
                    {
                        text: qsTr("Replace all missing values with a constant.");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    Component.onCompleted: initialise();
    function initialise()
    {
        parameters = { minimumCorrelation: 0.7, transpose: false,
            scaling: ScalingType.None, normalise: NormaliseType.None,
            missingDataType: MissingDataType.None };

        minimumCorrelationSpinBox.value = 0.7;
        transposeCheckBox.checked = false;
    }

    function scalingStringToEnum(scalingString)
    {
        switch(scalingString)
        {
        case qsTr("Log2(𝒙 + ε)"):    return ScalingType.Log2;
        case qsTr("Log10(𝒙 + ε)"):   return ScalingType.Log10;
        case qsTr("AntiLog2(𝒙)"):    return ScalingType.AntiLog2;
        case qsTr("AntiLog10(𝒙)"):   return ScalingType.AntiLog10;
        case qsTr("Arcsin(𝒙)"):      return ScalingType.ArcSin;
        }

        return ScalingType.None
    }

    function normaliseStringToEnum(normaliseString)
    {
        switch(normaliseString)
        {
        case qsTr("MinMax"):   return NormaliseType.MinMax;
        case qsTr("Quantile"): return NormaliseType.Quantile;
        }

        return NormaliseType.None;
    }

    function missingDataStringToEnum(missingDataString)
    {
        switch(missingDataString)
        {
        case qsTr("Constant"): return MissingDataType.Constant;
        }

        return MissingDataType.None;
    }
}
