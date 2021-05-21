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
    id: root

    title: qsTr("Type Ambiguous")
    width: 500

    property var application
    property var model

    property url url
    property string urlText
    property var types: []
    property string type
    property bool inNewTab

    GridLayout
    {
        columns: 2

        width: parent.width
        anchors.margins: Constants.margin

        Text
        {
            text: urlText + qsTr(" may be interpreted as two or more possible formats. " +
                "Please select how you wish to proceed below.")
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: qsTr("Open As:")
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: fileTypeChoice
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                id: proxyModel

                sourceModel: model
                filterRoleName: "name"
                filterPattern:
                {
                    let s = "";

                    for(let i = 0; i < root.types.length; i++)
                    {
                        if(i !== 0) s += "|";
                        s += root.types[i];
                    }

                    return s;
                }

                onFilterPatternChanged:
                {
                    // Reset to first item
                    fileTypeChoice.currentIndex = -1;
                    fileTypeChoice.currentIndex = 0;
                }
            }

            property var selectedFileType:
            {
                let mappedIndex = proxyModel.mapToSource(currentIndex);
                return root.model.nameAtIndex(mappedIndex);
            }

            textRole: "individualDescription"
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted:
    {
        type = fileTypeChoice.selectedFileType;
    }
}
