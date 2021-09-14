/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

import QtQuick 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.2

import app.graphia 1.0
import "../Controls"
import "../"

import SortFilterProxyModel 0.2

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
    finishEnabled: (attributeSelectedAExclusiveGroup.current !== null) && (attributeSelectedBExclusiveGroup.current !== null)

    function reset()
    {
        // Reset on finish
        goToPage(0);
        scrollViewA.flickableItem.contentY = 0;
        scrollViewB.flickableItem.contentY = 0;
    }

    onVisibleChanged:
    {
        reset();
        proxyModel.sourceModel = document.availableAttributesModel(ElementType.Node);
    }

    Item
    {
        SortFilterProxyModel
        {
            id: proxyModel
            filters:
            [
                ValueFilter
                {
                    roleName: "hasSharedValues"
                    value: true
                }
            ]
            sorters: ExpressionSorter
            {
                expression:
                {
                    // QML expressions are first evaluated with no captured variables
                    // and the required variables are deduced after. If the variable is not
                    // used during this no-context stage it cannot be captured hence this
                    // seemingly pointless assignment
                    let tempDoc = document;
                    if(tempDoc === null)
                        return false;
                    let leftCount = document.attribute(modelLeft.display).sharedValues.length;
                    let rightCount = document.attribute(modelRight.display).sharedValues.length;
                    return leftCount < rightCount;
                }
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
                                            " At least two groups are required</font>");
                            }
                            return desc;
                        }
                        wrapMode: Text.WordWrap
                        textFormat: Text.StyledText
                        Layout.fillWidth: true
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
                }

                ScrollView
                {
                    id: scrollViewA
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    frameVisible: true
                    ColumnLayout
                    {
                        ExclusiveGroup
                        {
                            id: attributeSelectedAExclusiveGroup
                            onCurrentChanged:
                            {
                                if(current !== null && root.visible)
                                {
                                    selectedAttributeGroupA = current.attributeName;

                                    // Disable analysis on selected
                                    for(let i = 0; i < attributeSelectBRepeater.count; i++)
                                    {
                                        let radioBtn = attributeSelectBRepeater.itemAt(i);
                                        radioBtn.enabled = radioBtn.attributeName !== current.attributeName;
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
                                property var attributeName: model.display

                                text:
                                {
                                    if(document !== null && model.display.length > 0)
                                    {
                                        return model.display + qsTr(" (") +
                                            document.attribute(model.display).sharedValues.length +
                                            qsTr(" entries)");
                                    }

                                    return "";
                                }

                                exclusiveGroup: attributeSelectedAExclusiveGroup
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
                }

                ScrollView
                {
                    id: scrollViewB
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    frameVisible: true
                    ColumnLayout
                    {
                        ExclusiveGroup
                        {
                            id: attributeSelectedBExclusiveGroup
                            onCurrentChanged:
                            {
                                if(current !== null && root.visible)
                                    selectedAttributeGroupB = current.attributeName;
                            }
                        }

                        Repeater
                        {
                            id: attributeSelectBRepeater
                            model: proxyModel
                            RadioButton
                            {
                                property var attributeName: model.display

                                text:
                                {
                                    if(document !== null && model.display.length > 0)
                                    {
                                        return model.display + qsTr(" (") +
                                            document.attribute(model.display).sharedValues.length +
                                            qsTr(" entries)");
                                    }

                                    return "";
                                }

                                exclusiveGroup: attributeSelectedBExclusiveGroup
                            }
                        }
                    }
                }
            }
        }
    }
}
