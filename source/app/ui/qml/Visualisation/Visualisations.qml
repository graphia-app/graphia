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

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 0

            Text
            {
                id: visualisationSummaryText

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
                visible: !visualisationSummaryText.visible
                text: qsTr("Add Visualisation")

                textColor: enabled ? enabledTextColor : disabledTextColor
                hoverColor: heldColor

                onClicked: { createVisualisationDialog.show(); }
            }

            ToolButton
            {
                visible: list.count > 0
                iconName: panel.hidden ? "go-top" : "go-bottom"
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

        // @disable-check M300
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

                alignment: Qt.AlignRight

                onItemMoved: { document.moveVisualisation(from, to); }
            }
        }

        ColumnLayout
        {
            Layout.alignment: Qt.AlignBottom

            ToolButton
            {
                Layout.alignment:  Qt.AlignRight

                // enabled may be changed externally, so have an
                // internal visibility property that we control
                property bool _visible: !panel.hidden && list.count > 0

                enabled: _visible
                iconName: "go-bottom"
                tooltip: qsTr("Hide")

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: _visible ? 1.0 : 0.0

                onClicked: { panel.hide(); }
            }

            RowLayout
            {
                spacing: 0

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

                ToolButton
                {
                    iconName: "list-add"
                    tooltip: qsTr("Add Visualisation")
                    onClicked: { createVisualisationDialog.show(); }

                    Hubble
                    {
                        title: "Add Visualisation"
                        alignment: Qt.AlignLeft | Qt.AlignTop
                        Text
                        {
                            text: qsTr("Visualisations use attributes to change the appearance\nof nodes and edges")
                        }
                        ToolButton
                        {
                            iconName: "document-open"
                        }
                    }
                }
            }
        }
    }
}
