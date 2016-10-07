import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.1

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
