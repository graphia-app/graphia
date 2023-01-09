/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

import QtQuick
import QtQuick.Controls
import QtQml.Models

import app.graphia

TextField
{
    id: root

    property var treeBox: null

    selectByMouse: true
    placeholderText: qsTr("Search")

    onVisibleChanged:
    {
        // Not sure why/how this happens, but it does when
        // moving between transforms
        if(!root)
            return;

        text = "";

        if(visible)
            forceActiveFocus();
        else if(treeBox)
            treeBox.forceActiveFocus();
    }

    ModelCompleter
    {
        id: modelCompleter
        model: treeBox && treeBox.model ? treeBox.model : null
    }

    validator: RegularExpressionValidator
    {
        // Disallow leading whitespace
        regularExpression: /^[^\s]+.*/
    }

    property bool _allowComplete: false

    function _enableCompletion()
    {
        _allowComplete = true;
    }

    function _disableCompletion()
    {
        if(_inSetTextByIndex)
            return;

        _allowComplete = false;
        modelCompleter.startsWith = "";
    }

    Keys.onPressed: function(event)
    {
        switch(event.key)
        {
        case Qt.Key_Backspace:
        case Qt.Key_Delete:
            _disableCompletion();
            break;

        case Qt.Key_Escape:
        case Qt.Key_Back:
        case Qt.Key_Enter:
        case Qt.Key_Return:
            event.accepted = true;
            root.accepted();
            break;

        default:
            _enableCompletion();
            break;
        }
    }

    onSelectionStartChanged: { _disableCompletion(); }
    onSelectionEndChanged:   { _disableCompletion(); }
    onCursorPositionChanged: { _disableCompletion(); }

    function _autoComplete()
    {
        if(_allowComplete && text.length > 0)
        {
            modelCompleter.startsWith = text;

            if(modelCompleter.candidates.length > 0)
            {
                let candidate =
                    modelCompleter.candidates.indexOf(treeBox.currentIndex) !== -1 ?
                    treeBox.currentIndex : modelCompleter.closestMatch;

                root._select(candidate);
            }
        }
    }

    onTextChanged:
    {
        if(_inSetTextByIndex)
            return;

        _autoComplete();
    }

    // Prevent recursive calls to onTextChanged
    property bool _inSetTextByIndex: false

    function _setTextByIndex(index)
    {
        _inSetTextByIndex = true;

        let previousCursorPosition = cursorPosition;
        text = treeBox.model.data(index);

        if(modelCompleter.commonPrefix.length > 0)
            select(text.length, previousCursorPosition);

        _inSetTextByIndex = false;
    }

    function _select(index)
    {
        if(index !== treeBox.currentIndex)
            treeBox.select(index);
        else
            _setTextByIndex(index);
    }

    Connections
    {
        target: treeBox

        function onCurrentIndexChanged()
        {
            if(!treeBox.currentIndexIsValid ||
                modelCompleter.candidates.indexOf(treeBox.currentIndex) === -1)
            {
                _disableCompletion();
            }

            if(!treeBox.currentIndexIsValid)
                return;

            root._setTextByIndex(treeBox.currentIndex);
        }
    }

    function _moveSelection(delta)
    {
        if(modelCompleter.startsWith.length === 0)
        {
            _enableCompletion();
            _autoComplete();
        }

        if(modelCompleter.candidates.length < 2)
            return;

        let currentRowIndex =
            modelCompleter.candidates.indexOf(treeBox.currentIndex);

        if(currentRowIndex === -1)
            return;

        let newRowIndex = currentRowIndex + delta;

        while(newRowIndex < 0)
            newRowIndex += modelCompleter.candidates.length;

        while(newRowIndex >= modelCompleter.candidates.length)
            newRowIndex -= modelCompleter.candidates.length;

        let newIndex = modelCompleter.candidates[newRowIndex];

        _select(newIndex);
    }

    Keys.onUpPressed:   function(event) { _moveSelection(-1); }
    Keys.onDownPressed: function(event) { _moveSelection( 1); }
}
