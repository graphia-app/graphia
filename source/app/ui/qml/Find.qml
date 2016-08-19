import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Item
{
    id: root

    property var document

    width: row.width
    height: row.height

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
        id: selectPreviousAction
        text: qsTr("Previous")
        iconName: "go-previous"
        shortcut: "Ctrl+Shift+G"
        enabled: document.numNodesFound > 0
        onTriggered: { document.selectPrevFound(); }
    }

    Action
    {
        id: selectNextAction
        text: qsTr("Next")
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
        shortcut: visible ? "Esc" : ""
        onTriggered:
        {
            findField.text = "";
            visible = false;
        }
    }

    RowLayout
    {
        id: row

        TextField
        {
            id: findField
            width: 150

            onTextChanged: document.find(text);
            onAccepted: { selectAllAction.trigger(); }
        }

        ToolButton { action: selectPreviousAction }
        ToolButton { action: selectNextAction }
        ToolButton { action: selectAllAction }
        ToolButton { action: closeAction }

        Text
        {
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
            color: document.textColor
        }
    }

    onVisibleChanged:
    {
        findField.forceActiveFocus();
    }

    function show()
    {
        root.visible = true;
        findField.focus = true;
        findField.selectAll();
    }
}
