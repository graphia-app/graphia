import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import "../../../../shared/ui/qml/Utils.js" as Utils

Item
{
    id: root

    implicitWidth: layout.implicitWidth + _padding
    implicitHeight: layout.implicitHeight + _padding

    property int _padding: 2 * 4

    property color hoverColor
    property color textColor

    property bool selected: false

    property bool propogatePresses: false

    property alias hoverEnabled: mouseArea.hoverEnabled

    function updatePalette()
    {
        if(configuration === undefined || configuration.length === 0)
            return;

        repeater.model = JSON.parse(configuration);
    }

    onEnabledChanged:
    {
        updatePalette();
    }

    property string configuration
    onConfigurationChanged:
    {
        updatePalette();
    }

    Rectangle
    {
        id: button

        anchors.centerIn: parent
        width: root.width
        height: root.height
        radius: 2
        color:
        {
            if(mouseArea.containsMouse && hoverEnabled)
                return root.hoverColor;
            else if(selected)
                return systemPalette.highlight;

            return "transparent";
        }
    }

    RowLayout
    {
        id: layout

        anchors.centerIn: parent

        width: root.width !== undefined ? root.width - _padding : undefined
        height: root.height !== undefined ? root.height - _padding : undefined

        Repeater
        {
            id: repeater
            Rectangle
            {
                width: 16
                height: 16
                radius: 2

                border.width: 0.5
                border.color: root.textColor

                color:
                {
                    var color = modelData;

                    if(!root.enabled)
                        color = Utils.desaturate(color, 0.2);

                    return color;
                }
            }
        }
    }

    MouseArea
    {
        id: mouseArea

        anchors.fill: root

        onClicked: root.clicked(mouse)
        onDoubleClicked: root.doubleClicked(mouse)

        hoverEnabled: true

        onPressed: { mouse.accepted = !propogatePresses; }
    }

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
}
