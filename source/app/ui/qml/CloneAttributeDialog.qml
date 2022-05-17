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
import app.graphia.Controls 1.0
import app.graphia.Shared 1.0

import SortFilterProxyModel 0.2

import "AttributeUtils.js" as AttributeUtils

Window
{
    id: root

    property var document: null
    property string sourceAttributeName: ""

    property string selectedAttributeName:
    {
        if(sourceAttributeName.length === 0)
            return attributeList.selectedValue ? attributeList.selectedValue : "";

        return sourceAttributeName;
    }

    title: qsTr("Clone Attribute")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    readonly property int _preselectedAttributeDialogHeight: (mainLayout.implicitHeight + (2 * Constants.margin))

    minimumWidth: 400
    minimumHeight: sourceAttributeName.length === 0 ? 250 : _preselectedAttributeDialogHeight
    maximumHeight: sourceAttributeName.length === 0 ? (1 << 24) - 1 : minimumHeight

    onVisibleChanged:
    {
        if(visible)
        {
            newAttributeName.manuallyChanged = false;

            if(root.sourceAttributeName.length === 0)
            {
                attributeList.model = document.availableAttributesModel();
                newAttributeName.text = "";
            }
            else
            {
                newAttributeName.setDefaultName(root.sourceAttributeName);
                newAttributeName.forceActiveFocus();
            }
        }
        else
            attributeList.model = null;
    }

    Preferences
    {
        section: "misc"
        property alias transformAttributeSortOrder: attributeList.ascendingSortOrder
        property alias transformAttributeSortBy: attributeList.sortRoleName
    }

    ColumnLayout
    {
        id: mainLayout

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

                text: root.sourceAttributeName.length === 0 ?
                    qsTr("Please select an attribute to clone, then enter a name for the new attribute.") :
                    qsTr("Please enter a name for the new attribute.")
            }

            NamedIcon
            {
                iconName: "edit-copy"

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

            visible: root.sourceAttributeName.length === 0

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
                    newAttributeName.setDefaultName(selectedValue);
            }

            AttributeListSortMenu { attributeList: parent }
        }

        TextField
        {
            id: newAttributeName

            Layout.fillWidth: true

            placeholderText: qsTr("New Attribute Name")

            function setDefaultName(attributeName)
            {
                text = qsTr("Clone of ") + AttributeUtils.prettify(attributeName);
            }

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
                enabled: root.selectedAttributeName.length > 0 && newAttributeName.text.length > 0

                onClicked:
                {
                    document.cloneAttribute(root.selectedAttributeName, newAttributeName.text);
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
