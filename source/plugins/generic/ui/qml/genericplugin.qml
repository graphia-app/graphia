import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import SortFilterProxyModel 0.2

PluginContent
{
    id: root

    enabled: plugin.model.nodeAttributeTableModel.columnNames.length > 0

    anchors.fill: parent

    Action
    {
        id: resizeColumnsToContentsAction
        text: qsTr("&Resize Columns To Contents")
        iconName: "format-justify-fill"
        onTriggered: tableView.resizeColumnsToContentsBugWorkaround();
    }

    Action
    {
        id: toggleCalculatedAttributes
        text: qsTr("&Show Calculated Attributes")
        iconName: "computer"
        checkable: true
        checked: true

        onCheckedChanged: { root.saveRequired = true; }
    }

    function createMenu(index, menu)
    {
        switch(index)
        {
        case 0:
            tableView.populateTableMenu(menu);
            return true;
        }

        return false;
    }

    toolStrip: RowLayout
    {
        anchors.fill: parent
        ToolButton { action: resizeColumnsToContentsAction }
        ToolButton { action: toggleCalculatedAttributes }
        Item { Layout.fillWidth: true }
    }

    ColumnLayout
    {
        anchors.fill: parent

        NodeAttributeTableView
        {
            id: tableView

            Layout.fillWidth: true
            Layout.fillHeight: true

            showCalculatedAttributes: toggleCalculatedAttributes.checked
            nodeAttributesModel: plugin.model.nodeAttributeTableModel

            onSortIndicatorColumnChanged: { root.saveRequired = true; }
            onSortIndicatorOrderChanged: { root.saveRequired = true; }
        }
    }

    property bool saveRequired: false

    function save()
    {
        var data =
        {
            "showCalculatedAttributes": toggleCalculatedAttributes.checked,
            "sortColumn": tableView.sortIndicatorColumn,
            "sortOrder": tableView.sortIndicatorOrder
        };

        return data;
    }

    function load(data, version)
    {
        if(data.showCalculatedAttributes !== undefined) toggleCalculatedAttributes.checked = data.showCalculatedAttributes;
        if(data.sortColumn !== undefined)               tableView.sortIndicatorColumn = data.sortColumn;
        if(data.sortOrder !== undefined)                tableView.sortIndicatorOrder = data.sortOrder;
    }
}
