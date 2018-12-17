import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

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
    property var visualisationExpressions: []

    Preferences
    {
        section: "misc"
        property alias visualisationAttributeSortOrder: attributeList.ascendingSortOrder
        property alias visualisationAttributeSortBy: attributeList.sortRoleName
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            TreeBox
            {
                id: attributeList
                Layout.fillWidth: true
                Layout.fillHeight: true

                showSections: sortRoleName !== "display"
                sortRoleName: "elementType"

                onSelectedValueChanged:
                {
                    var attribute = document.attribute(selectedValue);
                    channelList.model = document.availableVisualisationChannelNames(attribute.valueType);
                    description.update();
                    updateVisualisationExpressions();
                }

                AttributeListSortMenu { attributeList: attributeList }
            }

            ListBox
            {
                id: channelList
                Layout.fillWidth: true
                Layout.fillHeight: true

                allowMultipleSelection: true

                onSelectedValueChanged:
                {
                    description.update();
                    updateVisualisationExpressions();
                }
            }

            Text
            {
                id: description
                Layout.fillHeight: true
                Layout.preferredWidth: 200

                textFormat: Text.StyledText
                wrapMode: Text.WordWrap

                onLinkActivated: Qt.openUrlExternally(link);

                function update()
                {
                    text = "";

                    if(attributeList.selectedValue !== undefined && attributeList.selectedValue.length > 0)
                    {
                        var attribute = document.attribute(attributeList.selectedValue);
                        text += attribute.description;

                        if(channelList.selectedValue !== undefined && channelList.selectedValue.length > 0)
                        {
                            var visualisationDescription = document.visualisationDescription(
                                attributeList.selectedValue, channelList.selectedValue);
                            text += "<br><br>" + visualisationDescription;
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: { return visualisationExpressions.every(document.visualisationIsValid); }
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

    function updateVisualisationExpressions()
    {
        visualisationExpressions = [];

        channelList.selectedValues.forEach(function(channelName)
        {
            var expression = "\"" + attributeList.selectedValue + "\" \"" + channelName +"\"";

            var attribute = document.attribute(attributeList.selectedValue);
            var parameters = document.visualisationDefaultParameters(attribute.valueType,
                                                                     channelName);

            if(Object.keys(parameters).length !== 0)
                expression += " with";

            for(var key in parameters)
            {
                var parameter = parameters[key];
                parameter = Utils.sanitiseJson(parameter);
                parameter = Utils.escapeQuotes(parameter);

                expression += " " + key + " = \"" + parameter + "\"";
            }

            visualisationExpressions.push(expression);
        });
    }

    onAccepted:
    {
        updateVisualisationExpressions();
        document.update([], visualisationExpressions);
    }

    onVisibleChanged:
    {
        if(visible)
            attributeList.model = document.availableAttributesModel(ElementType.Node|ElementType.Edge);
    }
}
