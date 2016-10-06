import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.1

Item
{
    enabled: plugin.model.rowAttributes.columnNames.length > 0

    anchors.fill: parent

    ColumnLayout
    {
        anchors.fill: parent

        RowAttributeTableView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            rowAttributesModel: plugin.model.rowAttributes
        }
    }
}
