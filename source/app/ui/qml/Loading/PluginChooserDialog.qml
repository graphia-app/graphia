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
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import app.graphia 1.0

import SortFilterProxyModel 0.2

import "../../../../shared/ui/qml/Constants.js" as Constants

Dialog
{
    id: pluginChooserDialog

    title: qsTr("Multiple Plugins Applicable")
    width: 500

    property var application
    property var model

    property string fileUrl
    property string fileType
    property var pluginNames: []
    property string pluginName
    property bool inNewTab

    GridLayout
    {
        columns: 2

        width: parent.width
        anchors.margins: Constants.margin

        Text
        {
            text: QmlUtils.baseFileNameForUrl(fileUrl) +
                  qsTr(" may be loaded by two or more plugins. " +
                       "Please select how you wish to proceed below.")
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: qsTr("Open With Plugin:")
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: pluginChoice
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                id: proxyModel

                sourceModel: pluginChooserDialog.model
                filterRoleName: "name"
                filterPattern:
                {
                    let s = "";

                    for(let i = 0; i < pluginChooserDialog.pluginNames.length; i++)
                    {
                        if(i !== 0) s += "|";
                        s += pluginChooserDialog.pluginNames[i];
                    }

                    return s;
                }

                onFilterPatternChanged:
                {
                    // Reset to first item
                    pluginChoice.currentIndex = -1;
                    pluginChoice.currentIndex = 0;
                }
            }

            property var selectedPlugin:
            {
                let mappedIndex = proxyModel.mapToSource(currentIndex);
                return pluginChooserDialog.model.nameAtIndex(mappedIndex);
            }

            textRole: "name"
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted:
    {
        pluginName = pluginChoice.selectedPlugin;
    }
}
