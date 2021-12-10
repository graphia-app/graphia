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
import QtQuick.Controls 1.5
import QtQuick.Controls 2.12 as QQC2
import QtGraphicalEffects 1.0

Item
{
    id: root

    property var selectedValue: undefined

    property var model: null
    onModelChanged: { root.selectedValue = undefined; }

    property alias currentIndex: popupTreeBox.currentIndex
    property alias currentIndexIsValid: popupTreeBox.currentIndexIsValid

    property alias sortRoleName: popupTreeBox.sortRoleName
    property alias ascendingSortOrder: popupTreeBox.ascendingSortOrder

    property alias showSections: popupTreeBox.showSections

    property alias filters: popupTreeBox.filters

    property var previousIndex

    implicitWidth: 200
    implicitHeight: button.implicitHeight

    property string placeholderText: ""
    property var prettifyFunction: function(value) { return value; }

    function accept()
    {
        popup._valueAccepted = popupTreeBox.currentIndexIsSelectable;
        popup.close();
        popup._valueAccepted = false;

        root.selectedValue = popupTreeBox.selectedValue;
    }

    function reject()
    {
        popup._valueAccepted = false;
        popup.close();
    }

    // The ComboBox is only used here so that the UI looks
    // the part; its functionality is not used
    ComboBox
    {
        id: button

        model: [text]

        property string text:
        {
            if(!root.model || !root.currentIndexIsValid || !root.selectedValue)
                return root.prettifyFunction(root.placeholderText);

            return root.prettifyFunction(root.selectedValue);
        }

        anchors.fill: parent

        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                popup.open();
                parent.visible = false;

                if(root.currentIndexIsValid)
                {
                    treeboxSearch._select(root.currentIndex);
                    treeboxSearch.selectAll();
                }

                treeboxSearch.forceActiveFocus();
            }
        }
    }

    TreeBoxSearch
    {
        id: treeboxSearch

        anchors.fill: parent
        visible: !button.visible

        treeBox: popupTreeBox

        onActiveFocusChanged:
        {
            if(activeFocus)
                selectAll();
        }

        onAccepted: { root.accept(); }
    }

    QQC2.Popup
    {
        id: popup

        y: root.height
        implicitWidth: root.width
        implicitHeight: Math.min(popupTreeBox.contentHeight, 240)

        closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutsideParent

        padding: 0
        margins: 0

        TreeBox
        {
            id: popupTreeBox
            anchors.fill: parent
            model: root.model
            prettifyFunction: root.prettifyFunction

            onAccepted: { root.accept(); }
        }

        background: DropShadow
        {
            source: popup.contentItem

            radius: 12
            samples: (radius * 2) + 1
            color: "grey"
        }

        onOpened:
        {
            if(root.currentIndex === null)
                return;

            // Clone the current index
            root.previousIndex = root.model.index(root.currentIndex.row,
                root.currentIndex.column, root.currentIndex.parent);
        }

        property bool _valueAccepted: false
        onClosed:
        {
            button.visible = true;

            if(!_valueAccepted)
                root.select(root.previousIndex);
        }
    }

    function select(modelIndex)
    {
        popupTreeBox.select(modelIndex);
        root.selectedValue = popupTreeBox.selectedValue;
    }
}
