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

    CreateTransformDialog
    {
        id: createTransformDialog

        document: root.document
    }

    RowLayout
    {
        id: layout
        spacing: 0

        SlidingPanel
        {
            id: panel

            Layout.alignment: Qt.AlignTop
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

        ColumnLayout
        {
            Layout.alignment: Qt.AlignTop

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
                                               qsTr("transform"),
                                               qsTr("transforms"));
                    }

                    textColor: enabled ? enabledTextColor : disabledTextColor
                    hoverColor: heldColor

                    onClicked: { panel.show(); }
                }

                ToolButton
                {
                    iconName: "add"
                    tooltip: qsTr("Add Transform")
                    onClicked: { createTransformDialog.show(); }
                }
            }

            ToolButton
            {
                Layout.alignment: Qt.AlignRight

                enabled: !panel.hidden && list.count > 0
                iconName: "top"
                tooltip: qsTr("Hide")

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: enabled ? 1.0 : 0.0

                onClicked: { panel.hide(); }
            }
        }
    }
}
