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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import app.graphia 1.0

import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "VisualisationUtils.js" as VisualisationUtils
import "../AttributeUtils.js" as AttributeUtils

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

    property bool visualisationExpressionsValid:
    {
        return visualisationExpressions.length > 0 &&
            visualisationExpressions.every(document.visualisationIsValid);
    }

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
                showSearch: true
                showParentGuide: true
                sortRoleName: "elementType"
                prettifyFunction: AttributeUtils.prettify

                onSelectedValueChanged:
                {
                    let attribute = document.attribute(selectedValue);

                    if(currentIndexIsSelectable && attribute.isValid)
                    {
                        channelList.model = document.availableVisualisationChannelNames(
                            attribute.valueType);
                    }
                    else
                        channelList.model = null;

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

                onAccepted: { root.accept(); }
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

                    if(attributeList.selectedValue === undefined || !attributeList.currentIndexIsSelectable)
                        return;

                    let attribute = document.attribute(attributeList.selectedValue);

                    if(attribute.description === undefined)
                        return;

                    text += attribute.description;

                    if(channelList.selectedValues === undefined || channelList.selectedValues.length === 0)
                        return;

                    let visualisationDescriptions = document.visualisationDescription(
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
                enabled: root.visualisationExpressionsValid
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
        if(!root.visualisationExpressionsValid)
            return;

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
        let newVisualsiationExpressions = [];

        channelList.selectedValues.forEach(function(channelName)
        {
            let attribute = document.attribute(attributeList.selectedValue);

            let expression = VisualisationUtils.expressionFor(
                document, attributeList.selectedValue, attribute.flags,
                attribute.valueType, channelName);

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
        {
            attributeList.model = document.availableAttributesModel(ElementType.Node|ElementType.Edge);
            channelList.model = null;
        }
    }
}
