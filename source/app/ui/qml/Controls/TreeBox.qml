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

        onCurrentIndexChanged:
        {
            if(!root.model)
                return;

            if(typeof root.model.get === 'function')
                root.selectedValue = root.model.get(currentIndex);
            else if(typeof root.model.data === 'function')
                root.selectedValue = root.model.data(currentIndex);
            else
                root.selectedValue = undefined;
        }
    }
}

