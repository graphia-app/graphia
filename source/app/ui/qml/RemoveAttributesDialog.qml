/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
import QtQuick.Controls 2.12
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

    title: qsTr("Remove Attributes")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 400
    minimumHeight: 250

    onVisibleChanged:
    {
        if(visible)
            attributeList.model = document.availableAttributesModel();
    }

    Preferences
    {
        section: "misc"
        property alias removeAttributesSortOrder: attributeList.ascendingSortOrder
        property alias removeAttributesSortBy: attributeList.sortRoleName
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        RowLayout
        {
            Layout.fillWidth: true
            spacing: Constants.spacing

            Text
            {
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                text: qsTr("Please select an attribute(s) to remove.")
            }

            NamedIcon
            {
                iconName: "edit-delete"

                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop
            }
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

            allowMultiSelection: true

            filters:
            [
                ValueFilter
                {
                    roleName: "userDefined"
                    value: qsTr("User Defined")
                },
                ValueFilter
                {
                    roleName: "hasParameter"
                    value: false
                }
            ]

            AttributeListSortMenu { attributeList: parent }
        }

        RowLayout
        {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: attributeList.selectedValues.length > 0

                onClicked:
                {
                    document.removeAttributes(attributeList.selectedValues);
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
