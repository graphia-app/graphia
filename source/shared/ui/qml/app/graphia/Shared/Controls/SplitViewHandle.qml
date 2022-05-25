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

import QtQuick 2.15

Rectangle
{
    id: root

    implicitWidth: 6
    implicitHeight: 6

    color: palette.window

    readonly property int orientation: parent.orientation

    Canvas
    {
        anchors.fill: parent

        onPaint: function(rect)
        {
            let context = getContext("2d");
            context.fillStyle = palette.midlight;

            const numPips = 5;
            const pipSize = 1.75;
            const separation = 6.5;
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
                context.lineWidth = 0.75;
                context.strokeStyle = palette.dark;
                context.stroke();

                if(parent.orientation === Qt.Vertical)
                    x += separation;
                else if(parent.orientation === Qt.Horizontal)
                    y += separation;
            }
        }
    }
}
