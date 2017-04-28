import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../Constants.js" as Constants

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

        DraggableList
        {
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

        Item
        {
            // This is a bit of a hack to get margins around the add button:
            // As the button has no top anchor, anchors.topMargin doesn't
            // work, so instead we just pad out the button by the margin
            // size with a parent Item

            anchors.right: parent.right

            width: addButton.width + Constants.margin * 2
            height: addButton.height + Constants.margin * 2

            Button
            {
                id: addButton

                enabled: root.document.idle

                anchors
                {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }

                text: qsTr("Add Transform")
                onClicked: { createTransformDialog.show(); }
            }
        }
    }
}
