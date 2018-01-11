import QtQuick 2.0
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0
import "Controls"

Wizard
{
    minimumWidth: 640
    minimumHeight: 400
    property var attributeGroups;
    property var selectedAttributeGroupsAgainst;
    property var selectedAttributeGroup;

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

    onAccepted:
    {
        // Find the selected nodes attribute to test on
        var selectedAttribute;
        for(var i=0; i<attributeSelectedRepeater.model.length; i++)
        {
            if(attributeSelectedRepeater.itemAt(i).checked)
            {
                selectedAttribute = attributeSelectedRepeater.itemAt(i).text;
                break;
            }
        }
        selectedAttributeGroup = selectedAttribute;

        // Find the attribute to test against
        var selectedAgainstAttributes = [];
        for(var i=0; i<attributeAgainstRepeater.model.length; i++)
        {
            if(attributeAgainstRepeater.itemAt(i).checked)
                selectedAgainstAttributes.push(attributeAgainstRepeater.itemAt(i).text);
        }
        selectedAttributeGroupsAgainst = selectedAgainstAttributes;
    }
}
