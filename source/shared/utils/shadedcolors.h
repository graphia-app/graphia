/* Copyright © 2013-2025 Tim Angus
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

#ifndef SHADEDCOLORS_H
#define SHADEDCOLORS_H

#include "shared/utils/utils.h"

#include <QColor>
#include <QPalette>

#include <algorithm>

static QColor blend(const QPalette& palette, float f)
{
    const QColor& dark = palette.color(QPalette::Text);
    const QColor& light = palette.color(QPalette::Window);

    auto r = std::clamp(u::interpolate(dark.redF(),     light.redF(),   f), 0.0f, 1.0f);
    auto g = std::clamp(u::interpolate(dark.greenF(),   light.greenF(), f), 0.0f, 1.0f);
    auto b = std::clamp(u::interpolate(dark.blueF(),    light.blueF(),  f), 0.0f, 1.0f);

    QColor c;
    c.setRgbF(r, g, b);
    return c;
}

class ShadedColors
{
public:
    static QColor light(const QPalette& palette)     { return blend(palette, 1.1f); }
    static QColor neutral(const QPalette& palette)   { return blend(palette, 1.0f); }
    static QColor midlight(const QPalette& palette)  { return blend(palette, 0.95f); }
    static QColor mid(const QPalette& palette)       { return blend(palette, 0.65f); }
    static QColor dark(const QPalette& palette)      { return blend(palette, 0.5f); }
    static QColor shadow(const QPalette& palette)    { return blend(palette, 0.35f); }
    static QColor darkest(const QPalette& palette)   { return blend(palette, 0.0f); }
};

#endif // SHADEDCOLORS_H
