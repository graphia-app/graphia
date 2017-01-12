import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import "Constants.js" as Constants

GridLayout
{
    columns: 2

    width: 500
    anchors.margins: Constants.margin

    Text
    {
        text: qsTr("Please select the parameters with which to build the correlation graph.")
        Layout.fillWidth: true
        Layout.columnSpan: 2
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
        implicitWidth: 100

        minimumValue: 0.0
        maximumValue: 1.0

        decimals: 3
        stepSize: (maximumValue - minimumValue) / 100.0

        onValueChanged: { settings.minimumCorrelation = value; }
    }

    function initialise()
    {
        minimumCorrelationSpinBox.value = 0.7;
    }
}
