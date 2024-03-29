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
import QtQuick.Layouts

import app.graphia.Controls
import app.graphia.Shared

Window
{
    id: pluginsWindow

    property var pluginDetails

    property string pluginDescription:
    {
        return pluginNamesList.modelRoleAt(pluginNamesList.selectedIndex, "description");
    }

    property string pluginImageSource:
    {
        return pluginNamesList.modelRoleAt(pluginNamesList.selectedIndex, "imageSource");
    }

    onVisibleChanged:
    {
        // Force selection of first row
        if(visible && pluginNamesList.count > 0)
            pluginNamesList.select(0);
    }

    title: qsTr("About Plugins")
    flags: Qt.Window|Qt.Dialog
    color: palette.window

    width: 600
    height: 400
    minimumWidth: 600
    minimumHeight: 400

    GridLayout
    {
        id: grid

        rows: 2
        columns: 2

        anchors.fill: parent
        anchors.margins: Constants.margin

        ListBox
        {
            id: pluginNamesList

            Layout.rowSpan: pluginImageSource.length > 0 ? 3 : 2
            Layout.fillHeight: true

            displayRole: "name"

            model: pluginDetails
        }

        Image
        {
            Layout.margins: Constants.margin
            visible: pluginImageSource.length > 0
            source: pluginImageSource
            sourceSize.width: 96
            sourceSize.height: 96
        }

        Text
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Constants.margin
            wrapMode: Text.WordWrap
            color: palette.buttonText

            text: pluginDescription
        }

        Button
        {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignRight
            onClicked: pluginsWindow.close()
        }
    }
}

