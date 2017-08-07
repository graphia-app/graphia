import QtQuick 2.7
import QtQuick.Controls 1.5

MouseArea
{
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
            ExclusiveGroup { id: sortByExclusiveGroup }

            Component.onCompleted:
            {
                var items =
                [
                    {"Name":    "display"},
                    {"Type":    "type"}
                ];

                items.forEach(function(item)
                {
                    var name = Object.keys(item)[0];
                    var roleName = item[name];

                    var menuItem = sortRoleMenu.addItem(qsTr(name));
                    menuItem.checkable = true;
                    menuItem.exclusiveGroup = sortByExclusiveGroup;
                    menuItem.checked = Qt.binding(function()
                    {
                        return transformsList.sortRoleName === roleName;
                    });
                    menuItem.triggered.connect(function()
                    {
                        return transformsList.sortRoleName = roleName;
                    });
                });
            }
        }

        Menu
        {
            id: sortAscendingMenu
            title: qsTr("Sort Order")
            ExclusiveGroup { id: sortOrderExclusiveGroup }

            Component.onCompleted:
            {
                var items =
                [
                    {"Ascending":   true},
                    {"Descending":  false}
                ];

                items.forEach(function(item)
                {
                    var name = Object.keys(item)[0];
                    var ascendingSortOrder = item[name];

                    var menuItem = sortAscendingMenu.addItem(qsTr(name));
                    menuItem.checkable = true;
                    menuItem.exclusiveGroup = sortOrderExclusiveGroup;
                    menuItem.checked = Qt.binding(function()
                    {
                        return transformsList.ascendingSortOrder === ascendingSortOrder;
                    });
                    menuItem.triggered.connect(function()
                    {
                        transformsList.ascendingSortOrder = ascendingSortOrder;
                    });
                });
            }
        }
    }

    onClicked: { contextMenu.popup(); }
}
