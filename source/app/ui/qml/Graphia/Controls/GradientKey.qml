/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

import QtQuick
import QtQuick.Layouts

import Graphia.Utils

Item
{
    id: root

    implicitWidth: layout.implicitWidth + root._padding
    implicitHeight: root.keyHeight + root._padding

    property int _padding: hoverEnabled ? Constants.padding : 0

    property int keyWidth
    property int keyHeight: 20

    property double minimum
    property double maximum

    property double mappedMinimum
    property double mappedMaximum

    property int _decimalPoints: Utils.decimalPointsForRange(root.minimum, root.maximum)

    property color hoverColor
    property color textColor: "black"

    property color _contrastingColor:
    {
        if(root.enabled && mouseArea.containsMouse && hoverEnabled)
            return NativeUtils.contrastingColor(hoverColor);

        return textColor;
    }

    property bool selected: false


    property bool showLabels: true
    property bool invert: false

    property alias hoverEnabled: mouseArea.hoverEnabled

    Component
    {
        id: stopComponent
        GradientStop {}
    }

    function updateGradient()
    {
        if(configuration === undefined || configuration.length === 0)
            return;

        let stops = [];

        let object = JSON.parse(configuration);
        for(let prop in object)
        {
            let color = object[prop];

            if(!root.enabled)
                color = Utils.desaturate(color, 0.2);

            stops.push(stopComponent.createObject(rectangle.gradient,
                { "position": prop, "color": color }));
        }

        rectangle.gradient.stops = stops;
    }

    onEnabledChanged:
    {
        updateGradient();
    }

    property string configuration
    onConfigurationChanged:
    {
        updateGradient();
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
            if(root.enabled && mouseArea.containsMouse && hoverEnabled)
                return root.hoverColor;
            else if(selected)
                return palette.highlight;

            return "transparent";
        }
    }

    RowLayout
    {
        id: layout

        anchors.centerIn: parent

        width: root.width !== undefined ? root.width - _padding : undefined
        height: root.height !== undefined ? root.height - _padding : undefined

        Text
        {
            id: minimumLabel

            visible: root.showLabels
            text:
            {
                if(mappedMinimum > minimum)
                    return "≤ " + NativeUtils.formatNumberScientific(root.mappedMinimum);

                return NativeUtils.formatNumberScientific(root.minimum);
            }
            color: root._contrastingColor
        }

        Item
        {
            // The wrapper item is here to give the
            // rotated Rectangle something to fill
            id: item

            width: root.keyWidth !== 0 ? root.keyWidth : 0
            Layout.fillWidth: root.keyWidth === 0
            Layout.fillHeight: true

            Rectangle
            {
                id: rectangle

                anchors.centerIn: parent
                width: parent.height
                height: parent.width
                radius: 2

                border.width: 0.5
                border.color: root._contrastingColor

                rotation: root.invert ? 90 : -90
                gradient: Gradient {}
            }
        }

        Text
        {
            id: maximumLabel

            visible: root.showLabels
            text:
            {
                if(mappedMaximum < maximum)
                    return "≥ " + NativeUtils.formatNumberScientific(root.mappedMaximum);

                return NativeUtils.formatNumberScientific(root.maximum);
            }
            color: root._contrastingColor
        }
    }

    MouseArea
    {
        id: mouseArea

        anchors.fill: root

        onClicked: function(mouse) { root.clicked(mouse); }
        onDoubleClicked: function(mouse) { root.doubleClicked(mouse); }

        hoverEnabled: true

        onPressed: function(mouse) { mouse.accepted = false; }
    }

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
}
