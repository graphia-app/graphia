import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import SortFilterProxyModel 0.2

import com.kajeka 1.0

import "Controls"
import "../../../shared/ui/qml/Constants.js" as Constants

Rectangle
{
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

    property bool _finding: false
    property bool _pendingFind: false

    property string _findText:
    {
        if(_type === Find.ByAttribute)
            return valueComboBox.currentText;

        return findField.text;
    }

    on_FindTextChanged: { _doFind(); }

    property int _options:
    {
        var o = 0;

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
            o = FindOptions.MatchCase|FindOptions.MatchWholeWords;
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
            return;

        if(!_finding)
        {
            if(_closing)
                document.resetFind();
            else if(_type === Find.ByAttribute && selectOnlyAction.checked)
            {
                document.resetFind();
                document.selectByAttributeValue(selectAttributeComboBox.currentText, valueComboBox.currentText);
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
    }

    width: row.width
    height: row.height

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: "white"

    Action
    {
        id: selectAllAction
        text: qsTr("Select All")
        iconName: "weather-clear"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectAll(); }
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
        shortcut: _visible ? "Esc" : ""

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


    SortFilterProxyModel
    {
        id: proxyModel
        filters:
        [
            ValueFilter
            {
                enabled: _type === Find.Advanced
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
            for(var i = 0; i < rowCount(); i++)
            {
                if(data(index(i, 0)) === attributeName)
                    return i;
            }
        }
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

                        onAccepted: { selectAllAction.trigger(); }

                        style: TextFieldStyle
                        {
                            background: Rectangle
                            {
                                implicitWidth: 192
                                color: "transparent"
                            }
                        }
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
                                var index = document.foundIndex + 1;

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
                        }
                    }

                    ComboBox
                    {
                        id: valueComboBox
                        implicitWidth: 175

                        enabled: valueComboBox.count > 0

                        model:
                        {
                            var attribute = document.attribute(selectAttributeComboBox.currentText);
                            return attribute.sharedValues;
                        }
                    }
                }

                ToolButton { action: _previousAction }
                ToolButton { action: _nextAction }
                ToolButton
                {
                    visible: _type === Find.Simple || _type === Find.Advanced
                    action: selectAllAction
                }
                ToolButton
                {
                    visible: _type === Find.ByAttribute
                    action: selectOnlyAction
                }
                ToolButton { action: closeAction }
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
                }

                ToolButton { action: matchCaseAction }
                ToolButton { action: matchWholeWordsAction }
                ToolButton { action: matchUsingRegexAction }
            }

            Connections
            {
                target: document

                onCommandCompleted:
                {
                    _finding = false;

                    if(_pendingFind)
                    {
                        _doFind();
                        _pendingFind = false;
                    }
                }
            }
        }
    }

    function show(findType)
    {
        _closing = false;

        if(findType === undefined)
            _type = Find.Simple;
        else
            _type = findType;

        proxyModel.sourceModel = document.availableAttributes(ElementType.Node);

        if(_type === Find.Advanced)
        {
            var rowIndex = proxyModel.rowIndexForAttributeName(lastAdvancedFindAttributeName);

            if(rowIndex >= 0)
            {
                attributeComboBox.currentIndex = rowIndex;
                attributeCheckBox.checked = true;
            }
            else if(attributeComboBox.count > 0)
            {
                attributeComboBox.currentIndex = 0;
                attributeCheckBox.checked = false;
            }
            else
            {
                attributeComboBox.currentIndex = -1;
                attributeCheckBox.checked = false;
            }
        }
        else if(_type === Find.ByAttribute)
        {
            var rowIndex = proxyModel.rowIndexForAttributeName(lastFindByAttributeName);

            if(rowIndex >= 0)
                selectAttributeComboBox.currentIndex = rowIndex;
            else if(selectAttributeComboBox.count > 0)
                selectAttributeComboBox.currentIndex = 0;
            else
                selectAttributeComboBox.currentIndex = -1;
        }

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

    // At the bottom of the file to avoid (completely) screwing up QtCreator's (4.5.1) syntax highlighting
    enum FindType
    {
        Simple,
        Advanced,
        ByAttribute
    }
}
