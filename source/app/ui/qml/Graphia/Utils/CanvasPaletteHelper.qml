/*
 * Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

// Canvas onPaint functions may reference the palette in order to draw using theme
// appropriate colours. If the palette changes, there is no built-in mechanism by
// which the Canvas is notified that a repaint must occur. This is such a mechanism.

import QtQuick

Item
{
    // onPaletteChanged doesn't fire unless palette is referenced in some way, hence:
    property var dummy: palette

    onPaletteChanged:
    {
        if(!(parent instanceof Canvas))
        {
            console.log("The parent of a CanvasPaletteHelper must be a Canvas");
            return;
        }

        parent.requestPaint();
    }
}
