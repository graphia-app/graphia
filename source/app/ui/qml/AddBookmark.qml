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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import app.graphia.Controls 1.0
import app.graphia.Shared 1.0

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
        icon.name: "emblem-unreadable"

        onTriggered:
        {
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

                    background: Rectangle
                    {
                        implicitWidth: 192
                        color: "transparent"
                    }

                    onFocusChanged:
                    {
                        if(!focus)
                            closeAction.trigger();
                    }
                }

                Button { action: doneAction }
                FloatingButton { action: closeAction }
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
