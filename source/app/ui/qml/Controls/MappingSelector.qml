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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import app.graphia 1.0
import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

Window
{
    id: root

    title: qsTr("Edit Mapping")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 640
    minimumHeight: 400

    property string configuration
    property string _initialConfiguration

    // The window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    property var values: []
    property bool invert: false

    property double _minimumValue: 0.0
    property double _maximumValue: 0.0

    function setup()
    {
        if(root.configuration.length === 0)
            return;

        var mapping = JSON.parse(root.configuration);

        if(mapping.min !== undefined && mapping.max !== undefined)
        {
            mappingPlot.minimum = mapping.min;
            mappingPlot.maximum = mapping.max;
        }

        if(mapping.type !== undefined)
        {
            if(mapping.type === "minmax")
                minmaxRadioButton.checked = true;
            else if(mapping.type === "stddev")
                stddevRadioButton.checked = true;
        }

        if(typeGroup.current === null)
            userDefinedRadioButton.checked = true;

        mappingPlot.minimum = Math.max(mappingPlot.minimum, root._minimumValue);
        mappingPlot.maximum = Math.min(mappingPlot.maximum, root._maximumValue);

        if(mapping.exponent !== undefined)
            exponentSlider.value = Math.log2(mapping.exponent);
    }

    onValuesChanged:
    {
        _minimumValue = Math.min.apply(null, values);
        _maximumValue = Math.max.apply(null, values);

        setup();
    }

    property double _exponent:
    {
        return Math.pow(2.0, exponentSlider.value);
    }

    onConfigurationChanged:
    {
        setup();
    }

    function resetConfiguration()
    {
        minmaxRadioButton.checked = true;
        mappingPlot.minimum = root._minimumValue;
        mappingPlot.maximum = root._maximumValue;
        exponentSlider.value = 0;
    }

    property string _selectedConfiguration:
    {
        let mapping = {};

        if(minmaxRadioButton.checked)
            mapping.type = "minmax";
        else if(stddevRadioButton.checked)
            mapping.type = "stddev";
        else
        {
            mapping.min = mappingPlot.minimum;
            mapping.max = mappingPlot.maximum;
        }

        mapping.exponent = root._exponent;

        return JSON.stringify(mapping);
    }

    signal accepted()
    signal rejected()

    onVisibleChanged:
    {
        // When the window is first shown
        if(visible)
            root._initialConfiguration = root.configuration;
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            Layout.fillHeight: true

            ColumnLayout
            {
                spacing: Constants.spacing

                Text
                {
                    Layout.preferredWidth: 250
                    text: qsTr("The Y-axis of the plot shows the visualised " +
                        "attribute's values and their distribution. The X-axis " +
                        "represents the value of the chosen visualisation " +
                        "parameter. Choose one of the automatic range types " +
                        "below or manually select boundaries by dragging " +
                        "the dotted lines.")
                    wrapMode: Text.WordWrap
                }

                Label
                {
                    font.bold: true
                    text: qsTr("Type")
                }

                ColumnLayout
                {
                    ExclusiveGroup { id: typeGroup }

                    RadioButton
                    {
                        id: minmaxRadioButton
                        text: qsTr("Minimum/Maximum")
                        exclusiveGroup: typeGroup

                        onCheckedChanged:
                        {
                            if(checked)
                                mappingPlot.setRangeToMinMax();
                        }
                    }

                    RadioButton
                    {
                        id: stddevRadioButton
                        text: qsTr("Standard Deviation")
                        exclusiveGroup: typeGroup

                        onCheckedChanged:
                        {
                            if(checked)
                                mappingPlot.setRangeToStddev();
                        }
                    }

                    RowLayout
                    {
                        RadioButton
                        {
                            id: userDefinedRadioButton
                            text: qsTr("User Defined")
                            exclusiveGroup: typeGroup
                        }

                        SpinBox
                        {
                            enabled: userDefinedRadioButton.checked
                            value: mappingPlot.minimum

                            minimumValue: root._minimumValue
                            maximumValue: root._maximumValue
                            decimals: { return Utils.decimalPointsForRange(minimumValue, maximumValue); }
                            stepSize: { return Utils.incrementForRange(minimumValue, maximumValue); }

                            onEditingFinished: { mappingPlot.minimum = value; }
                        }

                        SpinBox
                        {
                            enabled: userDefinedRadioButton.checked
                            value: mappingPlot.maximum

                            minimumValue: root._minimumValue
                            maximumValue: root._maximumValue
                            decimals: { return Utils.decimalPointsForRange(minimumValue, maximumValue); }
                            stepSize: { return Utils.incrementForRange(minimumValue, maximumValue); }

                            onEditingFinished: { mappingPlot.maximum = value; }
                        }
                    }
                }

                Label
                {
                    font.bold: true
                    text: qsTr("Curve")
                }

                Slider
                {
                    id: exponentSlider
                    minimumValue: -4
                    maximumValue: 4
                    stepSize: 1
                    tickmarksEnabled: true
                }

                Item { Layout.fillHeight: true }
            }

            VisualisationMappingPlot
            {
                id: mappingPlot

                Layout.fillWidth: true
                Layout.fillHeight: true

                values: root.values
                invert: root.invert
                exponent: root._exponent

                onManualChangeToMinMax: { userDefinedRadioButton.checked = true; }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("OK")
                onClicked:
                {
                    root.configuration = root._selectedConfiguration;

                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked:
                {
                    if(root._initialConfiguration !== null && root._initialConfiguration !== "")
                        root.configuration = root._initialConfiguration;

                    rejected();
                    root.close();
                }
            }
        }
    }
}
