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
import QtQuick.Controls 1.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import app.graphia 1.0

import SortFilterProxyModel 0.2

import "../../../../shared/ui/qml/Constants.js" as Constants

Dialog
{
    id: root

    width: 500

    property var model: null
    property string displayRole: "display"
    property string valueRole: "display"
    property string explanationText: ""
    property string choiceLabelText: ""

    property var values: []

    GridLayout
    {
        columns: 2

        width: parent.width
        anchors.margins: Constants.margin

        Text
        {
            text: explanationText
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: root.choiceLabelText
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: comboBox
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                id: proxyModel

                sourceModel: root.model
                filterRoleName: root.valueRole
                filterPattern:
                {
                    let s = "";

                    for(let i = 0; i < root.values.length; i++)
                    {
                        if(i !== 0) s += "|";
                        s += root.values[i];
                    }

                    return s;
                }

                onFilterPatternChanged:
                {
                    // Reset to first item
                    comboBox.currentIndex = -1;
                    comboBox.currentIndex = 0;
                }
            }

            property string selectedValue:
            {
                if(root.model === null)
                    return "";

                let row = proxyModel.mapToSource(currentIndex);
                if(row < 0)
                    return "";

                let role = QmlUtils.modelRoleForName(root.model, root.valueRole);
                return root.model.data(root.model.index(row, 0), role);
            }

            textRole: root.displayRole
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    property var _onAcceptedFn: null

    onAccepted:
    {
        //FIXME: macOS seems to need an explicit close, for some reason
        close();

        if(_onAcceptedFn !== null)
            _onAcceptedFn(comboBox.selectedValue);
    }

    function show(onAcceptedFn)
    {
        root._onAcceptedFn = onAcceptedFn;
        Qt.callLater(function()
        {
            // Delay the opening in case an existing choice is still "in-flight",
            // e.g. when choosing a plugin immediately after choosing file type
            open();
        });
    }
}
