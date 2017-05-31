import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import "Constants.js" as Constants

GridLayout
{
    columns: 3

    width: 500
    anchors.margins: Constants.margin

    Text
    {
        text: qsTr("Please select the parameters with which to build the correlation graph.")
        Layout.fillWidth: true
        Layout.columnSpan: 3
        wrapMode: Text.WordWrap
    }

    ColumnLayout
    {
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
                model: [ qsTr("None"), qsTr("Log2(x+c)"), qsTr("Log10(x+c)"), qsTr("AntiLog2(x)"), qsTr("AntiLog10(x)"), qsTr("Arcsin(x)")]
                onCurrentIndexChanged:
                {
                    parameters.scaling = scalingStringToEnum(currentText);
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
                model: [ qsTr("None"), qsTr("MinMax")]
                onCurrentIndexChanged:
                {
                    parameters.normalise = normaliseStringToEnum(currentText);
                }
            }

            CheckBox
            {
                id: transposeCheckBox

                Layout.alignment: Qt.AlignLeft
                text: qsTr("Transpose")
                onCheckedChanged: { parameters.transpose = checked; }
            }
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
                id: slider
                minimumValue: 0.0
                maximumValue: 1.0
                onValueChanged: minimumCorrelationSpinBox.value = value;
            }
        }
    }


    function initialise()
    {
        parameters = {minimumCorrelation: 0.7, transpose: false, scaling: ScalingType.None, normalise: NormaliseType.None};

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
        }
        return NormaliseType.None;
    }
}
