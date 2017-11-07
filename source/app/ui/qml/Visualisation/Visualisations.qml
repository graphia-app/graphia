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

            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
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
    }

    Hubble
    {
        title: qsTr("Add Visualisation")
        alignment: Qt.AlignLeft | Qt.AlignTop
        target: layout
        tooltipMode: true
        RowLayout
        {
            spacing: 10
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
                textFormat: Text.StyledText
                text: qsTr("Visualisations modify the appearance of the Graph depending on attributes<br>" +
                      "<b>Node Colour</b>, <b>Size</b> and <b>Text</b> can all be linked to an attribute<br>" +
                      "Existing visualisations can be modified from here and new ones added<br><br>" +
                      "Click <b>Add Visualisation</b> to bring up the add visualisation dialog")
            }
        }
    }
}
