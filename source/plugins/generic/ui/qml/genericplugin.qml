import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import SortFilterProxyModel 0.2

Item
{
    enabled: plugin.model.nodeAttributes.columnNames.length > 0

    anchors.fill: parent

    ColumnLayout
    {
        anchors.fill: parent

        NodeAttributeTableView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            nodeAttributesModel: plugin.model.nodeAttributes
        }
    }
}
