import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQml.Models 2.2

import SortFilterProxyModel 0.2

Item
{
    property var selectedValue
    property var model

    property alias sortRoleName: sortFilterProxyModel.sortRoleName
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property bool showSections: false

    onModelChanged: { selectedValue = undefined; }

    id: root

    // Just some semi-sensible defaults
    width: 200
    height: 100

    TreeView
    {
        id: treeView

        anchors.fill: root
        model: SortFilterProxyModel
        {
            id: sortFilterProxyModel
            sourceModel: root.model !== undefined ? root.model : null
        }

        // Clear the selection when the model is changed
        selection: ItemSelectionModel { model: treeView.model }
        onModelChanged: { selection.clear(); }

        TableViewColumn { role: "display" }

        // Hide the header
        headerDelegate: Item {}

        alternatingRowColors: false

        section.property: showSections && root.sortRoleName.length > 0 ? root.sortRoleName : ""
        section.delegate: Component
        {
            Text
            {
                // "Hide" when the section text is empty
                height: text.length > 0 ? implicitHeight : 0
                text: section
                font.italic: true
                font.bold: true
            }
        }

        Connections
        {
            target: treeView.selection
            onSelectionChanged:
            {
                if(!root.model)
                    return;

                var sourceIndex = treeView.model.mapToSource(target.currentIndex);

                if(typeof root.model.get === 'function')
                    root.selectedValue = root.model.get(sourceIndex);
                else if(typeof root.model.data === 'function')
                    root.selectedValue = root.model.data(sourceIndex);
                else
                    root.selectedValue = undefined;
            }
        }
    }
}

