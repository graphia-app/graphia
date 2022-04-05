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

import QtQuick 2.7
import QtQuick.Controls 2.12

import "../../../../shared/ui/qml/Utils.js" as Utils

import app.graphia 1.0

Item
{
    id: root

    readonly property var selectedIndices: _selectedIndices

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

    property var model: null
    property string displayRole: "display"

    property var _selectedIndices: []
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

    SystemPalette { id: systemPalette }

    Rectangle
    {
        anchors.fill: parent
        border.width: 1
        border.color: systemPalette.mid
        clip: true

        ListView
        {
            anchors.margins: 1

            anchors.fill: parent
            model: root.model

            delegate: Rectangle
            {
                width: parent.width
                height: label.implicitHeight

                color:
                {
                    if(root._selectedIndices.indexOf(index) !== -1)
                        return systemPalette.highlight;

                    return "transparent";
                }

                Label
                {
                    id: label

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8

                    text:
                    {
                        if(model[root.displayRole])
                            return model[root.displayRole];

                        return modelData;
                    }

                    color: QmlUtils.contrastingColor(parent.color)
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }
            }

            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: Qt.ScrollBarAsNeeded }
            interactive: false

            MouseArea
            {
                anchors.fill: parent

                function setNewSelection(newSelection, index)
                {
                    newSelection.sort();

                    root._lastSelectedIndex = newSelection.length > 0 &&
                        newSelection.indexOf(index) < 0 ?
                        newSelection[newSelection.length - 1] : index;

                    root._selectedIndices = newSelection;
                }

                function indexAt(mouse)
                {
                    return parent.indexAt(mouse.x + parent.contentX, mouse.y + parent.contentY);
                }

                onWheel: { parent.flick(0, wheel.angleDelta.y * 5); }

                onPressed:
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

                        newSelectedIndices = root._selectedIndices;

                        for(let i = min; i <= max; i++)
                        {
                            if(newSelectedIndices.indexOf(i) < 0)
                                newSelectedIndices.push(i);
                        }
                    }
                    else if(root.allowMultipleSelection && (mouse.modifiers & Qt.ControlModifier))
                    {
                        newSelectedIndices = root._selectedIndices;
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

                onPositionChanged:
                {
                    let newSelectedIndices = [];

                    let index = indexAt(mouse);
                    if(index < 0)
                        return;

                    if(root.allowMultipleSelection)
                    {
                        newSelectedIndices = root._selectedIndices
                        let metaIndex = newSelectedIndices.indexOf(index);

                        if(metaIndex < 0)
                            newSelectedIndices.push(index);
                    }
                    else
                        newSelectedIndices = [index];

                    setNewSelection(newSelectedIndices, index);
                }

                onClicked:
                {
                    root.forceActiveFocus();
                    root.clicked(indexAt(mouse));
                }

                onDoubleClicked:
                {
                    root.forceActiveFocus();
                    root.doubleClicked(indexAt(mouse));

                    if(root.selectedValue)
                        root.accepted();
                }
            }
        }
    }

    Keys.onPressed:
    {
        let moveSelection = function(delta)
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

    function clear() { root._selectedIndices = []; }
    function selectAll()
    {
        if(!root.allowMultipleSelection)
        {
            console.log("Can't selectAll() when allowMultipleSelection is false.");
            return;
        }

        root._selectedIndices = [...Array(root.count).keys()];
    }

    signal accepted()

    signal clicked(var row)
    signal doubleClicked(var row)
}

