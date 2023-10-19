/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

import app.graphia
import app.graphia.Shared.Controls

// The Rectangle based implementation of Outline can't render half pixel widths
// that you see on Retina displays, so on macOS we do it manually using a Canvas

Canvas
{
    id: root

    property bool outlineVisible: true
    readonly property real outlineWidth: outlineVisible ? 1 : 0
    property color color: ControlColors.outline

    clip: outlineVisible

    onPaint: function(rect)
    {
        let context = getContext("2d");
        context.clearRect(0, 0, width, height);

        if(!outlineVisible)
            return;

        context.beginPath();

        context.strokeStyle = root.color;
        context.lineWidth = 1.0 / Screen.devicePixelRatio;

        context.moveTo(0, 0);
        context.lineTo(width, 0);
        context.lineTo(width, height);
        context.lineTo(0, height);
        context.lineTo(0, 0);

        context.closePath();

        context.stroke();
    }

    onOutlineVisibleChanged: { requestPaint(); }
    onColorChanged: { requestPaint(); }
    PaletteChangeNotifier { onPaletteChanged: { root.requestPaint(); } }
    Connections
    {
        target: Screen
        function onDevicePixelRatioChanged() { root.requestPaint(); }
    }
}
