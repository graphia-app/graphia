/* Copyright © 2013-2024 Graphia Technologies Ltd.
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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import app.graphia.Utils

Window
{
    id: root

    title: qsTr("Edit Mapping")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    minimumWidth: 640
    minimumHeight: 400

    property bool applied: false

    // The window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    property var values: []
    property bool invert: false

    property double _minimumValue: 0.0
    property double _maximumValue: 0.0

    function initialise(value)
    {
        if(value.length === 0)
            return;

        let mapping = JSON.parse(value);

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

        if(typeGroup.checkedButton === null)
            userDefinedRadioButton.checked = true;

        mappingPlot.minimum = Utils.clamp(mappingPlot.minimum, root._minimumValue, root._maximumValue);
        mappingPlot.maximum = Utils.clamp(mappingPlot.maximum, root._minimumValue, root._maximumValue);

        if(mappingPlot.minimum === mappingPlot.maximum)
        {
            mappingPlot.minimum = root._minimumValue;
            mappingPlot.maximum = root._maximumValue;
        }

        if(mapping.exponent !== undefined)
            exponentSlider.value = Math.log2(mapping.exponent);
    }

    onValuesChanged:
    {
        _minimumValue = Math.min.apply(null, values);
        _maximumValue = Math.max.apply(null, values);

        mappingPlot.minimum = Math.max(mappingPlot.minimum, root._minimumValue);
        mappingPlot.maximum = Math.min(mappingPlot.maximum, root._maximumValue);
    }

    property double _exponent:
    {
        return Math.pow(2.0, exponentSlider.value);
    }

    function reset()
    {
        minmaxRadioButton.checked = true;
        mappingPlot.minimum = root._minimumValue;
        mappingPlot.maximum = root._maximumValue;
        exponentSlider.value = 0;
    }

    property string configuration:
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
    signal applyClicked(bool alreadyApplied)

    onVisibleChanged:
    {
        // When the window is first shown
        if(visible)
            root.applied = false;
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
                    color: palette.buttonText

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
                    ButtonGroup
                    {
                        id: typeGroup
                        buttons: [minmaxRadioButton, stddevRadioButton, userDefinedRadioButton]
                    }

                    RadioButton
                    {
                        id: minmaxRadioButton
                        text: qsTr("Minimum/Maximum")

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

                        onCheckedChanged:
                        {
                            if(checked)
                                mappingPlot.setRangeToStddev();
                        }
                    }

                    RadioButton
                    {
                        id: userDefinedRadioButton
                        text: qsTr("User Defined")
                    }

                    RowLayout
                    {
                        Layout.leftMargin: Constants.margin * 2

                        DoubleSpinBox
                        {
                            id: minimumSpinBox
                            Layout.preferredWidth: 90
                            editable: true

                            enabled: userDefinedRadioButton.checked

                            from: root._minimumValue
                            to: root._maximumValue
                            decimals: { return Utils.decimalPointsForRange(from, to); }
                            stepSize: { return Utils.incrementForRange(from, to); }

                            onValueModified: { mappingPlot.minimum = value; }
                        }

                        DoubleSpinBox
                        {
                            id: maximumSpinBox
                            Layout.preferredWidth: 90
                            editable: true

                            enabled: userDefinedRadioButton.checked

                            from: root._minimumValue
                            to: root._maximumValue
                            decimals: { return Utils.decimalPointsForRange(from, to); }
                            stepSize: { return Utils.incrementForRange(from, to); }

                            onValueModified: { mappingPlot.maximum = value; }
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
                    from: -4
                    to: 4
                    stepSize: 1
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

                onMinimumChanged: { minimumSpinBox.value = minimum; }
                onMaximumChanged: { maximumSpinBox.value = maximum; }
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
                onClicked: function(mouse)
                {
                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked: function(mouse)
                {
                    rejected();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Apply")
                onClicked: function(mouse)
                {
                    applyClicked(root.applied);
                    root.applied = true;
                }
            }
        }
    }
}
