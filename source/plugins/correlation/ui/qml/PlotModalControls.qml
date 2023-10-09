/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

Rectangle
{
    id: root

    property CorrelationPlot plot: null

    readonly property alias roiPercentile: percentileSlider.value
    readonly property alias roiWeight: weightSlider.value

    Preferences
    {
        section: "correlation"

        property alias rowsOfInterestPercentile: percentileSlider.value
        property alias rowsOfInterestWeight: weightSlider.value
    }

    // Don't pass clicks through
    MouseArea { anchors.fill: parent }

    implicitWidth: layout.implicitWidth + (Constants.padding * 2)
    implicitHeight: layout.implicitHeight + (Constants.padding * 2)

    width: implicitWidth + (Constants.margin * 4)
    height: implicitHeight + (Constants.margin * 4)

    border.color: ControlColors.outline
    border.width: 1
    radius: 4
    color: ControlColors.background

    Action
    {
        id: closeAction
        icon.name: "emblem-unreadable"

        onTriggered: function(source) { closed(); }
    }

    RowLayout
    {
        id: layout

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Constants.padding

        RowLayout
        {
            id: columnAnnotationSelectionControls

            Button
            {
                text: qsTr("Show All Annotations")
                onClicked: function(mouse) { plot.showAllColumnAnnotations(); }
            }

            Button
            {
                text: qsTr("Hide All Annotations")
                onClicked: function(mouse) { plot.hideAllColumnAnnotations(); }
            }
        }

        RowLayout
        {
            id: rowsOfInterestColumnSelectionControls
            enabled: plot.selectedColumns.length > 0

            Text
            {
                color: palette.buttonText
                text: qsTr("Relative")
            }

            Slider
            {
                id: weightSlider

                live: false
                value: 0
                from: -29
                to: 29
                stepSize: 0.2

                onValueChanged: { plot.selectRowsOfInterest(); }
            }

            Text
            {
                color: palette.buttonText
                text: qsTr("Absolute")
            }

            HelpTooltip
            {
                Layout.rightMargin: Constants.spacing

                title: qsTr("Selection Weighting")
                Text
                {
                    wrapMode: Text.WordWrap
                    text: qsTr("Rows are selected on the basis of weighting the sum of the " +
                        "selected columns against the non-selected columns. Weighting for " +
                        "the selected columns (denoted <i>Absolute</i>) tends to find rows which have " +
                        "peaks in those columns, regardless of the non-selected columns. In " +
                        "contrast, (negatively) weighting for the non-selected columns " +
                        "(denoted <i>Relative</i>) tends to find rows which have significant values " +
                        "specifically in the selected columns.")
                }
            }

            Text
            {
                Layout.preferredWidth: 40
                horizontalAlignment: Text.AlignRight
                color: palette.buttonText

                text:
                {
                    // Reverse engineer the value from the position so that
                    // the text updates live, even if the actual value doesn't
                    let i = Math.round(Utils.lerp(percentileSlider.from,
                        percentileSlider.to, percentileSlider.position));

                    let j = i % 10;
                    let k = i % 100;

                    if(j == 1 && k != 11)
                      return i + qsTr("st");

                    if(j == 2 && k != 12)
                      return i + qsTr("nd");

                    if(j == 3 && k != 13)
                      return i + qsTr("rd");

                    return i + qsTr("th");
                }
            }

            Slider
            {
                id: percentileSlider

                live: false
                value: 5
                from: 100
                to: 1
                stepSize: 1

                onValueChanged: { plot.selectRowsOfInterest(); }
            }

            HelpTooltip
            {
                Layout.rightMargin: Constants.spacing

                title: qsTr("Percentile")
                Text
                {
                    wrapMode: Text.WordWrap
                    text: qsTr("The quantity of rows to select and rank.")
                }
            }

            Button
            {
                visible: plot.selectedRowsDifferFromRowsOfInterest
                text: qsTr("Reselect")
                onClicked: { plot.selectRowsOfInterest(); }
            }
        }

        FloatingButton { action: closeAction }
    }

    function show()
    {
        columnAnnotationSelectionControls.visible = (plot.plotMode === PlotMode.ColumnAnnotationSelection);
        rowsOfInterestColumnSelectionControls.visible = (plot.plotMode === PlotMode.RowsOfInterestColumnSelection);

        // Hack to delay emitting shown() until the implicit
        // size of the item has had a chance to be resolved
        Qt.callLater(() => Qt.callLater(() => shown()));
    }

    function hide() { hidden(); }

    signal shown();
    signal hidden();
    signal closed();
}
