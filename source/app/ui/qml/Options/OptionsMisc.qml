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

import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import app.graphia 1.0

import "../Controls"
import "../../../../shared/ui/qml/Constants.js" as Constants

Item
{
    Preferences
    {
        id: misc
        section: "misc"

        property alias focusFoundNodes: focusFoundNodesCheckbox.checked
        property alias focusFoundComponents: focusFoundComponentsCheckbox.checked
        property alias stayInComponentMode: stayInComponentModeCheckbox.checked
        property alias disableHubbles: disableHubblesCheckbox.checked
        property alias maxUndoLevels: maxUndoSpinBox.value
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property alias disableMultisampling: disableMultisamplingCheckbox.checked
    }

    Preferences
    {
        id: find
        section: "find"

        property alias findByAttributeSortLexically: fbavSortStyleCombobox.lexical
    }

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        ColumnLayout
        {
            spacing: Constants.spacing

            Label
            {
                font.bold: true
                text: qsTr("Find")
            }

            CheckBox
            {
                id: focusFoundNodesCheckbox
                text: qsTr("Focus Found Nodes")
            }

            CheckBox
            {
                id: focusFoundComponentsCheckbox
                enabled: focusFoundNodesCheckbox.checked
                text: qsTr("Switch Modes When Finding")
            }

            CheckBox
            {
                id: stayInComponentModeCheckbox

                Layout.leftMargin: Constants.margin * 2

                enabled: focusFoundNodesCheckbox.checked &&
                    focusFoundComponentsCheckbox.checked
                text: qsTr("…But Not From Component Mode")
            }

            RowLayout
            {
                Label { text: qsTr("Sort Find By Attribute:") }

                ComboBox
                {
                    id: fbavSortStyleCombobox

                    model: [qsTr("By Quantity"), qsTr("By Value")]

                    property bool lexical
                    onLexicalChanged: { currentIndex = lexical ? 1 : 0; }
                    onCurrentIndexChanged: { lexical = (currentIndex !== 0); }
                }
            }

            Label
            {
                Layout.topMargin: Constants.margin * 2

                font.bold: true
                text: qsTr("Help")
            }

            CheckBox
            {
                id: disableHubblesCheckbox
                text: qsTr("Disable Extended Help Tooltips")
            }

            Item { Layout.fillHeight: true }
        }

        ColumnLayout
        {
            spacing: Constants.spacing

            Label
            {
                font.bold: true
                text: qsTr("Performance")
            }

            CheckBox
            {
                id: disableMultisamplingCheckbox
                text: qsTr("Disable Multisampling (Restart Required)")
            }

            Text
            {
                Layout.preferredWidth: parent.width

                visible: Qt.platform.os === "osx"

                wrapMode: Text.WordWrap
                textFormat: Text.StyledText

                text: qsTr("Particularly on older hardware, further performance may " +
                    "be gained by running the application in Low Resolution: <a href=\"dummy\">Locate ") +
                    application.name + qsTr(" in Finder</a> and choose <b>Get Info</b> from its context menu, " +
                    "then select <b>Open In Low Resolution</b>. Restart ") + application.name + qsTr(".")

                PointingCursorOnHoverLink {}
                onLinkActivated: { QmlUtils.showAppInFileManager(); }
            }

            Label
            {
                Layout.topMargin: Constants.margin * 2

                font.bold: true
                text: qsTr("Other")
            }

            RowLayout
            {
                Label { text: qsTr("Maximum Undo Levels:") }

                SpinBox
                {
                    id: maxUndoSpinBox

                    Layout.preferredWidth: 50
                    minimumValue: 0
                    maximumValue: 50
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
