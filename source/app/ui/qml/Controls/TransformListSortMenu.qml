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
import QtQuick.Controls 2.15

MouseArea
{
    id: root

    property var transformsList

    anchors.fill: parent
    propagateComposedEvents: true
    acceptedButtons: Qt.RightButton

    Menu
    {
        id: contextMenu

        Menu
        {
            id: sortRoleMenu
            title: qsTr("Sort By")
            ButtonGroup { id: sortByExclusiveGroup }

            Component
            {
                id: sortByComponent

                MenuItem
                {
                    checkable: true
                    ButtonGroup.group: sortByExclusiveGroup
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
                        return transformsList.sortBy === roleName;
                    });
                    menuItem.triggered.connect(function()
                    {
                        return transformsList.sortBy = roleName;
                    });

                    sortRoleMenu.addItem(menuItem);
                });
            }
        }

        Menu
        {
            id: sortAscendingMenu
            title: qsTr("Sort Order")
            ButtonGroup { id: sortOrderExclusiveGroup }

            Component
            {
                id: sortAscendingComponent

                MenuItem
                {
                    checkable: true
                    ButtonGroup.group: sortOrderExclusiveGroup
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

    onClicked: { contextMenu.popup(); }
}
