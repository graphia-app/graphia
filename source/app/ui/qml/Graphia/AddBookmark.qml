/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import Graphia.Controls
import Graphia.Utils

Rectangle
{
    id: root

    property var document

    readonly property bool showing: _visible
    property bool _visible: false

    implicitWidth: layout.implicitWidth + (Constants.padding * 2)
    implicitHeight: layout.implicitHeight + (Constants.padding * 2)
    height: layout.implicitHeight + (Constants.margin * 4)

    border.color: ControlColors.outline
    border.width: 1
    radius: 4
    color: ControlColors.background

    Action
    {
        id: doneAction
        text: qsTr("Done")
        enabled: nameField.text.length > 0
        onTriggered: function(source)
        {
            document.addBookmark(nameField.text);
            closeAction.trigger();
        }
    }

    Action
    {
        id: closeAction
        icon.name: "emblem-unreadable"

        onTriggered: function(source)
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

    ColumnLayout
    {
        id: layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Constants.padding

        RowLayout
        {
            Item
            {
                Layout.fillHeight: true
                Layout.preferredWidth: 192
                Layout.margins: Constants.margin
                clip: true

                TextInput
                {
                    id: nameField

                    anchors.fill: parent
                    anchors.verticalCenter: parent.verticalCenter

                    selectByMouse: true
                    color: palette.text
                    selectionColor: palette.highlight
                    selectedTextColor: palette.highlightedText

                    onAccepted: { doneAction.trigger(); }

                    onFocusChanged:
                    {
                        if(!focus)
                            closeAction.trigger();
                    }
                }
            }

            Button { action: doneAction }
            FloatingButton { action: closeAction }
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
