/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQml.Models 2.2

import SortFilterProxyModel 0.2

Item
{
    id: root

    property var selectedValue: undefined
    property var selectedValues: []
    property var model: null

    property var currentIndex:
    {
        if(!model)
            return null;

        return sortFilterProxyModel.mapToSource(treeView.selection.currentIndex);
    }

    property bool currentIndexIsValid: currentIndex && currentIndex.valid

    property bool currentIndexIsSelectable:
    {
        if(!model || !currentIndexIsValid)
            return false;

        return model.flags(currentIndex) & Qt.ItemIsSelectable;
    }

    property alias sortRoleName: sortFilterProxyModel.sortRoleName
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property alias selectionMode: treeView.selectionMode
    property alias filters: sortFilterProxyModel.filters

    property bool showSections: false

    property bool showSearch: false
    property bool showParentGuide: false

    property var prettifyFunction: function(value) { return value; }

    //FIXME: 2 is fudge for frame/margins/something; need to account for it properly
    readonly property double contentHeight: treeView.__listView.contentHeight + 2
    readonly property double contentWidth: treeView.__listView.contentWidth + 2

    onModelChanged:
    {
        selectedValue = undefined;
        selectedValues = [];
        treeBoxSearch.visible = false;
    }

    function textFor(index)
    {
        // AvailableAttributesModel has a 'get' function that
        // returns more relevant results that QAbstractItemModel::data
        if(typeof root.model.get === 'function')
            return root.model.get(index);
        else if(typeof root.model.data === 'function')
            return root.model.data(index);

        return undefined;
    }

    // Just some semi-sensible defaults
    width: 200
    height: 100

    ColumnLayout
    {
        anchors.fill: root

        TreeBoxSearch
        {
            id: treeBoxSearch

            Layout.fillWidth: true

            visible: false

            onVisibleChanged:
            {
                searchButton.checked = visible;

                if(!visible)
                    treeView.forceActiveFocus();
            }

            treeBox: root
            onAccepted: { visible = false; }
        }

        TreeView
        {
            id: treeView

            Layout.fillWidth: true
            Layout.fillHeight: true

            model: SortFilterProxyModel
            {
                id: sortFilterProxyModel
                sourceModel: root.model !== undefined ? root.model : null
            }

            // Clear the selection when the model is changed
            selection: ItemSelectionModel { model: sortFilterProxyModel }
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
                function onSelectionChanged()
                {
                    if(!root.model)
                        return;

                    let sourceIndex = sortFilterProxyModel.mapToSource(target.currentIndex);
                    root.selectedValue = root.textFor(sourceIndex);

                    let newSelectedValues = [];
                    for(let index of target.selectedIndexes)
                    {
                        sourceIndex = sortFilterProxyModel.mapToSource(index);
                        newSelectedValues.push(root.textFor(sourceIndex));
                    }

                    root.selectedValues = newSelectedValues;
                }
            }

            onClicked: { root.clicked(index); }
            onDoubleClicked:
            {
                // FIXME: There seems to be a bug in TreeView(?) where if it is hidden in
                // the middle of a click, the mouse release event never gets delivered to
                // its MouseArea, and it gets into a state where the mouse button is
                // considered held down. This results in moving the mouse over the list
                // selecting the items, without any clicks. So instead we wait until the
                // mouse button is released, and then trigger the doubleClicked signal.

                doubleClickHack.index = index;
            }

            Connections
            {
                id: doubleClickHack
                property var index: null

                target: treeView.__mouseArea

                function onPressedChanged()
                {
                    if(!treeView.__mouseArea.pressed && index !== null)
                    {
                        root.doubleClicked(index);
                        index = null;

                        if(root.currentIndexIsSelectable)
                            root.accepted();
                    }
                }
            }

            Keys.onPressed:
            {
                if(root.showSearch)
                {
                    if((event.key === Qt.Key_F && event.modifiers & Qt.ControlModifier) ||
                        event.key === Qt.Key_Slash)
                    {
                        event.accepted = true;
                        root.toggleSearch();
                    }
                }

                switch(event.key)
                {
                case Qt.Key_Enter:
                case Qt.Key_Return:
                    if(root.currentIndexIsSelectable)
                    {
                        event.accepted = true;
                        root.accepted();
                    }
                    break;
                }
            }

            readonly property int _scrollBarWidth: __verticalScrollBar.visible ?
                __verticalScrollBar.width : 0

            FloatingButton
            {
                id: searchButton
                visible: root.showSearch && root.enabled

                anchors.rightMargin: treeView._scrollBarWidth + 4
                anchors.bottomMargin: 4
                anchors.right: parent !== undefined ? parent.right : undefined
                anchors.bottom: parent !== undefined ? parent.bottom : undefined

                iconName: "edit-find"
                hoverOpacity: 0.7
                checkable: true

                onClicked: { root.toggleSearch(); }
            }

            Rectangle
            {
                id: parentGuide
                visible: root.showParentGuide && root.enabled && opacity > 0.0

                property string text: ""
                property bool show: false

                readonly property int _internalMargin: 4
                readonly property int _margin: 6
                readonly property int _idealWidth: parentGuideText.contentWidth + (_internalMargin * 2)
                readonly property int _maxWidth: (treeView.width - treeView._scrollBarWidth) - (_margin * 2)

                implicitWidth: Math.min(_idealWidth, _maxWidth)
                implicitHeight: parentGuideText.implicitHeight + (_internalMargin * 2)

                anchors.margins: _margin
                anchors.top: parent !== undefined ? parent.top : undefined
                anchors.left: parent !== undefined ? parent.left : undefined

                color: "white"
                border.width: 1
                border.color: "grey"
                radius: 2

                opacity: show ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 250 } }

                Text
                {
                    id: parentGuideText
                    anchors.left: parent !== undefined ? parent.left : undefined
                    anchors.top: parent !== undefined ? parent.top : undefined
                    anchors.margins: parentGuide._internalMargin
                    text: root.prettifyFunction(parent.text)
                    elide: Text.ElideRight
                    font.italic: true
                }
            }

            function visibleIndices()
            {
                let indices = new Set();

                // This is clearly not a very efficient way of finding visible items,
                // but on the other hand it doesn't seem particularly slow either
                for(let y = 0; y < flickableItem.height; y++)
                {
                    let index = treeView.indexAt(flickableItem.width * 0.5, y);
                    let sourceIndex = sortFilterProxyModel.mapToSource(index);

                    if(!sourceIndex.valid)
                        continue;

                    indices.add(sourceIndex);
                }

                return indices;
            }

            function indexIsInSet(index, set)
            {
                for(let value of set)
                {
                    if(value.row === index.row && value.parent === index.parent)
                        return true;
                }

                return false;
            }

            // Do the guide update in the timer so that we don't
            // do so on every pixel when scrolling
            Timer
            {
                id: parentGuideTimer
                interval: 100
                repeat: false

                onTriggered:
                {
                    if(!root.showParentGuide)
                        return;

                    if(root.currentIndexIsValid && root.currentIndex.parent.valid)
                    {
                        let vi = treeView.visibleIndices();
                        if(treeView.indexIsInSet(root.currentIndex, vi))
                        {
                            let parentIndex = root.currentIndex.parent;
                            while(parentIndex.valid)
                            {
                                if(!treeView.indexIsInSet(parentIndex, vi))
                                {
                                    // At least one ancestor isn't visible
                                    parentGuide.text = root.textFor(root.currentIndex.parent);
                                    parentGuide.show = true;
                                    return;
                                }

                                parentIndex = parentIndex.parent;
                            }
                        }
                    }

                    parentGuide.show = false;
                }
            }

            Connections
            {
                target: treeView.flickableItem
                function onContentYChanged() { parentGuideTimer.restart(); }
            }
        }
    }

    onCurrentIndexChanged: { parentGuideTimer.restart(); }

    function toggleSearch()
    {
        if(!root.showSearch)
            return;

        treeBoxSearch.visible = !treeBoxSearch.visible;
    }

    function _mapIndexToRow(index)
    {
        // This hideous hack is the only obvious way to get from
        // a QModelIndex to a row index that corresponds to an item
        // in the TreeView
        for(let i = 0; i < treeView.__listView.count; i++)
        {
            if(treeView.__model.mapRowToModelIndex(i) === index)
                return i;
        }

        return -1;
    }

    function select(modelIndex)
    {
        if(!modelIndex.valid)
            return;

        // Do the selection
        let index = sortFilterProxyModel.mapFromSource(modelIndex);
        treeView.selection.setCurrentIndex(index, ItemSelectionModel.NoUpdate);
        treeView.selection.select(index, ItemSelectionModel.ClearAndSelect);

        // Ensure any containing tree branches are expanded
        let parentIndex = index.parent;
        while(parentIndex.valid)
        {
            if(!treeView.isExpanded(parentIndex))
                treeView.expand(parentIndex);

            parentIndex = parentIndex.parent;
        }

        // Scroll so that it's visible
        let row = _mapIndexToRow(index);
        treeView.__listView.currentIndex = row;
        treeView.__listView.positionViewAtIndex(row, ListView.Center);
    }

    function clearSelection()
    {
        treeView.selection.clearSelection();
    }

    Keys.forwardTo: [treeView.__mouseArea]

    signal accepted()

    signal clicked(var index)
    signal doubleClicked(var index)
}

