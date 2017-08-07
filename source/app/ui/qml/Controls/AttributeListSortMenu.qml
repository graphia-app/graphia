import QtQuick 2.7
import QtQuick.Controls 1.5

MouseArea
{
    property var attributeList

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

            Component.onCompleted:
            {
                var items =
                [
                    {"Name":            "display"},
                    {"Element Type":    "elementType"},
                    {"Value Type":      "valueType"},
                    {"User Defined":    "userDefined"}
                ];

                items.forEach(function(item)
                {
                    var name = Object.keys(item)[0];
                    var roleName = item[name];

                    var menuItem = sortRoleMenu.addItem(qsTr(name));
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding(function()
                    {
                        return attributeList.sortRoleName === roleName;
                    });
                    menuItem.triggered.connect(function()
                    {
                        return attributeList.sortRoleName = roleName;
                    });
                });
            }
        }

        Menu
        {
            id: sortAscendingMenu
            title: qsTr("Sort Order")

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
                    menuItem.checked = Qt.binding(function()
                    {
                        return attributeList.ascendingSortOrder === ascendingSortOrder;
                    });
                    menuItem.triggered.connect(function()
                    {
                        attributeList.ascendingSortOrder = ascendingSortOrder;
                    });
                });
            }
        }
    }

    onClicked: { contextMenu.popup(); }
}
