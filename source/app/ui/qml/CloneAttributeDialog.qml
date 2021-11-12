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

import SortFilterProxyModel 0.2

import "../../../shared/ui/qml/Constants.js" as Constants
import "AttributeUtils.js" as AttributeUtils

import "Controls"

Window
{
    id: root

    property var document: null

    title: qsTr("Clone Attribute")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 400
    minimumHeight: 250

    onVisibleChanged:
    {
        if(visible)
        {
            attributeList.model = document.availableAttributesModel();
            newAttributeName.text = "";
            newAttributeName.manuallyChanged = false;
        }
    }

    Preferences
    {
        section: "misc"
        property alias transformAttributeSortOrder: attributeList.ascendingSortOrder
        property alias transformAttributeSortBy: attributeList.sortRoleName
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        Text
        {
            Layout.fillWidth: true

            text: qsTr("Please select and name the attribute to clone:")
        }

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

            filters: AnyOf
            {
                ValueFilter
                {
                    roleName: "elementType"
                    value: qsTr("Node")
                }

                ValueFilter
                {
                    roleName: "elementType"
                    value: qsTr("Edge")
                }
            }

            onSelectedValueChanged:
            {
                if(selectedValue != undefined && !newAttributeName.manuallyChanged)
                    newAttributeName.text = qsTr("Clone of ") + AttributeUtils.prettify(selectedValue);
            }

            AttributeListSortMenu { attributeList: parent }
        }

        TextField
        {
            id: newAttributeName

            Layout.fillWidth: true

            placeholderText: qsTr("New Attribute Name")

            property bool manuallyChanged: false
            Keys.onPressed: { manuallyChanged = true; }

            onFocusChanged:
            {
                if(focus && !manuallyChanged)
                    selectAll();
            }
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: attributeList.selectedValues.length === 1 && newAttributeName.text.length > 0

                onClicked:
                {
                    document.cloneAttribute(attributeList.selectedValue, newAttributeName.text);
                    root.close();
                }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); }
            }
        }
    }
}
