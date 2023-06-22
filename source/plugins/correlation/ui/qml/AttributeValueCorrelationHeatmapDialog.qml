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
import QtQuick.Window

import app.graphia
import app.graphia.Controls
import app.graphia.Utils
import app.graphia.Shared
import app.graphia.Shared.Controls

import SortFilterProxyModel

Window
{
    id: root

    property var model: null
    property PluginContent pluginContent: null

    title: qsTr("Attribute Value Correlation")
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 500
    minimumHeight: 600

    function refresh()
    {
        if(!visible)
            return;

        // Keep the existing attribute selected, if it still exists
        let previousSelectedValue = attributeList.selectedValue;
        attributeList.model = pluginContent.availableAttributesModel();

        let modelIndex = attributeList.model.find(previousSelectedValue);
        if(modelIndex.valid)
        {
            attributeList.select(modelIndex);
            worker.start(attributeList.selectedValue);
        }
        else
        {
            maxSpinBox.to = 1000;
            heatmap.reset();
        }
    }

    onVisibleChanged:
    {
        if(visible)
            refresh();
        else if(worker.busy)
        {
            worker.cancel();
            attributeList.clearSelection();
        }
    }

    AttributeValueCorrelationHeatmapWorker
    {
        id: worker
        pluginModel: root.model

        onFinished:
        {
            heatmap.rebuild(worker.result);

            if(heatmap.numAttributeValues >= 2)
                maxSpinBox.to = heatmap.numAttributeValues;
        }
    }

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        RowLayout
        {
            Layout.fillWidth: true
            spacing: Constants.spacing

            Text
            {
                text: Utils.format(qsTr("Select an attribute below and a heatmap will be computed where " +
                    "each row and column represents a particular attribute value, as denoted by the " +
                    "labels on the axes. The content of the heatmap shows the {0} between the means " +
                    "of the data rows for each respective pair of attribute values. Clicking on a cell " +
                    "will select the associated nodes in the graph."),
                    QmlUtils.redirectLink("pearson", qsTr("Pearson Correlation Coefficient")))

                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                PointingCursorOnHoverLink {}
                onLinkActivated: function(link) { Qt.openUrlExternally(link); }
            }

            NamedIcon
            {
                iconName: "heatmap"

                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop
            }
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: Constants.spacing
            enabled: !worker.busy

            TreeComboBox
            {
                id: attributeList

                Layout.fillWidth: true

                placeholderText: qsTr("Select an Attribute")

                sortRoleName: "display"
                prettifyFunction: Attribute.prettify

                filters:
                [
                    ValueFilter
                    {
                        roleName: "elementType"
                        value: qsTr("Node")
                    },
                    ValueFilter
                    {
                        roleName: "hasSharedValues"
                        value: true
                    }

                ]

                onSelectedValueChanged:
                {
                    if(root.visible)
                        worker.start(selectedValue);
                }
            }

            Label
            {
                Layout.leftMargin: Constants.spacing
                text: qsTr("Maximum Values:")
            }

            SpinBox
            {
                id: maxSpinBox

                Layout.preferredWidth: 90

                value: 20
                from: 2
                editable: true
            }
        }

        Label
        {
            visible: !heatmap.visible && !worker.busy

            Layout.fillWidth: true
            Layout.fillHeight: true

            horizontalAlignment: Qt.AlignCenter
            verticalAlignment: Qt.AlignVCenter
            font.pixelSize: 16
            font.italic: true

            text: qsTr("Select an Attribute")
        }

        Item
        {
            visible: !heatmap.visible && worker.busy

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        AttributeValueCorrelationHeatmapItem
        {
            id: heatmap

            visible: valid
            maxNumAttributeValues: maxSpinBox.value
            elideLabelWidth: 100

            Layout.fillWidth: true
            Layout.fillHeight: true

            onCellClicked: function(a, b)
            {
                let term = "^(" + Utils.regexEscape(a) + "|" + Utils.regexEscape(b) + ")$"
                pluginContent.selectByAttributeValue(attributeList.selectedValue, term);
            }
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("Close")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }

    DelayedBusyIndicator
    {
        visible: worker.busy
        anchors.centerIn: layout
        width: 64
        height: 64

        delayedRunning: worker.busy
    }
}
