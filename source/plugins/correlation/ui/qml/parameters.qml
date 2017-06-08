import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import "Constants.js" as Constants
import "Controls"

Wizard
{
    minimumWidth: 470
    minimumHeight: 350
    Item
    {
        anchors.fill: parent;
        ColumnLayout
        {
            width: parent.width
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Correlation Plugin</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            RowLayout
            {
                Text
                {
                    text: qsTr("The correlation plugin creates graphs based on how similar row profiles are in a dataset.\n\n"
                               + "If specified, the input data will be scaled and normalised and a Pearson Correlation will be performed. "
                               + "Pearson Correlation provides a measure of correlation between nodes which will be used to create "
                               + "an edge.\n\nCritera for edge creation can be set using Transforms once the graph is created\n\n")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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

            Text
            {
                text: qsTr("<h2>Pearson Correlation</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            Text
            {
                text: qsTr("Pearson Correlation will be performed on the dataset providing a measure of correlation between rows. "
                           + "1.0 represents highly correlated rows and 0.0 represents no correlation. Negative correlation values are discarded. "
                           + "All values below the Minimum correlation value will also be discarded and will not be in the generated graph.\n\n "
                           + "By default a transform is created which will create edges for all values above the minimum correlation threshold. "
                           + "Is is not possible to create edges using values below the minimum correlation value.\n")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Text
            {
                Layout.fillWidth: true
                text: qsTr("Please select a minimum correlation value");
                wrapMode: Text.WordWrap
            }

            RowLayout
            {
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

                    onValueChanged: { parameters.minimumCorrelation = value; slider.value = value; }
                }
                Slider
                {
                    Layout.fillWidth: true
                    id: slider
                    minimumValue: 0.0
                    maximumValue: 1.0
                    onValueChanged: minimumCorrelationSpinBox.value = value;
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
                text: qsTr("<h2>Data Transpose & Scaling</h2>")
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
                    text: qsTr("Please select if the data should be transposed and the required method to " +
                               "scale the data input. This will occur before normalisation")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                CheckBox
                {
                    id: transposeCheckBox

                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("Transpose Dataset")
                    onCheckedChanged: { parameters.transpose = checked; }
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
                        model: [ qsTr("None"), qsTr("Log2(x+c)"), qsTr("Log10(x+c)"),
                            qsTr("AntiLog2(x)"), qsTr("AntiLog10(x)"), qsTr("Arcsin(x)")]
                        onCurrentIndexChanged:
                        {
                            parameters.scaling = scalingStringToEnum(currentText);
                        }
                    }
                }

                GridLayout
                {
                    columns: 2
                    Text
                    {
                        text: "<b>Logb(x + c):</b>"
                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                        textFormat: Text.StyledText
                    }
                    Text
                    {
                        text: qsTr("Will perform a Log of x+c to base b. Where x is the input data and c is 4.96 x 10⁻³²⁴");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                    Text
                    {
                        text: "<b>AntiLogb(x):</b>"
                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                        textFormat: Text.StyledText
                    }
                    Text
                    {
                        text: qsTr("Will raise x to the power of b. Where x is the input data.");
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                    Text
                    {
                        text: "<b>Arcsin(x):</b>"
                        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                        textFormat: Text.StyledText
                    }
                    Text
                    {
                        text: qsTr("Will perform an inverse sine function of x. Where x is the input data. This is useful when "
                                   + "you require a Log transform but the dataset contains negatives or zeros.");
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
                               "This will occur after scaling")
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
                            console.log(parameters);
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

    Component.onCompleted: initialise();
    function initialise()
    {
        parameters = {minimumCorrelation: 0.7, transpose: false,
            scaling: ScalingType.None, normalise: NormaliseType.None};

        minimumCorrelationSpinBox.value = 0.7;
        transposeCheckBox.checked = false;
    }

    function scalingStringToEnum(scalingString)
    {
        switch(scalingString)
        {
        case qsTr("Log2(x+c)"):
            return ScalingType.Log2
        case qsTr("Log10(x+c)"):
            return ScalingType.Log10
        case qsTr("AntiLog2(x)"):
            return ScalingType.AntiLog2
        case qsTr("AntiLog10(x)"):
            return ScalingType.AntiLog10
        case qsTr("Arcsin(x)"):
            return ScalingType.ArcSin
        }
        return ScalingType.None
    }

    function normaliseStringToEnum(normaliseString)
    {
        switch(normaliseString)
        {
        case qsTr("MinMax"):
            return NormaliseType.MinMax
        case qsTr("Quantile"):
            return NormaliseType.Quantile
        }
        return NormaliseType.None;
    }
}
