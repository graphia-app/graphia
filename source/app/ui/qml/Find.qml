import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import "Controls"
import "Constants.js" as Constants

Rectangle
{
    id: root

    property var document

    property var selectPreviousAction: _selectPreviousAction
    property var selectNextAction: _selectNextAction

    property bool _visible: false

    property bool _finding
    property string _pendingFindText

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
        shortcut: "Ctrl+Shift+G"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectPrevFound(); }
    }

    Action
    {
        id: _selectNextAction
        text: qsTr("Find Next")
        iconName: "go-next"
        shortcut: "Ctrl+G"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectNextFound(); }
    }

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
            _visible = false;

            hidden();
        }
    }

    RowLayout
    {
        id: row

        // The RowLayout in a RowLayout is just a hack to get some padding
        RowLayout
        {
            Layout.topMargin: Constants.padding + Constants.margin - 2
            Layout.bottomMargin: Constants.padding
            Layout.leftMargin: Constants.padding + Constants.margin - 2
            Layout.rightMargin: Constants.padding

            TextField
            {
                id: findField
                width: 150

                onTextChanged:
                {
                    if(!_finding)
                    {
                        _finding = true;
                        document.find(text);
                    }
                    else
                        _pendingFindText = text;
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

            Connections
            {
                target: document

                onCommandComplete:
                {
                    _finding = false;

                    if(_pendingFindText.length > 0)
                    {
                        document.find(_pendingFindText);
                        _pendingFindText = "";
                    }
                }
            }
        }
    }

    function show()
    {
        root._visible = true;
        findField.forceActiveFocus();
        findField.selectAll();

        shown();
    }

    signal shown();
    signal hidden();
}
