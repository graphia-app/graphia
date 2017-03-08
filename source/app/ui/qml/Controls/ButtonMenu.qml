import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQml 2.8

import "../Constants.js" as Constants

Item
{
    id: root
    width: button.width
    height: button.height
    implicitWidth: button.implicitWidth
    implicitHeight: button.implicitHeight

    property string defaultText: ""
    property string selectedValue: ""
    property color textColor

    property alias model: instantiator.model

    property bool checked: false
    onCheckedChanged:
    {
        if(!checked)
        {
            // Calling things that have underscores in their name is probably
            // not a good idea, but there doesn't appear to be a public API
            // to programmatically close a menu
            menu.__dismissAndDestroy();
        }
    }

    property ExclusiveGroup exclusiveGroup: null

    onExclusiveGroupChanged:
    {
        if(exclusiveGroup)
            exclusiveGroup.bindCheckable(root);
    }

    Button
    {
        id: button
        text: root.selectedValue != "" ? root.selectedValue : root.defaultText
        menu: Menu
        {
            id: menu

            onAboutToShow: root.checked = true
            onAboutToHide: root.checked = false

            Instantiator
            {
                id: instantiator
                delegate: MenuItem
                {
                    text: index >= 0 ? instantiator.model[index] : ""

                    onTriggered: { root.selectedValue = text; }
                }

                onObjectAdded: menu.insertItem(index, object)
                onObjectRemoved: menu.removeItem(object)
            }
        }

        style: ButtonStyle
        {

            background: Item
            {
                anchors.fill: parent

                Rectangle
                {
                    anchors.fill: parent
                    border.color: control.hovered ? "#888" : "transparent"
                    radius: 4

                    gradient: Gradient
                    {
                        GradientStop
                        {
                            position: 0
                            color: control.hovered ? (control.pressed ? "#77cccccc" : "#77eeeeee") :
                                                     "transparent"
                        }

                        GradientStop
                        {
                            position: 1
                            color: control.hovered ? (control.pressed ? "#77aaaaaa" : "#77cccccc") :
                                                     "transparent"
                        }
                    }
                }
            }

            label: Label
            {
                text: control.text
                color: textColor
                font.bold: true
            }

            padding
            {
                top: Constants.padding
                left: Constants.padding
                right: Constants.padding
                bottom: Constants.padding
            }
        }
    }
}
