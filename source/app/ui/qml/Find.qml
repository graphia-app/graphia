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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SortFilterProxyModel

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

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
                return "^" + Utils.regexEscape(valueComboBox.value) + "$";
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

    function _updateImplicitSize()
    {
        implicitWidth = layout.implicitWidth + (Constants.padding * 2);
        implicitHeight = layout.implicitHeight + (Constants.padding * 2);
    }

    width: implicitWidth + (Constants.margin * 4)
    height: implicitHeight + (Constants.margin * 4)

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: !root._interrupted ? "white" : "#f0f0f0"

    Action
    {
        id: selectAllAction
        text: qsTr("Select All")
        icon.name: "edit-select-all"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectAllFound(); }
    }

    Action
    {
        id: _previousAction
        text: qsTr("Find Previous")
        icon.name: "go-previous"
        shortcut: _visible ? "Ctrl+Shift+G" : ""
        enabled: _type === Find.ByAttribute || document.numNodesFound > 0
        onTriggered: function(source)
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
        icon.name: "go-next"
        shortcut: _visible ? "Ctrl+G" : ""
        enabled: _type === Find.ByAttribute || document.numNodesFound > 0
        onTriggered: function(source)
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
        icon.name: "font-x-generic"
        checkable: true
    }

    Action
    {
        id: matchWholeWordsAction
        text: qsTr("Match Whole Words")
        icon.name: "text-x-generic"
        checkable: true
    }

    Action
    {
        id: matchUsingRegexAction
        text: qsTr("Match Using Regex")
        icon.name: "list-add"
        checkable: true
    }

    Action
    {
        id: selectMultipleModeAction
        text: qsTr("Select Multiple")
        icon.name: "edit-copy"
        checkable: true

        onCheckedChanged: function(checked)
        {
            if(checked && valueComboBox.value.length > 0)
                _attributeValues = [valueComboBox.value];
            else
                _attributeValues = [];

            _doFind();
        }
    }

    Action
    {
        id: selectOnlyAction
        text: qsTr("Select Only")
        icon.name: "edit-select-all"
        checkable: true

        onCheckedChanged: function(checked) { _doFind(); }
    }

    property bool _closing: false
    Action
    {
        id: closeAction
        text: qsTr("Close")
        icon.name: "emblem-unreadable"

        onTriggered: function(source)
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

        property var model: null
        sourceModel: model
        property bool _sourceChanging: false

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

    // Sink all wheel and right click events so that they
    // don't get passed on to underlying controls
    MouseArea
    {
        anchors.fill: layout
        onWheel: function(wheel) { /* NO-OP */ }
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) { mouse.accepted = true; }
    }

    ColumnLayout
    {
        id: layout

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Constants.padding

        onImplicitWidthChanged: { root._updateImplicitSize(); }
        onImplicitHeightChanged: { root._updateImplicitSize(); }

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
                    selectByMouse: true

                    onAccepted: { selectAllAction.trigger(); }

                    background: Rectangle
                    {
                        implicitWidth: 192
                        color: "transparent"
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
                                return Utils.format(qsTr("{0} of {1}"), index, document.numNodesFound);
                            else if(document.numNodesFound > 0)
                                return Utils.format(qsTr("{0} found"), document.numNodesFound);
                            else
                                return qsTr("Not Found");
                        }
                        color: "grey"
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: function(mouse) { findField.forceActiveFocus(); }
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
                        // Prevent spurious changes to currentText
                        if(proxyModel._sourceChanging)
                            return;

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

                    property bool _modelChanging: false
                    property string value: ""

                    onCurrentTextChanged:
                    {
                        // Prevent spurious changes to currentText
                        if(_modelChanging)
                            return;

                        value = currentText;
                    }

                    function refresh()
                    {
                        // Try to keep the same value selected
                        let preUpdateText = currentText;

                        let attribute = document.attribute(selectAttributeComboBox.currentText);

                        if(preferences.findByAttributeSortLexically)
                            attribute.sharedValues.sort(QmlUtils.localeCompareStrings);

                        _modelChanging = true;
                        model = attribute.sharedValues;
                        _modelChanging = false;

                        let rowIndex = find(preUpdateText);

                        if(rowIndex >= 0)
                            currentIndex = rowIndex;
                        else if(count > 0)
                            currentIndex = 0;
                        else
                            currentIndex = -1;

                        value = currentText;
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

        FramedScrollView
        {
            id: scrollView

            Layout.fillWidth: true
            Layout.preferredHeight: 128

            visible: _selectMultipleMode

            ListView
            {
                boundsBehavior: Flickable.StopAtBounds

                topMargin: 2
                leftMargin: 2

                model: valueComboBox.model
                delegate: Loader
                {
                    sourceComponent: CheckBox
                    {
                        text: modelData

                        bottomPadding: 2
                        rightPadding: scrollView.scrollBarWidth

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

                        ToolTip.visible: hovered
                        ToolTip.delay: Constants.toolTipDelay
                        ToolTip.text: modelData
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
                    // Prevent spurious changes to currentText
                    if(proxyModel._sourceChanging)
                        return;

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
        proxyModel._sourceChanging = true;
        proxyModel.model = document.availableAttributesModel(ElementType.Node);
        proxyModel._sourceChanging = false;

        if(_type === Find.Advanced)
            attributeComboBox.refresh();
        else if(_type === Find.ByAttribute)
            selectAttributeComboBox.refresh();
    }

    function show(findType)
    {
        if(_visible && findType === _type)
            return;

        _closing = false;

        if(findType === undefined)
            _type = Find.Simple;
        else
            _type = findType;

        root._updateImplicitSize();

        refresh();

        if(_type === Find.Simple || _type === Find.Advanced)
        {
            findField.forceActiveFocus();
            findField.selectAll();
        }

        root._visible = true;

        // Restore find state (if appropriate)
        _doFind();

        // Hack to delay emitting shown() until the implicit
        // size of the item has had a chance to be resolved
        Qt.callLater(() => Qt.callLater(() => shown()));
    }

    function hide()
    {
        closeAction.trigger();
        proxyModel.model = null;
    }

    signal shown();
    signal hidden();
}
