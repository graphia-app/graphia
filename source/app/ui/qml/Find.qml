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

    property var selectPreviousAction: _selectPreviousAction
    property var selectNextAction: _selectNextAction

    property bool hasTextFocus: findField.focus

    property string lastSearchedAttributeName

    property bool _visible: false

    property bool _finding: false
    property bool _pendingFind: false

    property int _options:
    {
        if(!advancedRow.visible)
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

    property var _attributes:
    {
        if(advancedRow.visible && attributeCheckBox.checked)
            return [attributeComboBox.currentText];

        return [];
    }

    on_AttributesChanged: { _doFind(); }

    function _doFind()
    {
        if(findField.text.length === 0)
            return;

        if(!_finding)
        {
            _finding = true;
            document.find(findField.text, _options, _attributes);
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
        id: _selectPreviousAction
        text: qsTr("Find Previous")
        iconName: "go-previous"
        shortcut: _visible ? "Ctrl+Shift+G" : ""
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectPrevFound(); }
    }

    Action
    {
        id: _selectNextAction
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

            // Reset find
            document.find();

            hidden();
        }
    }

    RowLayout
    {
        id: row

        // The ColumnLayout in a RowLayout is just a hack to get some padding
        ColumnLayout
        {
            Layout.topMargin: Constants.padding + Constants.margin - 2
            Layout.bottomMargin: Constants.padding
            Layout.leftMargin: Constants.padding + Constants.margin - 2
            Layout.rightMargin: Constants.padding

            RowLayout
            {
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

                ToolBarSeparator {}

                ToolButton { action: _selectPreviousAction }
                ToolButton { action: _selectNextAction }
                ToolButton { action: selectAllAction }
                ToolButton { action: closeAction }
            }

            RowLayout
            {
                id: advancedRow

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

                    // Restrict the attribute list to only those which are searchable
                    model: SortFilterProxyModel
                    {
                        id: proxyModel
                        filters:
                        [
                            ValueFilter
                            {
                                roleName: "searchable"
                                value: true
                            }
                        ]
                    }

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

                onCommandComplete:
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

    function show(includeAdvancedOptions)
    {
        proxyModel.sourceModel = document.availableAttributes(ElementType.Node);
        var model = proxyModel;

        for(var i = 0; i < model.rowCount(); i++)
        {
            var attributeName = model.data(model.index(i, 0));

            if(attributeName === lastSearchedAttributeName)
            {
                attributeComboBox.currentIndex = i;
                attributeCheckBox.checked = true;
                break;
            }
        }

        if(i === model.rowCount())
        {
            attributeComboBox.currentIndex = 0;
            attributeCheckBox.checked = false;
        }

        advancedRow.visible = includeAdvancedOptions;

        root._visible = true;
        findField.forceActiveFocus();
        findField.selectAll();

        shown();
    }

    signal shown();
    signal hidden();
}
