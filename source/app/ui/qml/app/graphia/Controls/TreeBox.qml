/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import app.graphia
import app.graphia.Controls
import app.graphia.Utils

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
    property alias sortExpression: sortFilterProxyModel.sortExpression
    property alias ascendingSortOrder: sortFilterProxyModel.ascendingSortOrder

    property alias filterRoleName: sortFilterProxyModel.filterRoleName
    property alias filterRegularExpression: sortFilterProxyModel.filterRegularExpression
    property alias filterExpression: sortFilterProxyModel.filterExpression

    function modelRole(name) { return sortFilterProxyModel.role(name); }

    property bool allowMultipleSelection: false

    property var itemTextDelegateFunction: (model) => model.display

    property bool showSections: false
    property string sectionRoleName: ""

    property bool showSearch: false
    property bool showParentGuide: false

    property var prettifyFunction: (value) => value

    readonly property double contentWidth: treeView.implicitWidth
    readonly property double contentHeight: treeView.implicitHeight

    onModelChanged:
    {
        treeView.clearSelection();
        treeBoxSearch.visible = false;
        treeView.positionViewAtRow(0, Qt.AlignLeft|Qt.AlignTop, 0.0);
    }

    function textFor(index)
    {
        // AvailableAttributesModel has a 'get' function that
        // returns more relevant results than QAbstractItemModel::data
        if(typeof root.model.get === 'function')
            return root.model.get(index);
        else if(typeof root.model.data === 'function')
            return root.model.data(index);

        return undefined;
    }

    implicitWidth: 240
    implicitHeight: 160

    ColumnLayout
    {
        anchors.fill: root

        Item
        {
            // This is a hack to avoid a bug on macOS where the layout
            // gets out of whack and ends up shorter than it should be
            id: treeBoxSearch

            Layout.fillWidth: true
            height: _treeBoxSearch.height

            visible: false

            onVisibleChanged:
            {
                searchButton.checked = visible;

                if(!visible)
                    treeView.forceActiveFocus();
            }

            TreeBoxSearch
            {
                id: _treeBoxSearch

                width: parent.width

                treeBox: root
                onAccepted: { visible = false; }
            }
        }

        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: ControlColors.background

            TreeView
            {
                id: treeView

                anchors.fill: parent
                anchors.margins: outline.outlineWidth
                clip: true

                alternatingRows: false

                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { id: scrollBar; policy: Qt.ScrollBarAsNeeded }
                readonly property real scrollBarWidth: scrollBar.size < 1 ? scrollBar.width : 0
                interactive: false

                keyNavigationEnabled: false
                pointerNavigationEnabled: false

                // This might be the way to go in future if ItemSelectionModel allows
                // for multiple selection, but for the time being (6.4.x) it doesn't
                // (And for 6.5.0 it doesn't allow for multiple selection via ctrl/shift click)
                //selectionBehavior: TableView.SelectRows
                //selectionModel: ItemSelectionModel {}

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

                    if(!sortFilterProxyModel.sourceModel)
                        return "";

                    let value = model.data(index(row, 0), treeView.sectionRole);

                    if(!value)
                        return "";

                    return value;
                }

                property int lastSelectedRow: -1
                property var selectedRows: []

                onSelectedRowsChanged:
                {
                    if(!root.model)
                        return;

                    if(lastSelectedRow >= 0)
                    {
                        let sourceIndex = sortFilterProxyModel.mapToSource(index(lastSelectedRow, 0));

                        if(root._indexIsSelectable(sourceIndex))
                            root.selectedValue = root.textFor(sourceIndex);
                    }
                    else
                        root.selectedValue = undefined;

                    for(let loadedRow = 0; loadedRow <= rows; loadedRow++)
                    {
                        if(!isRowLoaded(loadedRow))
                            continue;

                        let delegateItem = treeView.itemAtIndex(index(loadedRow, 0));
                        if(delegateItem !== null)
                            delegateItem.selected = false;
                    }

                    let newSelectedValues = [];
                    for(let row of selectedRows)
                    {
                        let sourceIndex = sortFilterProxyModel.mapToSource(index(row, 0));

                        if(!root._indexIsSelectable(sourceIndex))
                            continue;

                        newSelectedValues.push(root.textFor(sourceIndex));

                        let delegateItem = treeView.itemAtIndex(index(row, 0));
                        if(delegateItem !== null)
                            delegateItem.selected = true;
                    }

                    root.selectedValues = newSelectedValues;
                }

                function clearSelection()
                {
                    lastSelectedRow = -1;
                    selectedRows = [];
                }

                function selectionWithRangeAddedRow(row)
                {
                    if(treeView.lastSelectedRow < 0)
                        return [row];

                    let min = Math.min(row, treeView.lastSelectedRow);
                    let max = Math.max(row, treeView.lastSelectedRow);

                    let newSelectedRows = treeView.selectedRows;

                    for(let i = min; i <= max; i++)
                    {
                        if(newSelectedRows.indexOf(i) < 0)
                            newSelectedRows.push(i);
                    }

                    return newSelectedRows;
                }

                function selectionWithIndividualAddedRow(row)
                {
                    let newSelectedRows = treeView.selectedRows;
                    let metaIndex = newSelectedRows.indexOf(row);

                    if(metaIndex < 0)
                        newSelectedRows.push(row);
                    else
                        newSelectedRows.splice(metaIndex, 1);

                    return newSelectedRows;
                }

                property var selectedIndex:
                {
                    if(lastSelectedRow < 0)
                        return null;

                    return sortFilterProxyModel.mapToSource(index(lastSelectedRow, 0));
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

                function keyboardSelect(row)
                {
                    let newSelectedRows = [];

                    if(root.allowMultipleSelection && (treeView.modifiers & Qt.ShiftModifier))
                        newSelectedRows = treeView.selectionWithRangeAddedRow(row);
                    else
                        newSelectedRows = [row];

                    treeView.setSelectedRows(newSelectedRows, row);
                    treeView.positionViewAtRow(row, TableView.Visible);
                }

                Keys.onUpPressed:
                {
                    if(treeView.lastSelectedRow === -1)
                        return;

                    let newRow = (treeView.lastSelectedRow - 1);
                    if(newRow >= 0)
                        keyboardSelect(newRow);
                }

                Keys.onDownPressed:
                {
                    if(treeView.lastSelectedRow === -1)
                        return;

                    let newRow = (treeView.lastSelectedRow + 1);
                    if(newRow < treeView.rows)
                        keyboardSelect(newRow);
                }

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

                delegate: Rectangle
                {
                    implicitWidth: treeViewDelegate.implicitWidth
                    implicitHeight: treeViewDelegate.implicitHeight * (hasSectionRow ? 2 : 1)

                    required property bool current
                    required property int depth
                    required property bool editing
                    required property bool expanded
                    required property bool hasChildren
                    required property bool isTreeNode
                    required property bool selected
                    required property var treeView

                    TableView.onReused:
                    {
                        selected = treeView.selectedRows.indexOf(model.row) >= 0;
                    }

                    required property int row
                    required property var model

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
                        color: root.palette.text

                        text: { return hasSectionRow ? treeView.sectionTextFor(model.row) : ""; }
                    }

                    color: ControlColors.background

                    Rectangle
                    {
                        anchors.fill: treeViewDelegate
                        visible: selected
                        color: root.palette.highlight
                    }

                    TreeViewDelegate
                    {
                        id: treeViewDelegate

                        implicitWidth: root.width
                        rightPadding: treeView.scrollBarWidth
                        y: hasSectionRow ? implicitHeight : 0

                        current: parent.current
                        depth: parent.depth
                        editing: parent.editing
                        expanded: parent.expanded
                        hasChildren: parent.hasChildren
                        isTreeNode: parent.isTreeNode
                        selected: parent.selected
                        treeView: parent.treeView

                        model: parent.model
                        row: parent.row

                        Component.onCompleted:
                        {
                            // Everything in here is speculative and relies on the actual internal
                            // structure of the delegate implementation, so it could break in future

                            let contrastBinding = Qt.binding(() =>
                                selected ? root.palette.highlightedText : root.palette.text);

                            // On some styles, the background can't be changed, so we display the selection
                            // marker separately (see above), and hide the original background entirely
                            background.visible = false;
                            background.implicitHeight = 0;

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
                            {
                                if(indicator.children[i].color !== undefined)
                                    indicator.children[i].color = contrastBinding;
                            }

                            // Hack that uses the indicator to affect the overall height of the row
                            if(indicator.implicitHeight)
                                indicator.implicitHeight = contentItem.implicitHeight + 1;
                        }

                        property var sourceIndex:
                        {
                            let sfpmIndex = treeView.index(model.row, model.column);
                            let index = sortFilterProxyModel.mapToSource(sfpmIndex);
                            return index;
                        }

                        onPressed:
                        {
                            treeView.toggleExpanded(row);

                            let newSelectedRows = [];

                            if(!sourceIndex || !sourceIndex.valid)
                                return;

                            if(root.allowMultipleSelection && (treeView.modifiers & Qt.ShiftModifier))
                                newSelectedRows = treeView.selectionWithRangeAddedRow(model.row);
                            else if(root.allowMultipleSelection && (treeView.modifiers & Qt.ControlModifier))
                                newSelectedRows = treeView.selectionWithIndividualAddedRow(model.row);
                            else
                                newSelectedRows = [model.row];

                            treeView.setSelectedRows(newSelectedRows, model.row);
                        }

                        onReleased:
                        {
                            if(root._acceptancePending)
                            {
                                // This is just a workaround to avoid a (benign) Qt warning:
                                // qt.core.qobject.connect: QObject::disconnect: Unexpected nullptr parameter
                                root._acceptancePending = false;
                                root.accepted();
                            }
                        }

                        onClicked:
                        {
                            // Give the control keyboard focus so that shortcuts work
                            treeView.forceActiveFocus();

                            root.clicked(sourceIndex);
                        }

                        onDoubleClicked:
                        {
                            root.doubleClicked(sourceIndex);

                            if(root.selectedValue)
                                root._acceptancePending = true;
                        }
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onWheel: function(wheel) { treeView.flick(0, wheel.angleDelta.y * Constants.mouseWheelStep); }
                }

                onContentYChanged: { parentGuideTimer.restart(); }
            }

            Outline
            {
                id: outline
                anchors.fill: parent
            }

            FloatingButton
            {
                id: searchButton
                visible: root.showSearch && root.enabled

                anchors.rightMargin: 4 + treeView.scrollBarWidth
                anchors.bottomMargin: 4
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                icon.name: "edit-find"
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

                color: ControlColors.background
                border.width: 1
                border.color: ControlColors.outline
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
                    color: root.palette.text

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
                        let index = treeView.index(row, 0);
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

        if(!index.valid)
            return;

        treeView.expandToIndex(index);

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

    property bool _acceptancePending: false
    signal accepted()

    signal clicked(var index)
    signal doubleClicked(var index)
}

