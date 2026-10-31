/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Graphia.Controls
import Graphia.Utils

Rectangle
{
    id: root

    readonly property list<int> selectedIndices: _selectedIndices

    readonly property int selectedIndex:
    {
        return root.selectedIndices.length > 0 ?
            root.selectedIndices.slice(-1)[0] : -1;
    }

    readonly property var selectedValues:
    {
        if(!root.model)
            return [];

        let values = [];

        for(const index of root._selectedIndices)
        {
            let value;
            if(typeof root.model.get === 'function')
                value = root.model.get(index);
            else if(typeof root.model.data === 'function')
                value = root.model.data(root.model.index(index, 0));
            else
                value = root.model[index];

            values.push(value);
        }

        return values;
    }

    readonly property var selectedValue:
    {
        return root.selectedValues.length > 0 ?
            root.selectedValues[root.selectedValues.length - 1] : "";
    }

    function modelRoleAt(index, roleName)
    {
        if(!root.model || index < 0 || roleName.length === 0)
            return "";

        if(typeof root.model.data !== 'function')
        {
            console.log("ListBox.selectedRole: model doesn't implement data(...)");
            return "";
        }

        return root.model.data(root.model.index(index, 0),
            NativeUtils.modelRoleForName(root.model, roleName));
    }

    property var model: null
    property string displayRole: "display"
    property var prettifyFunction: (value) => value

    property var highlightedProvider: function(index)
    {
        return root._selectedIndices.includes(index);
    }

    readonly property real scrollBarWidth: scrollBar.size < 1 ? scrollBar.width : 0

    property Component delegate: Label
    {
        id: listBoxDelegate

        required property var model
        required property var modelData
        required property int index

        anchors.left: parent ? parent.left : undefined
        anchors.right: parent ? parent.right : undefined
        anchors.leftMargin: -leftInset
        rightPadding: root.scrollBarWidth
        leftInset: -8
        bottomPadding: 1

        background: Rectangle
        {
            visible: root.highlightedProvider(listBoxDelegate.index)
            color: palette.highlight
        }

        text: root.prettifyFunction(model[root.displayRole] ?
            model[root.displayRole] : modelData)

        color: root.highlightedProvider(index) ?
            palette.highlightedText : palette.text
        elide: Text.ElideRight
    }

    property list<int> _selectedIndices: []
    property int _lastSelectedIndex: -1

    onModelChanged:
    {
        root._selectedIndices = [];
        root._lastSelectedIndex = -1;
    }

    readonly property int count:
    {
        if(!model)
            return 0;

        if(model.length !== undefined)
            return model.length;

        if(model.count !== undefined)
            return model.count;

        if(typeof model.rowCount === 'function')
            return model.rowCount();

        return 0;
    }

    property bool allowMultipleSelection: false

    // Just some semi-sensible defaults
    width: 200
    height: 100
    color: ControlColors.background

    ListView
    {
        id: listView

        anchors.fill: parent
        anchors.margins: outline.outlineWidth
        clip: true

        model: root.model

        delegate: root.delegate

        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: scrollBar; policy: Qt.ScrollBarAsNeeded }

        interactive: false

        MouseArea
        {
            anchors.fill: parent

            function setNewSelection(newSelection, index)
            {
                newSelection.sort((a, b) => a - b);

                root._lastSelectedIndex = newSelection.length > 0 &&
                    !newSelection.includes(index) ?
                    newSelection[newSelection.length - 1] : index;

                root._selectedIndices = newSelection;
            }

            function indexAt(mouse)
            {
                return parent.indexAt(mouse.x + parent.contentX, mouse.y + parent.contentY);
            }

            onWheel: (wheel) => { parent.flick(0, wheel.angleDelta.y * Constants.mouseWheelStep); }

            onPressed: (mouse) =>
            {
                root.forceActiveFocus();

                let newSelectedIndices = [];

                let index = indexAt(mouse);
                if(index < 0)
                    return;

                if(root.allowMultipleSelection && (mouse.modifiers & Qt.ShiftModifier) &&
                    root._lastSelectedIndex !== -1)
                {
                    let min = Math.min(index, root._lastSelectedIndex);
                    let max = Math.max(index, root._lastSelectedIndex);

                    newSelectedIndices = root._selectedIndices.slice();

                    for(let i = min; i <= max; i++)
                    {
                        if(!newSelectedIndices.includes(i))
                            newSelectedIndices.push(i);
                    }
                }
                else if(root.allowMultipleSelection && (mouse.modifiers & Qt.ControlModifier))
                {
                    newSelectedIndices = root._selectedIndices.slice();
                    let metaIndex = newSelectedIndices.indexOf(index);

                    if(metaIndex < 0)
                        newSelectedIndices.push(index);
                    else
                        newSelectedIndices.splice(metaIndex, 1);
                }
                else
                    newSelectedIndices = [index];

                setNewSelection(newSelectedIndices, index);
            }

            onPositionChanged: (mouse) =>
            {
                let newSelectedIndices = [];

                let index = indexAt(mouse);
                if(index < 0)
                    return;

                if(root.allowMultipleSelection)
                {
                    newSelectedIndices = root._selectedIndices.slice();
                    let metaIndex = newSelectedIndices.indexOf(index);

                    if(metaIndex < 0)
                        newSelectedIndices.push(index);
                }
                else
                    newSelectedIndices = [index];

                setNewSelection(newSelectedIndices, index);
            }

            onClicked: (mouse) =>
            {
                root.forceActiveFocus();
                root.clicked(indexAt(mouse));
            }

            onDoubleClicked: (mouse) =>
            {
                root.forceActiveFocus();
                root.doubleClicked(indexAt(mouse));

                if(root.selectedValue)
                    root.accepted();
            }
        }
    }

    Outline
    {
        id: outline
        anchors.fill: parent
    }

    Keys.onPressed: (event) =>
    {
        let moveSelection = (delta) =>
        {
            let newIndex = root._lastSelectedIndex;
            if(newIndex < 0 && root._selectedIndices.length > 0)
                newIndex = root._selectedIndices[root._selectedIndices.length - 1];

            newIndex = Utils.clamp(newIndex + delta, 0, root.count - 1);

            root._selectedIndices = [newIndex];
            root._lastSelectedIndex = newIndex;
        };

        switch(event.key)
        {
        case Qt.Key_Enter:
        case Qt.Key_Return:
            if(root.selectedValue)
            {
                event.accepted = true;
                root.accepted();
            }
            break;

        case Qt.Key_Up:     moveSelection(-1); break;
        case Qt.Key_Down:   moveSelection(1);  break;
        }
    }

    function clearSelection() { root._selectedIndices = []; }

    function select(index)
    {
        if(root._selectedIndices.length === 1 && root._selectedIndices[0] === index)
            return;

        root._selectedIndices = [index];
    }

    function selectAll()
    {
        if(!root.allowMultipleSelection)
        {
            console.log("ListBox: Can't selectAll() when allowMultipleSelection is false.");
            return;
        }

        root._selectedIndices = [...Array(root.count).keys()];
    }

    function itemAt(index) { return listView.itemAtIndex(index); }

    signal accepted()

    signal clicked(var index)
    signal doubleClicked(var index)
}

