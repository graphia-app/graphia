import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import "../../../shared/ui/qml/Constants.js" as Constants

Rectangle
{
    id: root

    property var document

    readonly property bool showing: _visible
    property bool _visible: false

    width: row.width
    height: row.height

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: "white"

    Action
    {
        id: doneAction
        text: qsTr("Done")
        enabled: nameField.text.length > 0
        onTriggered:
        {
            document.addBookmark(nameField.text);
            closeAction.trigger();
        }
    }

    Action
    {
        id: closeAction
        text: qsTr("Cancel")
        iconName: "emblem-unreadable"
        shortcut: _visible ? "Esc" : ""

        onTriggered:
        {
            _visible = false;
            hidden();
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
            Layout.leftMargin: Constants.padding
            Layout.rightMargin: Constants.padding

            RowLayout
            {
                TextField
                {
                    id: nameField
                    width: 150

                    onAccepted: { doneAction.trigger(); }

                    style: TextFieldStyle
                    {
                        background: Rectangle
                        {
                            implicitWidth: 192
                            color: "transparent"
                        }
                    }

                    onFocusChanged:
                    {
                        if(!focus)
                            closeAction.trigger();
                    }
                }

                Button { action: doneAction }
                ToolButton { action: closeAction }
            }
        }
    }

    function show()
    {
        nameField.forceActiveFocus();
        nameField.selectAll();

        root._visible = true;
        shown();
    }

    function hide()
    {
        closeAction.trigger();
    }

    signal shown()
    signal hidden()
}
