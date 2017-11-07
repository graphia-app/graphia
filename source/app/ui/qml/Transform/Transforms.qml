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
                visible: !transformSummaryText.visible
                text: qsTr("Add Transform")

                textColor: enabled ? enabledTextColor : disabledTextColor
                hoverColor: heldColor

                onClicked: { createTransformDialog.show(); }
            }

            ToolButton
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
        alignment: Qt.AlignLeft | Qt.AlignBottom
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
                textFormat: Text.StyledText
                text: qsTr("Transforms are a powerful way to modify the graph and calculate additional attributes<br>" +
                      "They can be used to remove nodes, collapse edges, calculate node metrics and much more<br>" +
                      "Existing transforms will appear above <b>Add Transform</b><br><br>" +
                      "Click <b>Add Transform</b> to bring up the add transform dialog<br>" +
                      "Each attribute added to graph will also add a Visualisation<br>")
            }
        }
    }
}
