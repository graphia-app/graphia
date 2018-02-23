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

    readonly property bool hasTextFocus: findField.focus

    property string lastSearchedAttributeName

    readonly property bool showing: _visible

    readonly property int type: _type
    property int _type: Find.Simple

    property bool _visible: false

    property bool _finding: false
    property bool _pendingFind: false

    property int _options:
    {
        if(_type !== Find.Advanced)
            return FindOptions.MatchUsingRegex;

        var o = 0;

        if(matchCaseAction.checked)
            o |= FindOptions.MatchCase;

        if(matchWholeWordsAction.checked)
            o |= FindOptions.MatchWholeWords;

        if(matchUsingRegexAction.checked)
            o |= FindOptions.MatchUsingRegex;

        return o;
    }

    on_OptionsChanged: { _doFind(); }

    property var _findAttributes:
    {
        if(_type === Find.Advanced && attributeCheckBox.checked)
            return [attributeComboBox.currentText];

        return [];
    }

    on_FindAttributesChanged:
    {
        _doFind();
    }

    function _doFind()
    {
        if(findField.text.length === 0)
            return;

        if(!_finding)
        {
            _finding = true;
            document.find(findField.text, _options, _findAttributes);
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
        onTriggered: { document.selectAllFound(); }
    }

    Action
    {
        id: _previousAction
        text: qsTr("Find Previous")
        iconName: "go-previous"
        shortcut: _visible ? "Ctrl+Shift+G" : ""
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectPrevFound(); }
    }

    Action
    {
        id: _nextAction
        text: qsTr("Find Next")
        iconName: "go-next"
        shortcut: _visible ? "Ctrl+G" : ""
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectNextFound(); }
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
        id: closeAction
        text: qsTr("Close")
        iconName: "emblem-unreadable"
        shortcut: _visible && (findField.focus || !document.canResetView) ? "Esc" : ""
        onTriggered:
        {
            findField.focus = false;
            findField.text = "";
            _visible = false;

            document.resetFind();

            hidden();
        }
    }

    ValueFilter
    {
        id: searchableAttributesFilter
        roleName: "searchable"
        value: true
    }

    ValueFilter
    {
        id: stringAttributesFilter
        roleName: "valueType"
        value: "Textual"
    }

    SortFilterProxyModel
    {
        id: proxyModel
        filters:
        {
            if(_type === Find.Advanced)
                return [searchableAttributesFilter];
            else if(_type === Find.SelectByAttribute)
                return [stringAttributesFilter];

            return [];
        }

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

                        onTextChanged:
                        {
                            _doFind();
                        }

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
                    visible: _type === Find.SelectByAttribute

                    ComboBox
                    {
                        id: selectAttributeComboBox
                        implicitWidth: 150

                        textRole: "display"

                        model: proxyModel
                    }

                    ComboBox
                    {
                        id: valueComboBox
                        implicitWidth: 150
                    }
                }

                ToolBarSeparator {}

                ToolButton { action: _previousAction }
                ToolButton { action: _nextAction }
                ToolButton
                {
                    visible: _type === Find.Simple || _type === Find.Advanced
                    action: selectAllAction
                }
                ToolButton { action: closeAction }
            }

            RowLayout
            {
                visible: _type === Find.Advanced

                Rectangle { width: Constants.padding }

                CheckBox
                {
                    id: attributeCheckBox
                }

                ComboBox
                {
                    id: attributeComboBox
                    Layout.fillWidth: true

                    enabled: attributeCheckBox.checked
                    textRole: "display"

                    model: proxyModel

                    onEnabledChanged:
                    {
                        lastSearchedAttributeName = enabled ? currentText: "";
                    }

                    onCurrentTextChanged:
                    {
                        if(attributeCheckBox.checked)
                            lastSearchedAttributeName = currentText;
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
        if(findType === undefined)
            _type = Find.Simple;
        else
            _type = findType;

        proxyModel.sourceModel = document.availableAttributes(ElementType.Node);
        var rowIndex = proxyModel.rowIndexForAttributeName(lastSearchedAttributeName);

        if(_type === Find.Advanced)
        {
            if(rowIndex >= 0)
            {
                attributeComboBox.currentIndex = rowIndex;
                attributeCheckBox.checked = true;
            }
            else
            {
                attributeComboBox.currentIndex = 0;
                attributeCheckBox.checked = false;
            }
        }
        else if(_type === Find.SelectByAttribute)
        {
            if(rowIndex >= 0)
                selectAttributeComboBox.currentIndex = rowIndex;
            else
                selectAttributeComboBox.currentIndex = 0;
        }

        root._visible = true;

        if(_type === Find.Simple || _type === Find.Advanced)
        {
            findField.forceActiveFocus();
            findField.selectAll();
        }
        else
            document.resetFind();

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
        SelectByAttribute
    }
}
