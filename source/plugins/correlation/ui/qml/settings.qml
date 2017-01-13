import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

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

        onValueChanged: { settings.minimumCorrelation = value; }
    }

    CheckBox
    {
        id: transposeCheckBox

        Layout.alignment: Qt.AlignLeft
        text: qsTr("Transpose")
        onCheckedChanged: { settings.transpose = checked; }
    }

    function initialise()
    {
        settings = {minimumCorrelation: 0.7, transpose: false};

        minimumCorrelationSpinBox.value = 0.7;
        transposeCheckBox.checked = false;
    }
}
