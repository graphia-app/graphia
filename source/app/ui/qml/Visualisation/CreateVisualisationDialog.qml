import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../Constants.js" as Constants
import "../Utils.js" as Utils

import "../Controls"

Window
{
    id: root

    title: qsTr("Add Visualisation")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 600
    height: 250
    minimumWidth: 600
    minimumHeight: 250

    property var document
    property string visualisationExpression

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            ListBox
            {
                id: attributeList
                Layout.fillWidth: true
                Layout.fillHeight: true

                onSelectedValueChanged:
                {
                    channelList.model = document.availableVisualisationChannelNames(selectedValue);
                    updateVisualisationExpression();
                }
            }

            ListBox
            {
                id: channelList
                Layout.fillWidth: true
                Layout.fillHeight: true

                onSelectedValueChanged: { updateVisualisationExpression(); }
            }

            Text
            {
                id: description
                Layout.fillHeight: true
                Layout.preferredWidth: 200

                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                elide: Text.ElideRight

                onLinkActivated: Qt.openUrlExternally(link);
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: { return document.visualisationIsValid(visualisationExpression); }
                onClicked: { root.accept(); }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.reject(); }
            }
        }

        Keys.onPressed:
        {
            event.accepted = true;
            switch(event.key)
            {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function accept()
    {
        accepted();
        root.close();
    }

    function reject()
    {
        rejected();
        root.close();
    }

    signal accepted()
    signal rejected()

    function updateVisualisationExpression()
    {
        description.text = "";

        if(attributeList.selectedValue !== undefined && attributeList.selectedValue.length > 0)
        {
            var attribute = document.attributeByName(attributeList.selectedValue);
            description.text += attribute.description;

            if(channelList.selectedValue !== undefined && channelList.selectedValue.length > 0)
            {
                var visualisationDescription = document.visualisationDescription(
                    attributeList.selectedValue, channelList.selectedValue);
                description.text += "<br><br>" + visualisationDescription;
            }
        }

        var expression = "\"" + attributeList.selectedValue + "\" \"" + channelList.selectedValue + "\"";

        visualisationExpression = expression;
    }

    onAccepted:
    {
        updateVisualisationExpression();
        document.appendVisualisation(visualisationExpression);
        document.updateVisualisations();
    }

    onVisibleChanged:
    {
        attributeList.model = document.availableAttributes(ElementType.Node|ElementType.Edge);
    }
}
