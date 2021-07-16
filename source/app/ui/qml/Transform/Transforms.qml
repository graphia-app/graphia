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
import QtQuick.Layouts 1.3

import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

import "../Controls"

Item
{
    id: root
    width: layout.width
    height: layout.height

    property var document

    property color enabledTextColor
    property color disabledTextColor
    property color heldColor

    CreateTransformDialog
    {
        id: createTransformDialog

        document: root.document
    }

    ColumnLayout
    {
        id: layout
        spacing: 0

        // @disable-check M300
        SlidingPanel
        {
            id: panel

            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            alignment: Qt.AlignTop

            item: DraggableList
            {
                id: list

                component: Component
                {
                    Transform
                    {
                        property var document: root.document

                        Component.onCompleted:
                        {
                            enabledTextColor = Qt.binding(function() { return root.enabledTextColor; });
                            disabledTextColor = Qt.binding(function() { return root.disabledTextColor; });
                        }
                    }
                }

                model: document.transforms
                heldColor: root.heldColor
                parentWhenDragging: root

                alignment: Qt.AlignRight

                onItemMoved: { document.moveGraphTransform(from, to); }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 0

            Text
            {
                id: transformSummaryText

                visible: panel.hidden && list.count > 0
                text:
                {
                    return Utils.pluralise(list.count,
                                           qsTr("transform"),
                                           qsTr("transforms"));
                }
            }

            ButtonMenu
            {
                id: addTransformButton

                visible: !transformSummaryText.visible
                text: qsTr("Add Transform")
                font.bold: true

                textColor: enabled ? enabledTextColor : disabledTextColor
                hoverColor: heldColor

                onClicked: { createTransformDialog.show(); }
            }

            FloatingButton
            {
                visible: list.count > 0
                iconName: panel.hidden ? "go-bottom" : "go-top"
                tooltip: panel.hidden ? qsTr("Show") : qsTr("Hide")

                onClicked:
                {
                    if(panel.hidden)
                        panel.show();
                    else
                        panel.hide();
                }
            }
        }
    }

    Hubble
    {
        title: qsTr("Add Transform")
        alignment: Qt.AlignRight | Qt.AlignBottom
        edges: Qt.RightEdge | Qt.TopEdge
        target: addTransformButton
        tooltipMode: true
        RowLayout
        {
            spacing: Constants.spacing
            Column
            {
                Image
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "qrc:///imagery/mcl.svg"
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                    width: 150
                }
                Text
                {
                    text: qsTr("An MCL transform with a colour<br>visualisation applied")
                }
            }
            Text
            {
                Layout.preferredWidth: 500
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                text: qsTr("Transforms are a powerful way to modify the graph and calculate additional attributes. " +
                      "They can be used to remove nodes or edges, calculate metrics and much more. " +
                      "Transforms will appear above. They are applied in order, from top to bottom.<br><br>" +
                      "Click <b>Add Transform</b> to create a new one.")
            }
        }
    }
}
