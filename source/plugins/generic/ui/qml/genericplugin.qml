import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import SortFilterProxyModel 0.2

PluginContent
{
    enabled: plugin.model.nodeAttributeTableModel.columnNames.length > 0

    anchors.fill: parent

    Action
    {
        id: toggleCalculatedAttributes
        text: qsTr("&Show Calculated Attributes")
        iconName: "computer"
        checkable: true
        checked: true
    }

    toolStrip: RowLayout
    {
        anchors.fill: parent
        ToolButton { action: toggleCalculatedAttributes }
        Item { Layout.fillWidth: true }
    }

    ColumnLayout
    {
        anchors.fill: parent

        NodeAttributeTableView
        {
            id: nodeAttributeTableView

            Layout.fillWidth: true
            Layout.fillHeight: true

            showCalculatedAttributes: toggleCalculatedAttributes.checked
            nodeAttributesModel: plugin.model.nodeAttributeTableModel
        }
    }

    function save()
    {
        var data =
        {
            "showCalculatedAttributes": toggleCalculatedAttributes.checked,
            "sortColumn": nodeAttributeTableView.sortIndicatorColumn,
            "sortOrder": nodeAttributeTableView.sortIndicatorOrder
        };

        return data;
    }

    function load(data, version)
    {
        toggleCalculatedAttributes.checked = data.showCalculatedAttributes;
        nodeAttributeTableView.sortIndicatorColumn = data.sortColumn;
        nodeAttributeTableView.sortIndicatorOrder = data.sortOrder;
    }
}
