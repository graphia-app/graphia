/* Copyright © 2013-2024 Graphia Technologies Ltd.
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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

Window
{
    id: root

    title: qsTr("Options")
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    minimumWidth: 700
    minimumHeight: 420
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight

    property var application: null

    property bool enabled: true

    ColumnLayout
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin

        ColumnLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            enabled: root.enabled

            TabBar
            {
                id: tabBar

                TabBarButton { text: qsTr("Appearance") }
                TabBarButton { text: qsTr("Miscellaneous") }
                TabBarButton { text: qsTr("Network") }
                TabBarButton { text: qsTr("Defaults") }
            }

            Outline
            {
                Layout.fillWidth: true
                Layout.fillHeight: true

                StackLayout
                {
                    anchors.fill: parent
                    currentIndex: tabBar.currentIndex

                    OptionsAppearance {}
                    OptionsMisc { application: root.application }
                    OptionsNetwork {}
                    OptionsDefaults { application: root.application }
                }
            }
        }

        Button
        {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignRight
            onClicked: root.close()
        }
    }
}

