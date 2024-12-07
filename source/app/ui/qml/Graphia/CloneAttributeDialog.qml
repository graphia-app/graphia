/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

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
    flags: Constants.defaultWindowFlags
    color: palette.window

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
                color: palette.buttonText

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

            filterRoleName: "elementType"
            filterRegularExpression: { return new RegExp(qsTr("Node") + "|" + qsTr("Edge")); }

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

            selectByMouse: true
            placeholderText: qsTr("New Attribute Name")

            function setDefaultName(attributeName)
            {
                text = Utils.format(qsTr("Clone of {0}"), AttributeUtils.prettify(attributeName));
            }

            property bool manuallyChanged: false
            Keys.onPressed: function(event) { manuallyChanged = true; }

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

                onClicked: function(mouse)
                {
                    document.cloneAttribute(root.selectedAttributeName, newAttributeName.text);
                    root.close();
                }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }
}
