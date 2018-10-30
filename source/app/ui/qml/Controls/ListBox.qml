import QtQuick 2.7
import QtQuick.Controls 1.5

Item
{
    id: root

    property var selectedValue
    property var selectedValues: []
    property var model

    onModelChanged: { selectedValue = undefined; }

    property bool allowMultipleSelection: false

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

        selectionMode: root.allowMultipleSelection ?
            SelectionMode.ExtendedSelection : SelectionMode.SingleSelection

        Connections
        {
            target: tableView.selection

            onSelectionChanged:
            {
                root.selectedValues = [];

                if(target.count > 0)
                {
                    target.forEach(function(rowIndex)
                    {
                        var value;
                        if(typeof root.model.get === 'function')
                            value = root.model.get(rowIndex);
                        else
                            value = root.model[rowIndex];

                        root.selectedValue = value;
                        root.selectedValues.push(value);
                    });
                }
                else
                    root.selectedValue = undefined;
            }
        }
    }
}

