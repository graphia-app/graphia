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
    width: 640
    height: 350
    minimumWidth: 640
    minimumHeight: 350

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

                onSelectedValuesChanged:
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

                PointingCursorOnHoverLink {}
                onLinkActivated: Qt.openUrlExternally(link);

                function update()
                {
                    text = "";

                    if(attributeList.selectedValue === undefined || attributeList.selectedValue.length === 0)
                        return;

                    var attribute = document.attribute(attributeList.selectedValue);

                    if(attribute.description === undefined)
                        return;

                    text += attribute.description;

                    if(channelList.selectedValues === undefined || channelList.selectedValues.length === 0)
                        return;

                    var visualisationDescriptions = document.visualisationDescription(
                        attributeList.selectedValue, channelList.selectedValues);

                    visualisationDescriptions.forEach(function(visualisationDescription)
                    {
                        text += "<br><br>" + visualisationDescription;
                    });
                }
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled:
                {
                    return visualisationExpressions.length > 0 &&
                        visualisationExpressions.every(document.visualisationIsValid);
                }
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
        var newVisualsiationExpressions = [];

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

            newVisualsiationExpressions.push(expression);
        });

        visualisationExpressions = newVisualsiationExpressions;
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
