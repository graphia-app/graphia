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
import QtQuick.Window 2.2
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import app.graphia.Controls 1.0
import app.graphia.Shared 1.0
import app.graphia.Shared.Controls 1.0

Window
{
    id: root

    title: qsTr("Options")
    flags: Qt.Window|Qt.Dialog

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

            Frame
            {
                Layout.fillWidth: true
                Layout.fillHeight: true

                topPadding: 0
                leftPadding: 0
                rightPadding: 0
                bottomPadding: 0

                Component.onCompleted: { background.color = palette.light; }

                StackLayout
                {
                    anchors.fill: parent
                    currentIndex: tabBar.currentIndex

                    OptionsAppearance {}
                    OptionsMisc {}
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

