import QtQuick 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.2

import com.kajeka 1.0
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
    property DocumentUI document: null

    property string selectedAttributeGroupA: ""
    property string selectedAttributeGroupB: ""

    finishEnabled: (attributeSelectedAExclusiveGroup.current != null) && (attributeSelectedBExclusiveGroup.current != null)

    function reset()
    {
        // Reset on finish
        goToPage(0);
        scrollViewA.flickableItem.contentY = 0;
        scrollViewB.flickableItem.contentY = 0;
    }

    onVisibilityChanged:
    {
        reset();
        proxyModel.sourceModel = document.availableAttributes(ElementType.Node);
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

            function rowIndexForAttributeName(attributeName)
            {
                for(var i = 0; i < rowCount(); i++)
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
                        text: qsTr("Enrichment identifies the significance of a group makeup versus the null hypothesis.<br>" +
                                   "<br>" +
                                   "Two attribute groups will be selected to test for enrichment." +
                                   "<br>" +
                                   "The results will be presented in tabular form or using a heatmap.")
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
                                    for(var i = 0; i < attributeSelectBRepeater.count; i++)
                                    {
                                        var radioBtn = attributeSelectBRepeater.itemAt(i);
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
                                text: model.display + " (" + document.attribute(model.display).sharedValues.length + qsTr(" entries") + ")";
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
                                if(current != null && root.visible)
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
                                text: model.display + " (" + document.attribute(model.display).sharedValues.length + qsTr(" entries") + ")";
                                exclusiveGroup: attributeSelectedBExclusiveGroup
                            }
                        }
                    }
                }
            }
        }
    }
}
