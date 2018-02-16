import QtQuick 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1


import com.kajeka 1.0
import "../Controls"

Wizard
{
    id: root
    minimumWidth: 640
    minimumHeight: 400

    // Must be set before opening
    property var attributeGroups

    property var selectedAttributeGroupsAgainst: []
    property var selectedAttributeGroup: []
    property bool attributesSelected: false

    finishEnabled: attributesSelected

    function reset()
    {
        // Reset on finish
        goToPage(0);
        for(var i=0; i < attributeAgainstRepeater.count; i++)
            attributeAgainstRepeater.itemAt(i).checked = false;
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
                ColumnLayout
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
                    }
                }

                Image
                {
                    anchors.top: parent.top
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
                                    attributesSelected = (selectedAttributeGroupsAgainst.length > 0);
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
                            onCurrentChanged:
                            {
                                selectedAttributeGroup = current.text;
                            }
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
