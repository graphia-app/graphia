import QtQuick 2.0
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0
import "../Controls"

Wizard
{
    id: root
    minimumWidth: 640
    minimumHeight: 400
    property var attributeGroups
    property var selectedAttributeGroupsAgainst: []
    property var selectedAttributeGroup: []
    finishEnabled: false

    onAttributeGroupsChanged:
    {
        console.log("Attribute Groups");
        console.log(attributeGroups);
    }

    Item
    {
        anchors.fill: parent
        ColumnLayout
        {
            width: parent.width
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
                Text
                {
                    text: qsTr("Enrichment identifies the significance of a group makeup versus the null hypothesis<br>" +
                               "<br>" +
                               "An attribute group will be selected to test for enrichment" +
                               "<br>" +
                               "The edges may be filtered using transforms once the graph has been created.")
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    Layout.fillWidth: true

                    onLinkActivated: Qt.openUrlExternally(link);
                }

                Image
                {
                    anchors.top: parent.top
                    Layout.minimumWidth: 100
                    Layout.minimumHeight: 100
                    sourceSize.width: 100
                    sourceSize.height: 100
                    source: "../plots.svg"
                }
            }
        }
    }

    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Enrichment Attribute Against</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 20

                Text
                {
                    text: qsTr("Please select the attribute group(s) to perform<br>"+
                               "enrichment analysis against")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }

                ScrollView
                {
                    Layout.fillWidth: true
                    ColumnLayout
                    {
                        Repeater
                        {
                            id: attributeAgainstRepeater
                            model: attributeGroups
                            CheckBox
                            {
                                text: modelData
                                onCheckedChanged:
                                {
                                    if(checked)
                                        selectedAttributeGroupsAgainst.push(modelData)
                                    else
                                    {
                                        var index = selectedAttributeGroupsAgainst.indexOf(modelData);
                                        selectedAttributeGroupsAgainst.splice(index, 1);
                                    }
                                    root.finishEnabled = selectedAttributeGroupsAgainst.length > 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Item
    {
        ColumnLayout
        {
            anchors.left: parent.left
            anchors.right: parent.right

            Text
            {
                text: qsTr("<h2>Enrichment Attribute On</h2>")
                Layout.alignment: Qt.AlignLeft
                textFormat: Text.StyledText
            }

            ColumnLayout
            {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 20

                Text
                {
                    text: qsTr("Please select the attribute group(s) to perform<br>"+
                               "enrichment analysis on")
                    Layout.alignment: Qt.AlignLeft
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }

                ScrollView
                {
                    Layout.fillWidth: true
                    ColumnLayout
                    {
                        ExclusiveGroup
                        {
                            id: attributeSelectedExGroup
                        }

                        Repeater
                        {
                            id: attributeSelectedRepeater
                            model: attributeGroups
                            RadioButton
                            {
                                text: modelData
                                exclusiveGroup: attributeSelectedExGroup
                            }
                        }
                    }
                }
            }
        }
    }
}
