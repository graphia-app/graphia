import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQml.Models 2.2

Item
{
    property var selectedValue
    property var model

    onModelChanged: { selectedValue = undefined; }

    id: root

    // Just some semi-sensible defaults
    width: 200
    height: 100

    TreeView
    {
        id: treeView

        anchors.fill: root
        model: root.model !== undefined ? root.model : null

        // Clear the selection when the model is changed
        selection: ItemSelectionModel { model: treeView.model }
        onModelChanged: { selection.clear(); }

        TableViewColumn { role: "display" }

        // Hide the header
        headerDelegate: Item {}

        alternatingRowColors: false

        Connections
        {
            target: treeView.selection
            onSelectionChanged:
            {
                if(!root.model)
                    return;

                if(typeof root.model.get === 'function')
                    root.selectedValue = root.model.get(target.currentIndex);
                else if(typeof root.model.data === 'function')
                    root.selectedValue = root.model.data(target.currentIndex);
                else
                    root.selectedValue = undefined;
            }
        }
    }
}

