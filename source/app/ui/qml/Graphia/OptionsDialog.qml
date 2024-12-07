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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

Window
{
    id: root

    title: qsTr("Options")
    flags: Constants.defaultWindowFlags
    color: palette.window

    minimumWidth: 700
    minimumHeight: 420
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight

    property var applicationRef: null

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

                    OptionsAppearance { application: root.applicationRef }
                    OptionsMisc { application: root.applicationRef }
                    OptionsNetwork { application: root.applicationRef }
                    OptionsDefaults { application: root.applicationRef }
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

