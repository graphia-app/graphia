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

import QtQuick 2.12
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import SortFilterProxyModel 0.2

import app.graphia 1.0

import "Controls"
import "../../../shared/ui/qml/Constants.js" as Constants
import "../../../shared/ui/qml/Utils.js" as Utils

Rectangle
{
    enum FindType
    {
        Simple,
        Advanced,
        ByAttribute
    }

    id: root

    property var document

    property var previousAction: _previousAction
    property var nextAction: _nextAction

    property string lastAdvancedFindAttributeName
    property string lastFindByAttributeName

    readonly property bool showing: _visible

    readonly property int type: _type
    property int _type: Find.Simple

    property bool _visible: false

    property var _attributeValues: []
    property bool _selectMultipleMode:
        _type === Find.ByAttribute && selectMultipleModeAction.checked

    property bool _finding: false
    property bool _pendingFind: false

    // When there is an active search but the nodes mask is disabled, it means
    // the user has done some kind of selection outside of the found nodes
    property bool _interrupted: document.foundIndex < 0 &&
        document.numNodesFound > 0 && !document.nodesMaskActive

    property string _findText:
    {
        if(_type === Find.ByAttribute)
        {
            if(_selectMultipleMode)
            {
                let term = "";
                _attributeValues.forEach(function(value)
                {
                    if(term.length !== 0)
                        term += "|";

                    term += Utils.regexEscape(value);
                });

                return "^(" + term + ")$";
            }
            else
                return "^" + Utils.regexEscape(valueComboBox.currentText) + "$";
        }

        return findField.text;
    }

    on_FindTextChanged: { _doFind(); }

    property int _options:
    {
        let o = 0;

        switch(_type)
        {
        default:
        case Find.Simple:
            o = FindOptions.MatchUsingRegex;
            break;

        case Find.Advanced:
            if(matchCaseAction.checked)
                o |= FindOptions.MatchCase;

            if(matchWholeWordsAction.checked)
                o |= FindOptions.MatchWholeWords;

            if(matchUsingRegexAction.checked)
                o |= FindOptions.MatchUsingRegex;

            break;

        case Find.ByAttribute:
            o = FindOptions.MatchUsingRegex;
            break;
        }

        return o;
    }

    on_OptionsChanged: { _doFind(); }

    property var _findAttributes:
    {
        if(_type === Find.ByAttribute)
            return [selectAttributeComboBox.currentText];

        if(_type === Find.Advanced && attributeCheckBox.checked)
            return [attributeComboBox.currentText];

        return [];
    }

    property var _findSelectStyle:
    {
        if(_type === Find.ByAttribute)
            return FindSelectStyle.All;

        return FindSelectStyle.First;
    }

    on_FindAttributesChanged:
    {
        _doFind();
    }

    function _doFind()
    {
        if(!_visible)
        {
            document.resetFind();
            return;
        }

        if(!_finding)
        {
            if(_closing)
                document.resetFind();
            else if(_type === Find.ByAttribute && selectOnlyAction.checked)
            {
                document.resetFind();
                document.selectByAttributeValue(selectAttributeComboBox.currentText, _findText);
            }
            else if(_findText.length > 0)
                document.find(_findText, _options, _findAttributes, _findSelectStyle);
            else
                document.resetFind();

            _finding = true;
        }
        else
            _pendingFind = true;
    }

    Preferences
    {
        id: preferences
        section: "find"

        property alias matchCase: matchCaseAction.checked
        property alias matchWholeWords: matchWholeWordsAction.checked
        property alias matchUsingRegex: matchUsingRegexAction.checked
        property alias findByAttribtueSelectOnly: selectOnlyAction.checked
        property alias findByAttribtueSelectMultiple: selectMultipleModeAction.checked
        property bool findByAttributeSortLexically
    }

    width: row.width
    height: row.height

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: !root._interrupted ? "white" : "#f0f0f0"

    Action
    {
        id: selectAllAction
        text: qsTr("Select All")
        iconName: "edit-select-all"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectAllFound(); }
    }

    Action
    {
        id: _previousAction
        text: qsTr("Find Previous")
        iconName: "go-previous"
        shortcut: _visible ? "Ctrl+Shift+G" : ""
        enabled: _type === Find.ByAttribute || document.numNodesFound > 0
        onTriggered:
        {
            if(_type === Find.ByAttribute)
                valueComboBox.currentIndex = ((valueComboBox.currentIndex - 1) + valueComboBox.count) % valueComboBox.count;
            else
                document.selectPrevFound();
        }
    }

    Action
    {
        id: _nextAction
        text: qsTr("Find Next")
        iconName: "go-next"
        shortcut: _visible ? "Ctrl+G" : ""
        enabled: _type === Find.ByAttribute || document.numNodesFound > 0
        onTriggered:
        {
            if(_type === Find.ByAttribute)
                valueComboBox.currentIndex = (valueComboBox.currentIndex + 1) % valueComboBox.count;
            else
                document.selectNextFound();
        }
    }

    Action
    {
        id: matchCaseAction
        text: qsTr("Match Case")
        iconName: "font-x-generic"
        checkable: true
    }

    Action
    {
        id: matchWholeWordsAction
        text: qsTr("Match Whole Words")
        iconName: "text-x-generic"
        checkable: true
    }

    Action
    {
        id: matchUsingRegexAction
        text: qsTr("Match Using Regex")
        iconName: "list-add"
        checkable: true
    }

    Action
    {
        id: selectMultipleModeAction
        text: qsTr("Select Multiple")
        iconName: "edit-copy"
        checkable: true

        onCheckedChanged:
        {
            if(checked && valueComboBox.currentText.length > 0)
                _attributeValues = [valueComboBox.currentText];
            else
                _attributeValues = [];

            _doFind();
        }
    }

    Action
    {
        id: selectOnlyAction
        text: qsTr("Select Only")
        iconName: "edit-select-all"
        checkable: true

        onCheckedChanged: { _doFind(); }
    }

    property bool _closing: false
    Action
    {
        id: closeAction
        text: qsTr("Close")
        iconName: "emblem-unreadable"

        onTriggered:
        {
            findField.focus = false;
            findField.text = "";
            _closing = true;

            // Will reset find state
            _doFind();

            _visible = false;
            hidden();
        }
    }

    Shortcut
    {
        enabled: _visible
        sequence: "Esc"
        onActivated: { closeAction.trigger(); }
    }


    SortFilterProxyModel
    {
        id: proxyModel
        filters:
        [
            ValueFilter
            {
                enabled: _type === Find.Advanced || _type === Find.ByAttribute
                roleName: "searchable"
                value: true
            },
            ValueFilter
            {
                enabled: _type === Find.ByAttribute
                roleName: "hasSharedValues"
                value: true
            }
        ]

        function rowIndexForAttributeName(attributeName)
        {
            for(let i = 0; i < rowCount(); i++)
            {
                if(data(index(i, 0)) === attributeName)
                    return i;
            }
        }
    }

    // Sink all wheel events so that they don't get
    // passed on to underlying controls
    MouseArea
    {
        anchors.fill: row
        onWheel: { /* NO-OP */ }
    }

    RowLayout
    {
        id: row

        // The ColumnLayout in a RowLayout is just a hack to get some padding
        ColumnLayout
        {
            Layout.topMargin: Constants.padding - root.parent.parent.anchors.topMargin
            Layout.bottomMargin: Constants.padding
            Layout.leftMargin: Constants.padding - root.parent.parent.anchors.leftMargin
            Layout.rightMargin: Constants.padding

            RowLayout
            {
                RowLayout
                {
                    visible: _type === Find.Simple || _type === Find.Advanced

                    TextField
                    {
                        id: findField
                        width: 150
                        font.strikeout: root._interrupted

                        onAccepted: { selectAllAction.trigger(); }

                        style: TextFieldStyle
                        {
                            background: Rectangle
                            {
                                implicitWidth: 192
                                color: "transparent"
                            }
                        }

                        Keys.onUpPressed: { _previousAction.trigger(); }
                        Keys.onDownPressed: { _nextAction.trigger(); }
                    }

                    Item
                    {
                        Layout.fillHeight: true
                        implicitWidth: 80

                        Text
                        {
                            anchors.fill: parent

                            wrapMode: Text.NoWrap
                            elide: Text.ElideLeft
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter

                            visible: findField.length > 0
                            text:
                            {
                                let index = document.foundIndex + 1;

                                if(index > 0)
                                    return index + qsTr(" of ") + document.numNodesFound;
                                else if(document.numNodesFound > 0)
                                    return document.numNodesFound + qsTr(" found");
                                else
                                    return qsTr("Not Found");
                            }
                            color: "grey"
                        }

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked: { findField.forceActiveFocus(); }
                        }
                    }
                }

                RowLayout
                {
                    id: findByAttributeRow

                    visible: _type === Find.ByAttribute

                    ComboBox
                    {
                        id: selectAttributeComboBox
                        implicitWidth: 175

                        enabled: selectAttributeComboBox.count > 0

                        textRole: "display"

                        model: proxyModel

                        onCurrentTextChanged:
                        {
                            if(_visible && findByAttributeRow.visible)
                                lastFindByAttributeName = currentText;

                            valueComboBox.refresh();
                            _attributeValues = [];
                        }

                        function refresh()
                        {
                            let rowIndex = proxyModel.rowIndexForAttributeName(lastFindByAttributeName);

                            if(rowIndex >= 0)
                            {
                                currentIndex = rowIndex;
                                valueComboBox.refresh();
                            }
                            else if(count > 0)
                            {
                                currentIndex = 0;
                                lastFindByAttributeName = currentText;
                            }
                            else
                                currentIndex = -1;
                        }
                    }

                    FloatingButton { action: selectMultipleModeAction }

                    ComboBox
                    {
                        id: valueComboBox
                        Layout.preferredWidth: 175

                        visible: !_selectMultipleMode
                        enabled: valueComboBox.count > 0

                        function refresh()
                        {
                            // Try to keep the same value selected
                            let preUpdateText = currentText;

                            let attribute = document.attribute(selectAttributeComboBox.currentText);

                            if(preferences.findByAttributeSortLexically)
                                attribute.sharedValues.sort(QmlUtils.localeCompareStrings);

                            model = attribute.sharedValues;

                            let rowIndex = find(preUpdateText);

                            if(rowIndex >= 0)
                                currentIndex = rowIndex;
                            else if(count > 0)
                                currentIndex = 0;
                            else
                                currentIndex = -1;
                        }
                    }
                }

                FloatingButton { action: _previousAction; visible: !_selectMultipleMode }
                FloatingButton { action: _nextAction; visible: !_selectMultipleMode }
                FloatingButton
                {
                    visible: _type === Find.Simple || _type === Find.Advanced
                    action: selectAllAction
                }
                FloatingButton
                {
                    visible: _type === Find.ByAttribute
                    action: selectOnlyAction
                }
                FloatingButton { action: closeAction }
            }

            ScrollView
            {
                Layout.fillWidth: true
                Layout.preferredHeight: 128

                visible: _selectMultipleMode

                frameVisible: true
                clip: true

                ListView
                {
                    anchors.leftMargin: Constants.padding
                    anchors.fill: parent

                    boundsBehavior: Flickable.StopAtBounds

                    model: valueComboBox.model
                    delegate: Loader
                    {
                        sourceComponent: CheckBox
                        {
                            text: modelData

                            function isChecked()
                            {
                                return Utils.setContains(_attributeValues, modelData);
                            }

                            checked: { return isChecked(); }

                            onCheckedChanged:
                            {
                                // Unbind to prevent binding loop
                                checked = checked;

                                if(checked)
                                    _attributeValues = Utils.setAdd(_attributeValues, modelData);
                                else
                                    _attributeValues = Utils.setRemove(_attributeValues, modelData);

                                // Rebind so that the delegate doesn't hold the state
                                checked = Qt.binding(isChecked);
                            }

                            tooltip: modelData
                        }
                    }
                }
            }

            RowLayout
            {
                id: advancedRow

                visible: _type === Find.Advanced

                Rectangle { width: Constants.padding }

                CheckBox
                {
                    id: attributeCheckBox
                    enabled: attributeComboBox.count > 0
                }

                ComboBox
                {
                    id: attributeComboBox
                    Layout.fillWidth: true

                    enabled: attributeCheckBox.checked && attributeComboBox.count > 0
                    textRole: "display"

                    model: proxyModel

                    onEnabledChanged:
                    {
                        if(_visible && advancedRow.visible)
                            lastAdvancedFindAttributeName = enabled ? currentText: "";
                    }

                    onCurrentTextChanged:
                    {
                        if(_visible && advancedRow.visible && enabled)
                            lastAdvancedFindAttributeName = currentText;
                    }

                    function refresh()
                    {
                        let rowIndex = proxyModel.rowIndexForAttributeName(lastAdvancedFindAttributeName);

                        if(rowIndex >= 0)
                        {
                            currentIndex = rowIndex;
                            attributeCheckBox.checked = true;
                        }
                        else if(count > 0)
                        {
                            currentIndex = 0;
                            attributeCheckBox.checked = false;
                        }
                        else
                        {
                            currentIndex = -1;
                            attributeCheckBox.checked = false;
                        }
                    }
                }

                FloatingButton { action: matchCaseAction }
                FloatingButton { action: matchWholeWordsAction }
                FloatingButton { action: matchUsingRegexAction }
            }
        }
    }

    Connections
    {
        target: document

        function onCommandsFinished()
        {
            _finding = false;

            if(_pendingFind)
            {
                _doFind();
                _pendingFind = false;
            }
        }

        function onGraphChanged(graph, changeOccurred)
        {
            refresh();
        }
    }

    function refresh()
    {
        proxyModel.sourceModel = document.availableAttributesModel(ElementType.Node);

        if(_type === Find.Advanced)
            attributeComboBox.refresh();
        else if(_type === Find.ByAttribute)
            selectAttributeComboBox.refresh();
    }

    function show(findType)
    {
        _closing = false;

        if(findType === undefined)
            _type = Find.Simple;
        else
            _type = findType;

        refresh();

        if(_type === Find.Simple || _type === Find.Advanced)
        {
            findField.forceActiveFocus();
            findField.selectAll();
        }

        root._visible = true;

        // Restore find state (if appropriate)
        _doFind();

        shown();
    }

    function hide()
    {
        closeAction.trigger();
    }

    signal shown();
    signal hidden();
}
