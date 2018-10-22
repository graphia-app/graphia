import QtQuick 2.7
import QtQuick.Controls 1.5

import com.kajeka 1.0

import "../../../../shared/ui/qml/Utils.js" as Utils

Item
{
    id: root

    implicitWidth: _width + _padding
    implicitHeight: row.implicitHeight + _padding

    property int _padding: 2 * 4

    property var stringValues

    property double _minimumWidth:
    {
        return root.highlightSize +
            ((repeater.count - 1) * (row.spacing + _minimumKeySize));
    }

    property double _width:
    {
        var w = (repeater.count * root.keySize) +
            ((repeater.count - 1) * row.spacing);

        return Math.max(w, _minimumWidth);
    }

    property double _minimumKeySize: 10
    property double keySize: 20
    property double highlightSize: 100

    property color hoverColor
    property color textColor

    property color _contrastingColor:
    {
        if(mouseArea.containsMouse && hoverEnabled)
            return QmlUtils.contrastingColor(hoverColor);

        return textColor;
    }

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

    Row
    {
        id: row

        anchors.centerIn: parent

        width: root.width !== undefined ? root.width - _padding : undefined
        height: root.height !== undefined ? root.height - _padding : undefined

        spacing: 2

        Repeater
        {
            id: repeater
            Rectangle
            {
                id: key

                property bool _hovered: index === root._indexUnderCursor

                implicitWidth:
                {
                    if(_hovered)
                        return root.highlightSize;

                    var w = root._width;
                    w -= (repeater.count - 1) * row.spacing;

                    var d = repeater.count;

                    if(root._indexUnderCursor !== -1)
                    {
                        w -= root.highlightSize;
                        d--;
                    }

                    return w / d;
                }

                implicitHeight: root.keySize

                radius: 2

                border.width: 0.5
                border.color: root._contrastingColor

                property string stringValue:
                {
                    if(index >= root.stringValues.length)
                        return "";

                    return root.stringValues[index];
                }

                color:
                {
                    var color = modelData;

                    if(!root.enabled)
                        color = Utils.desaturate(color, 0.2);

                    return color;
                }

                Text
                {
                    anchors.fill: parent
                    anchors.margins: 3

                    verticalAlignment: Text.AlignVCenter

                    elide: Text.ElideRight
                    visible: parent._hovered

                    color: QmlUtils.contrastingColor(key.color)
                    text: parent.stringValue
                }
            }
        }
    }

    property int _indexUnderCursor:
    {
        if(!mouseArea.containsMouse)
            return -1;

        var coord = mapToItem(row, mouseArea.mouseX, mouseArea.mouseY);
        var w = row.width + row.spacing;
        var f = (coord.x - (row.spacing * 0.5)) / w;

        var index = Math.floor(f * repeater.count);

        return index;
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
