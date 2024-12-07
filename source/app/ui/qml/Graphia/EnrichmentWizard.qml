/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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
import QtQuick.Dialogs
import QtQuick.Window

import Graphia.Controls
import Graphia.SharedTypes
import Graphia.Utils

Wizard
{
    id: root
    minimumWidth: 640
    minimumHeight: 400

    // Must be set before opening
    property var attributeGroups: null
    property var document: null

    property string selectedAttributeGroupA: ""
    property string selectedAttributeGroupB: ""

    nextEnabled: proxyModel.count > 1
    finishEnabled: selectedAttributeGroupA.length > 0 && selectedAttributeGroupB.length > 0

    function reset()
    {
        // Reset on finish
        goToPage(0);
        scrollViewA.resetScrollPosition();
        scrollViewB.resetScrollPosition();
    }

    onVisibleChanged:
    {
        reset();

        if(visible)
        {
            root.selectedAttributeGroupA = root.selectedAttributeGroupB = "";
            proxyModel.model = document.availableAttributesModel(ElementType.Node);
        }
        else
            proxyModel.model = null;
    }

    Item
    {
        SortFilterProxyModel
        {
            id: proxyModel

            property var model: null
            sourceModel: model

            filterRoleName: "hasSharedValues"
            filterRegularExpression: /true/

            sortExpression: function(left, right)
            {
                let leftAttributeName = model.data(left);
                let rightAttributeName = model.data(right);
                let leftCount = document.attribute(leftAttributeName).sharedValues.length;
                let rightCount = document.attribute(rightAttributeName).sharedValues.length;
                return leftCount < rightCount;
            }

            function rowIndexForAttributeName(attributeName)
            {
                for(let i = 0; i < rowCount(); i++)
                {
                    if(data(index(i, 0)) === attributeName)
                        return i;
                }
            }
        }

        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right
            Text
            {
                text: qsTr("<h2>Enrichment</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
                color: palette.buttonText
            }

            RowLayout
            {
                ColumnLayout
                {
                    Text
                    {
                        // Should expand this later
                        text:
                        {
                            let desc = qsTr("Enrichment identifies the significance of a group makeup versus the null hypothesis.<br>" +
                                   "<br>" +
                                   "Two attribute groups will be selected to test for enrichment." +
                                   "<br>" +
                                   "The results will be presented in tabular form or using a heatmap.<br>");
                            if(proxyModel.count < 2)
                            {
                                desc += qsTr("<br><font color=\"red\">This dataset does not have enough attribute groups to perform enrichment." +
                                            " At least two groups are required.</font>");
                            }
                            return desc;
                        }
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
                        color: palette.buttonText
                    }

                }

                Image
                {
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumWidth: 100
                    Layout.minimumHeight: 100
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: 100
                    source: "qrc:///imagery/bellcurve.svg"
                }
            }
        }
    }

    Item
    {
        ColumnLayout
        {
            anchors.fill: parent

            Text
            {
                text: qsTr("<h2>Enrichment Attribute - A</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
                color: palette.buttonText
            }

            ColumnLayout
            {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 20

                Text
                {
                    text: qsTr("Please select the first attribute group to test for enrichment:")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                    color: palette.buttonText
                }

                FramedScrollView
                {
                    id: scrollViewA

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout
                    {
                        spacing: 0

                        ButtonGroup
                        {
                            id: attributeSelectedAButtonGroup
                            onCheckedButtonChanged:
                            {
                                if(checkedButton !== null && root.visible)
                                {
                                    selectedAttributeGroupA = checkedButton.attributeName;

                                    // Disable analysis on selected
                                    for(let i = 0; i < attributeSelectBRepeater.count; i++)
                                    {
                                        let radioBtn = attributeSelectBRepeater.itemAt(i);
                                        radioBtn.enabled = radioBtn.attributeName !== checkedButton.attributeName;
                                    }
                                }
                            }
                        }

                        Repeater
                        {
                            id: attributeSelectARepeater
                            model: proxyModel

                            RadioButton
                            {
                                rightPadding: scrollViewA.scrollBarWidth

                                property var attributeName: model.display

                                text:
                                {
                                    if(document && model.display && model.display.length > 0)
                                    {
                                        return Utils.format(qsTr("{0} ({1} entries)"), model.display,
                                            document.attribute(model.display).sharedValues.length);
                                    }

                                    return "";
                                }

                                ButtonGroup.group: attributeSelectedAButtonGroup
                            }
                        }
                    }
                }
            }
        }
    }

    Item
    {
        Layout.fillHeight: true
        ColumnLayout
        {
            anchors.fill: parent

            Text
            {
                text: qsTr("<h2>Enrichment Attribute - B</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
                color: palette.buttonText
            }

            ColumnLayout
            {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 20

                Text
                {
                    text: qsTr("Please select the second attribute group to test for enrichment:")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                    color: palette.buttonText
                }

                FramedScrollView
                {
                    id: scrollViewB

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout
                    {
                        spacing: 0

                        ButtonGroup
                        {
                            id: attributeSelectedBButtonGroup
                            onCheckedButtonChanged:
                            {
                                if(checkedButton !== null && root.visible)
                                    selectedAttributeGroupB = checkedButton.attributeName;
                            }
                        }

                        Repeater
                        {
                            id: attributeSelectBRepeater
                            model: proxyModel

                            RadioButton
                            {
                                rightPadding: scrollViewB.scrollBarWidth

                                property var attributeName: model.display

                                text:
                                {
                                    if(document && model.display && model.display.length > 0)
                                    {
                                        return Utils.format(qsTr("{0} ({1} entries)"), model.display,
                                            document.attribute(model.display).sharedValues.length);
                                    }

                                    return "";
                                }

                                ButtonGroup.group: attributeSelectedBButtonGroup
                            }
                        }
                    }
                }
            }
        }
    }
}
