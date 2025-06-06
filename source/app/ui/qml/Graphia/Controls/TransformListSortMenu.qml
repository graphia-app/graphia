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
import QtQuick.Controls

import Graphia.Controls

MouseArea
{
    id: root

    property var transformsList

    anchors.fill: parent
    propagateComposedEvents: true
    acceptedButtons: Qt.RightButton

    PlatformMenu
    {
        id: contextMenu

        PlatformMenu
        {
            id: sortRoleMenu
            title: qsTr("Sort By")
            ButtonGroup { id: sortByButtonGroup }

            Component
            {
                id: sortByComponent

                PlatformMenuItem
                {
                    checkable: true
                    ButtonGroup.group: sortByButtonGroup
                }
            }

            Component.onCompleted:
            {
                let items =
                [
                    {"Name":        "display"},
                    {"Category":    "category"}
                ];

                items.forEach(function(item)
                {
                    let name = Object.keys(item)[0];
                    let roleName = item[name];

                    let menuItem = sortByComponent.createObject(root);
                    menuItem.text = qsTr(name);
                    menuItem.checked = Qt.binding(function()
                    {
                        return transformsList.sortRoleName === roleName;
                    });
                    menuItem.triggered.connect(function()
                    {
                        return transformsList.sortRoleName = roleName;
                    });

                    sortRoleMenu.addItem(menuItem);
                });
            }
        }

        PlatformMenu
        {
            id: sortAscendingMenu
            title: qsTr("Sort Order")
            ButtonGroup { id: sortOrderButtonGroup }

            Component
            {
                id: sortAscendingComponent

                PlatformMenuItem
                {
                    checkable: true
                    ButtonGroup.group: sortOrderButtonGroup
                }
            }

            Component.onCompleted:
            {
                let items =
                [
                    {"Ascending":   true},
                    {"Descending":  false}
                ];

                items.forEach(function(item)
                {
                    let name = Object.keys(item)[0];
                    let ascendingSortOrder = item[name];

                    let menuItem = sortAscendingComponent.createObject(root);
                    menuItem.text = qsTr(name);
                    menuItem.checked = Qt.binding(function()
                    {
                        return transformsList.ascendingSortOrder === ascendingSortOrder;
                    });
                    menuItem.triggered.connect(function()
                    {
                        transformsList.ascendingSortOrder = ascendingSortOrder;
                    });

                    sortAscendingMenu.addItem(menuItem);
                });
            }
        }
    }

    onClicked: function(mouse) { contextMenu.popup(); }
}
