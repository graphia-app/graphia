/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick 2.7

import app.graphia 1.0
import app.graphia.Shared 1.0

Item
{
    id: root

    implicitWidth: root._width + root._padding

    property double _width:
    {
        let w = (root._numKeys * root.keyWidth) +
            ((root._numKeys - 1) * root._spacing);

        let minW = (root._numKeys - 1) *
            (root._spacing + root._minimumKeySize)

        if(root.hoverEnabled)
            minW += root.highlightSize;

        return Math.max(w, minW);
    }

    implicitHeight: row.implicitHeight + root._padding

    property double _borderRadius: 2
    property double _borderWidth: 0.5

    property bool separateKeys: true

    property int _padding: hoverEnabled ? Constants.padding : 0
    property int _spacing: separateKeys ? 2 : 0

    property int _numKeys:
    {
        if(!enabled || stringValues.length === 0)
            return repeater.count;

        return Math.min(repeater.count, stringValues.length);
    }

    property var stringValues: []

    property double _minimumKeySize: 10
    property double keyHeight: 20
    property double keyWidth: 30
    property double highlightSize: 120

    property color hoverColor
    property color textColor

    property color _contrastingColor:
    {
        if(mouseArea.containsMouse && root.hoverEnabled)
            return QmlUtils.contrastingColor(hoverColor);

        return textColor;
    }

    property bool hoverEnabled: true

    property var _fixedColors: []
    property bool _lastColorIsDefault: false

    function updatePalette()
    {
        if(configuration === undefined || configuration.length === 0)
            return;

        let palette = JSON.parse(configuration);

        let colors = [];
        let assignedValues = [];

        if(palette.autoColors !== undefined)
        {
            let numKeys = palette.autoColors.length;

            if(stringValues.length > 0 && stringValues.length < numKeys)
                numKeys = stringValues.length;

            for(let i = 0; i < numKeys; i++)
            {
                colors.push(palette.autoColors[i]);

                if(i < stringValues.length)
                    Utils.setAdd(assignedValues, stringValues[i]);
            }
        }

        if(palette.fixedColors !== undefined)
        {
            let fixedColors = [];

            for(let stringValue in palette.fixedColors)
            {
                if(root.stringValues.indexOf(stringValue) === -1)
                    continue;

                let color = palette.fixedColors[stringValue];
                let colorIndex = -1;

                if(!Utils.setContains(assignedValues, stringValue))
                {
                    colorIndex = colors.length;
                    colors.push(color);
                }
                else
                {
                    // stringValue has already been assigned, so replace
                    // it with the specified fixed color
                    colorIndex = stringValues.indexOf(stringValue);
                    colors[colorIndex] = color;
                }

                Utils.setAdd(assignedValues, stringValue);
                fixedColors.push({"index": colorIndex, "stringValue": stringValue});
            }

            root._fixedColors = fixedColors;
        }

        root._lastColorIsDefault = (palette.defaultColor !== undefined) &&
            ((colors.length < stringValues.length) || stringValues.length === 0);

        if(root._lastColorIsDefault)
            colors.push(palette.defaultColor)

        repeater.model = colors;
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
            if(mouseArea.containsMouse && root.hoverEnabled)
                return root.hoverColor;

            return "transparent";
        }
    }

    Row
    {
        id: row

        anchors.centerIn: parent

        spacing: root._spacing

        // We set a fixed width here, otherwise the row's width is defined
        // by its contents and can fluctuate very slightly when hovered,
        // presumably due to floating point rounding/precision. This needs
        // to be avoided because said fluctuation can cause infinite loops
        // when the cursor sits on a boundary between elements.
        width: root.width - root._padding

        Repeater
        {
            id: repeater
            Rectangle
            {
                id: key

                property bool _isLastColor: root._lastColorIsDefault &&
                    index === (root._numKeys - 1)
                property bool _hovered: !_isLastColor && index === root._indexUnderCursor

                implicitWidth:
                {
                    if(_hovered)
                        return root.highlightSize;

                    let w = root.width - root._padding;
                    w -= (root._numKeys - 1) * root._spacing;

                    let d = root._numKeys;

                    if(!root._lastColorhovered && root._indexUnderCursor !== -1)
                    {
                        w -= root.highlightSize;
                        d--;
                    }

                    return w / d;
                }

                implicitHeight: root.keyHeight

                radius: root.separateKeys ? root._borderRadius : 0.0

                border.width: root.separateKeys ? root._borderWidth : 0.0
                border.color: root.separateKeys ? root._contrastingColor : "transparent"

                property string stringValue:
                {
                    let fixedAssignment = root._fixedColors.find(function(element)
                    {
                        return element.index === index;
                    });

                    if(fixedAssignment !== undefined)
                        return fixedAssignment.stringValue;

                    if(index >= root.stringValues.length)
                        return "";

                    return root.stringValues[index];
                }

                color:
                {
                    let color = modelData;

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

                Canvas
                {
                    anchors.fill: parent
                    visible: key._isLastColor

                    onPaint: function(rect)
                    {
                        let ctx = getContext("2d");

                        let stripeColor = QmlUtils.contrastingColor(key.color);

                        ctx.beginPath();
                        ctx.strokeStyle = stripeColor;
                        ctx.lineWidth = 0.5;
                        ctx.lineCap = 'round';

                        ctx.moveTo(key.radius, key.radius);
                        ctx.lineTo(width - key.radius, height - key.radius);
                        ctx.stroke();

                        ctx.moveTo(key.radius, height - key.radius);
                        ctx.lineTo(width - key.radius, key.radius);
                        ctx.stroke();
                    }
                }
            }
        }
    }

    // Draw a border around everything, if we're not displaying the keys separately
    Rectangle
    {
        anchors.centerIn: parent

        visible: !root.separateKeys

        width: row.width
        height: row.height

        color: "transparent"

        border.width: root._borderWidth
        border.color: root._contrastingColor
    }

    property int _indexUnderCursor:
    {
        if(!mouseArea.containsMouse || !mouseArea.hoverEnabled)
            return -1;

        let coord = mapToItem(row, mouseArea.mouseX, mouseArea.mouseY);
        let w = row.width + root._spacing;
        let f = (coord.x - (root._spacing * 0.5)) / w;

        let index = Math.floor(f * root._numKeys);

        if(index >= root._numKeys)
            return -1;

        return index;
    }

    property bool _lastColorhovered: root._lastColorIsDefault && (root._numKeys - 1) === root._indexUnderCursor

    MouseArea
    {
        id: mouseArea

        anchors.fill: root

        onClicked: function(mouse) { root.clicked(mouse); }
        onDoubleClicked: function(mouse) { root.doubleClicked(mouse); }

        hoverEnabled: root.hoverEnabled

        onPressed: function(mouse) { mouse.accepted = false; }
    }

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
}
