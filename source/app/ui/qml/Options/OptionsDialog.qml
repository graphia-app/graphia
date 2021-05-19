/* Copyright © 2013-2021 Graphia Technologies Ltd.
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
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import "../../../../shared/ui/qml/Constants.js" as Constants

Window
{
    id: optionsWindow

    title: qsTr("Options")
    flags: Qt.Window|Qt.Dialog

    minimumWidth: 700
    minimumHeight: 420
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight

    property bool enabled: true

    ColumnLayout
    {
        id: column
        anchors.fill: parent
        anchors.margins: Constants.margin

        TabView
        {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true

            enabled: optionsWindow.enabled

            Tab
            {
                title: qsTr("Appearance")
                OptionsAppearance {}
            }

            Tab
            {
                title: qsTr("Misc")
                OptionsMisc {}
            }
        }

        Button
        {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignRight
            onClicked: optionsWindow.close()
        }
    }
}

