import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../Constants.js" as Constants
import "../Utils.js" as Utils

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

    enabled: document.idle

    CreateVisualisationDialog
    {
        id: createVisualisationDialog

        document: root.document
    }

    GradientList
    {
        id: _gradientList
    }

    RowLayout
    {
        id: layout
        spacing: 0

        ColumnLayout
        {
            Layout.alignment: Qt.AlignBottom

            ToolButton
            {
                enabled: !panel.hidden && list.count > 0
                iconName: "bottom"
                tooltip: qsTr("Hide")

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: enabled ? 1.0 : 0.0

                onClicked: { panel.hide(); }
            }

            RowLayout
            {
                spacing: 0

                ToolButton
                {
                    iconName: "add"
                    tooltip: qsTr("Add Visualisation")
                    onClicked: { createVisualisationDialog.show(); }
                }

                ButtonMenu
                {
                    enabled: list.count > 0
                    visible: panel.hidden || list.count === 0

                    text:
                    {
                        return Utils.pluralise(list.count,
                                               qsTr("visualisation"),
                                               qsTr("visualisations"));
                    }

                    textColor: enabled ? enabledTextColor : disabledTextColor
                    hoverColor: heldColor

                    onClicked: { panel.show(); }
                }
            }
        }

        SlidingPanel
        {
            id: panel

            Layout.alignment: Qt.AlignBottom
            alignment: Qt.AlignBottom

            item: DraggableList
            {
                id: list

                component: Component
                {
                    Visualisation
                    {
                        property var document: root.document
                        gradientList: _gradientList

                        Component.onCompleted:
                        {
                            enabledTextColor = Qt.binding(function() { return root.enabledTextColor; });
                            disabledTextColor = Qt.binding(function() { return root.disabledTextColor; });
                            hoverColor = Qt.binding(function() { return root.heldColor; });
                        }
                    }
                }

                model: document.visualisations
                heldColor: root.heldColor
                parentWhenDragging: root

                alignment: Qt.AlignLeft

                onItemMoved: { document.moveVisualisation(from, to); }
            }
        }
    }
}
