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

import Graphia.Utils
import Graphia.Controls

Rectangle
{
    id: root

    implicitWidth: 7
    implicitHeight: 7

    readonly property int orientation: parent.orientation

    Canvas
    {
        id: canvas

        anchors.fill: parent

        onPaint: function(rect)
        {
            let context = getContext("2d");

            let gradientFactor = 1.02;
            let gradient = context.createLinearGradient(0, 0,
                parent.orientation === Qt.Horizontal ? parent.width  : 0,
                parent.orientation !== Qt.Horizontal ? parent.height : 0);
            gradient.addColorStop(0, ControlColors.light);
            gradient.addColorStop(0.05, Qt.lighter(ControlColors.neutral, gradientFactor));
            gradient.addColorStop(0.95, Qt.darker(ControlColors.neutral, gradientFactor));
            gradient.addColorStop(1, ControlColors.dark);

            context.fillStyle = gradient;
            context.fillRect(0, 0, parent.width, parent.height);

            context.fillStyle = ControlColors.midlight;

            const numPips = 4;
            const pipSize = 1.5;
            const separation = 5;
            const halfDimension = ((numPips * separation) - pipSize) * 0.5;

            let x = parent.width * 0.5;
            let y = parent.height * 0.5;

            if(parent.orientation === Qt.Vertical)
                x -= halfDimension;
            else if(parent.orientation === Qt.Horizontal)
                y -= halfDimension;

            for(let i = 0; i < numPips; i++)
            {
                context.beginPath();
                context.arc(x, y, pipSize, 0, 2 * Math.PI, false);

                context.fill();
                context.lineWidth = 0.7;
                context.strokeStyle = ControlColors.mid;
                context.stroke();

                if(parent.orientation === Qt.Vertical)
                    x += separation;
                else if(parent.orientation === Qt.Horizontal)
                    y += separation;
            }
        }

        CanvasPaletteHelper {}
    }

    onVisibleChanged:
    {
        // For some reason on macOS an initial paint doesn't happen, so force one
        // It's not really clear if this is a bug or some misunderstanding of how Canvas works...
        if(visible)
            canvas.requestPaint();
    }
}
