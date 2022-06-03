/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQml.Models
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SortFilterProxyModel

import app.graphia
import app.graphia.Shared
import app.graphia.Shared.Controls

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

        return treeView.selectedIndex;
    }

    property bool currentIndexIsValid: currentIndex && currentIndex.valid

    function _indexIsSelectable(index)
    {
        if(!model || !index || !index.valid)
            return false;

        return model.flags(index) & Qt.ItemIsSelectable;
    }

    property bool currentIndexIsSelectable:
    {
        return _indexIsSelectable(currentIndex);
    }

    property alias sortRoleName: sortFilterProxyModel.sortRoleName
    property alias sorters: sortFilterProxyModel.sorters
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property bool allowMultipleSelection: false
    property alias filters: sortFilterProxyModel.filters

    property var itemTextDelegateFunction: (model) => model.display

    property bool showSections: false
    property string sectionRoleName: ""

    property bool showSearch: false
    property bool showParentGuide: false

    property var prettifyFunction: (value) => value

    readonly property double contentHeight: treeView.contentHeight
    readonly property double contentWidth: treeView.contentWidth

    onModelChanged:
    {
        treeView.clearSelection();
        treeBoxSearch.visible = false;
        treeView.positionViewAtRow(0, Qt.AlignLeft|Qt.AlignTop, 0.0);
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

        Frame
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            topPadding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0

            Component.onCompleted:
            {
                if(background.color !== undefined)
                    background.color = "white";

                if(background.border !== undefined)
                    treeView.anchors.margins = background.border.width;
            }

            TreeView
            {
                id: treeView

                anchors.fill: parent
                clip: true

                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { policy: Qt.ScrollBarAsNeeded }
                interactive: false

                model: SortFilterProxyModel
                {
                    id: sortFilterProxyModel
                    sourceModel: root.model !== undefined ? root.model : null
                }

                readonly property int sectionRole:
                {
                    if(!root.model || !root.showSections)
                        return -1;

                    if(root.sectionRoleName.length > 0)
                        return QmlUtils.modelRoleForName(root.model, root.sectionRoleName);

                    if(root.sortRoleName.length > 0)
                        return QmlUtils.modelRoleForName(root.model, root.sortRoleName);

                    return -1;
                }

                function sectionTextFor(row)
                {
                    if(treeView.sectionRole < 0)
                        return "";

                    return model.data(modelIndex(row, 0), sectionRole);
                }

                property int lastSelectedRow: -1
                property var selectedRows: []

                onSelectedRowsChanged:
                {
                    if(!root.model)
                        return;

                    if(lastSelectedRow >= 0)
                    {
                        let sourceIndex = sortFilterProxyModel.mapToSource(modelIndex(lastSelectedRow, 0));

                        if(root._indexIsSelectable(sourceIndex))
                            root.selectedValue = root.textFor(sourceIndex);
                    }
                    else
                        root.selectedValue = undefined;

                    let newSelectedValues = [];
                    for(let row of selectedRows)
                    {
                        let sourceIndex = sortFilterProxyModel.mapToSource(modelIndex(row, 0));

                        if(!root._indexIsSelectable(sourceIndex))
                            continue;

                        newSelectedValues.push(root.textFor(sourceIndex));
                    }

                    root.selectedValues = newSelectedValues;
                }

                function clearSelection()
                {
                    lastSelectedRow = -1;
                    selectedRows = [];
                }

                property var selectedIndex:
                {
                    if(lastSelectedRow < 0)
                        return null;

                    return sortFilterProxyModel.mapToSource(modelIndex(lastSelectedRow, 0));
                }

                function setSelectedRows(newSelectedRows, row)
                {
                    newSelectedRows.sort();

                    lastSelectedRow = newSelectedRows.length > 0 &&
                        newSelectedRows.indexOf(row) < 0 ?
                        newSelectedRows[newSelectedRows.length - 1] : row;

                    selectedRows = newSelectedRows;
                }

                function selectRow(row)
                {
                    lastSelectedRow = row;
                    selectedRows = [row];
                }

                property int modifiers: 0

                Keys.onPressed: function(event)
                {
                    modifiers = event.modifiers;

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

                Keys.onReleased: function(event) { modifiers = event.modifiers; }
                onActiveFocusChanged: { if(!activeFocus) modifiers = 0; }

                delegate: Item
                {
                    implicitWidth: treeViewDelegate.implicitWidth
                    implicitHeight: treeViewDelegate.implicitHeight * (hasSectionRow ? 2 : 1)

                    property alias treeView: treeViewDelegate.treeView
                    property alias isTreeNode: treeViewDelegate.isTreeNode
                    property alias expanded: treeViewDelegate.expanded
                    property alias hasChildren: treeViewDelegate.hasChildren
                    property alias depth: treeViewDelegate.depth

                    property bool hasSectionRow:
                    {
                        if(!root.showSections)
                            return false;

                        if(model.row === 0)
                            return true;

                        let rowSection = treeView.sectionTextFor(model.row);
                        let rowMinusOneSection = treeView.sectionTextFor(model.row - 1);

                        if(!rowSection || !rowMinusOneSection)
                            return false;

                        return rowSection !== rowMinusOneSection;
                    }

                    Text
                    {
                        height: treeViewDelegate.implicitHeight
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 4

                        visible: hasSectionRow
                        font.bold: true
                        font.italic: true

                        text: { return hasSectionRow ? treeView.sectionTextFor(model.row) : ""; }
                    }

                    TreeViewDelegate
                    {
                        id: treeViewDelegate

                        implicitWidth: root.width
                        y: hasSectionRow ? implicitHeight : 0

                        property bool selected: treeView.selectedRows.indexOf(model.row) >= 0
                        property var highlightColor: selected ? palette.highlight : "transparent"

                        Component.onCompleted:
                        {
                            // Everything thing in here is speculative and relies on the actual internal
                            // structure of the delegate implementation, so it could break in future

                            let contrastBinding = Qt.binding(() =>
                                QmlUtils.contrastingColor(treeViewDelegate.highlightColor));

                            if(contentItem instanceof Text)
                            {
                                contentItem.color = contrastBinding;
                                contentItem.textFormat = Text.StyledText;
                            }

                            if(contentItem.text)
                                contentItem.text = Qt.binding(() => root.itemTextDelegateFunction(model));

                            if(indicator.color)
                                indicator.color = contrastBinding;

                            for(let i = 0; i < indicator.children.length; i++)
                                indicator.children[i].color = contrastBinding;
                        }

                        background: Rectangle { color: treeViewDelegate.highlightColor }

                        property var sourceIndex:
                        {
                            let sfpmIndex = treeView.modelIndex(model.row, model.column);
                            let index = sortFilterProxyModel.mapToSource(sfpmIndex);
                            return index;
                        }

                        onPressed:
                        {
                            let newSelectedRows = [];

                            if(!sourceIndex || !sourceIndex.valid)
                                return;

                            if(root.allowMultipleSelection && (treeView.modifiers & Qt.ShiftModifier) &&
                                treeView.lastSelectedRow !== -1)
                            {
                                let min = Math.min(model.row, treeView.lastSelectedRow);
                                let max = Math.max(model.row, treeView.lastSelectedRow);

                                newSelectedRows = treeView.selectedRows;

                                for(let i = min; i <= max; i++)
                                {
                                    if(newSelectedRows.indexOf(i) < 0)
                                        newSelectedRows.push(i);
                                }
                            }
                            else if(root.allowMultipleSelection && (treeView.modifiers & Qt.ControlModifier))
                            {
                                newSelectedRows = treeView.selectedRows;
                                let metaIndex = newSelectedRows.indexOf(model.row);

                                if(metaIndex < 0)
                                    newSelectedRows.push(model.row);
                                else
                                    newSelectedRows.splice(metaIndex, 1);
                            }
                            else
                                newSelectedRows = [model.row];

                            treeView.setSelectedRows(newSelectedRows, model.row);
                        }

                        onClicked:
                        {
                            root.clicked(sourceIndex);
                        }

                        onDoubleClicked:
                        {
                            root.doubleClicked(sourceIndex);

                            if(root.selectedValue)
                                root.accepted();
                        }
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onWheel: function(wheel) { treeView.flick(0, wheel.angleDelta.y * 5); }
                }

                onContentYChanged: { parentGuideTimer.restart(); }
            }

            FloatingButton
            {
                id: searchButton
                visible: root.showSearch && root.enabled

                anchors.rightMargin: 4
                anchors.bottomMargin: 4
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                iconName: "edit-find"
                hoverOpacity: 0.7
                checkable: true

                onClicked: function(mouse) { root.toggleSearch(); }
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
                readonly property int _maxWidth: treeView.width - (_margin * 2)

                implicitWidth: Math.min(_idealWidth, _maxWidth)
                implicitHeight: parentGuideText.implicitHeight + (_internalMargin * 2)

                anchors.margins: _margin
                anchors.top: parent.top
                anchors.left: parent.left

                color: "white"
                border.width: 1
                border.color: "grey"
                radius: 2

                opacity: show ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 250 } }

                Text
                {
                    id: parentGuideText

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: parentGuide._internalMargin

                    elide: Text.ElideRight
                    font.italic: true

                    text: root.prettifyFunction(parent.text)
                }
            }

            // Do the guide update in the timer so that we don't
            // do so on every pixel when scrolling
            Timer
            {
                id: parentGuideTimer
                interval: 100
                repeat: false

                function visibleIndices()
                {
                    let indices = new Set();

                    for(let row = treeView.topRow; row <= treeView.bottomRow; row++)
                    {
                        let index = treeView.modelIndex(row, 0);
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

                onTriggered: function(source)
                {
                    if(!root.showParentGuide)
                        return;

                    if(root.currentIndexIsValid && root.currentIndex.parent.valid)
                    {
                        let vi = visibleIndices();
                        if(indexIsInSet(root.currentIndex, vi))
                        {
                            let parentIndex = root.currentIndex.parent;
                            while(parentIndex.valid)
                            {
                                if(!indexIsInSet(parentIndex, vi))
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
        }
    }

    onCurrentIndexChanged: { parentGuideTimer.restart(); }

    function toggleSearch()
    {
        if(!root.showSearch)
            return;

        treeBoxSearch.visible = !treeBoxSearch.visible;
    }

    function select(modelIndex)
    {
        if(!modelIndex || !modelIndex.valid)
            return;

        if(!root._indexIsSelectable(modelIndex))
            return;


        let index = sortFilterProxyModel.mapFromSource(modelIndex);

        function expandAncestorsOf(childIndex)
        {
            let ancestors = [];

            let parentIndex = childIndex.parent;
            while(parentIndex.valid)
            {
                ancestors.unshift(parentIndex);
                parentIndex = parentIndex.parent;
            }

            for(let ancestor of ancestors)
            {
                let row = treeView.rowAtIndex(ancestor);

                if(!treeView.isExpanded(row))
                    treeView.expand(row);
            }
        }

        // Ensure any containing tree branches are expanded
        expandAncestorsOf(index);

        // Do the selection
        let row = treeView.rowAtIndex(index);
        treeView.selectRow(row);

        // Scroll so that it's visible
        treeView.positionViewAtRow(row, Qt.AlignLeft|Qt.AlignVCenter, 0.0);
    }

    function clearSelection()
    {
        treeView.clearSelection();
    }

    Keys.forwardTo: [treeView]

    signal accepted()

    signal clicked(var index)
    signal doubleClicked(var index)
}

