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

import "../../../shared/ui/qml/Constants.js" as Constants

import "Controls"

Window
{
    id: pluginsWindow

    property var pluginDetails

    // These data members are undocumented, so this could break in future
    property string pluginDescription: pluginNamesList.contentItem.currentItem ?
        pluginNamesList.contentItem.currentItem.rowItem.itemModel.description : ""
    property string pluginImageSource: pluginNamesList.contentItem.currentItem ?
        pluginNamesList.contentItem.currentItem.rowItem.itemModel.imageSource : ""

    onVisibleChanged:
    {
        // Force selection of first row
        if(visible && pluginNamesList.rowCount > 0)
        {
            pluginNamesList.currentRow = 0;
            pluginNamesList.selection.select(0);
        }
    }

    title: qsTr("About Plugins")
    flags: Qt.Window|Qt.Dialog
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

        TableView
        {
            id: pluginNamesList

            Layout.rowSpan: pluginImageSource.length > 0 ? 3 : 2
            Layout.fillHeight: true

            TableViewColumn { role: "name" }
            headerDelegate: Item {}

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

