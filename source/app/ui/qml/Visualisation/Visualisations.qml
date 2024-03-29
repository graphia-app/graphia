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
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

Item
{
    id: root
    width: layout.width
    height: layout.height

    property var document

    property color enabledTextColor
    property color disabledTextColor
    property color heldColor

    CreateVisualisationDialog
    {
        id: createVisualisationDialog

        document: root.document
    }

    GradientSelector { id: _gradientSelector }
    PaletteSelector { id: _paletteSelector }
    MappingSelector { id: _mappingSelector }

    ColumnLayout
    {
        id: layout
        spacing: 0

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 0

            Text
            {
                id: visualisationSummaryText

                color: enabled ? enabledTextColor : disabledTextColor
                visible: panel.hidden && list.count > 0
                text:
                {
                    return Utils.pluralise(list.count,
                                           qsTr("visualisation"),
                                           qsTr("visualisations"));
                }
            }

            ButtonMenu
            {
                id: addVisualisationButton

                visible: !visualisationSummaryText.visible
                text: qsTr("Add Visualisation")
                font.bold: true

                textColor: enabled ? enabledTextColor : disabledTextColor
                hoverColor: heldColor

                onClicked: function(mouse) { createVisualisationDialog.show(); }
            }

            FloatingButton
            {
                visible: list.count > 0
                icon.name: panel.hidden ? "go-top" : "go-bottom"
                text: panel.hidden ? qsTr("Show") : qsTr("Hide")

                onClicked: function(mouse)
                {
                    if(panel.hidden)
                        panel.show();
                    else
                        panel.hide();
                }
            }
        }

        SlidingPanel
        {
            id: panel

            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            alignment: Qt.AlignBottom

            item: DraggableList
            {
                id: list

                component: Component
                {
                    Visualisation
                    {
                        document: root.document
                        enabledTextColor: root.enabledTextColor
                        disabledTextColor: root.disabledTextColor
                        hoverColor: root.heldColor

                        gradientSelector: _gradientSelector
                        paletteSelector: _paletteSelector
                        mappingSelector: _mappingSelector
                    }
                }

                model: document.visualisations
                heldColor: root.heldColor

                alignment: Qt.AlignRight

                onItemMoved: function(from, to) { document.moveVisualisation(from, to); }
            }
        }
    }

    Hubble
    {
        title: qsTr("Add Visualisation")
        alignment: Qt.AlignRight | Qt.AlignTop
        edges: Qt.RightEdge | Qt.BottomEdge
        target: addVisualisationButton
        tooltipMode: true
        RowLayout
        {
            spacing: Constants.spacing
            Column
            {
                Image
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "qrc:///imagery/visualisations.svg"
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                    width: 200
                }
                Text
                {
                    text: qsTr("A graph with Colour, Size and Text <br>visualisations applied")
                }
            }
            Text
            {
                Layout.preferredWidth: 500
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                text: qsTr("Visualisations allow for the display of attribute values by modifying the appearance of the graph elements. " +
                      "Node or edge <b>Colour</b>, <b>Size</b> and <b>Text</b> can all be linked to an attribute. " +
                      "Visualisations can be created here or existing ones modified.")
            }
        }
    }
}
