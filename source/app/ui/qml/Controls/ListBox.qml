import QtQuick 2.5
import QtQuick.Controls 1.4

Item
{
    property var selectedValue
    property var model

    onModelChanged: { selectedValue = undefined; }

    id: root

    // Just some semi-sensible defaults
    width: 200
    height: 100

    TableView
    {
        id: tableView

        anchors.fill: root
        model: root.model

        TableViewColumn { role: "display" }

        // Hide the header
        headerDelegate: Item {}

        alternatingRowColors: false

        Connections
        {
            target: tableView.selection

            onSelectionChanged:
            {
                if(target.count > 0)
                {
                    target.forEach(function(rowIndex)
                    {
                        if(typeof root.model.get === 'function')
                            root.selectedValue = root.model.get(rowIndex);
                        else
                            root.selectedValue = root.model[rowIndex];
                    });
                }
                else
                    root.selectedValue = undefined;
            }
        }
    }
}

