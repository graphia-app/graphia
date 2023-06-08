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

#ifndef IELEMENTVISUAL_H
#define IELEMENTVISUAL_H

#include "shared/utils/flags.h"

#include <QColor>

enum VisualFlags
{
    None          = 0x0,
    Selected      = 0x1,
    Unhighlighted = 0x2
};

struct IElementVisual
{
    IElementVisual() = default;
    IElementVisual(const IElementVisual&) = default;
    IElementVisual(IElementVisual&&) noexcept = default;
    IElementVisual& operator=(const IElementVisual&) = default;
    IElementVisual& operator=(IElementVisual&&) noexcept = default;

    virtual ~IElementVisual() = default;

    virtual float size() const = 0;
    virtual QColor outerColor() const = 0;
    virtual QColor innerColor() const = 0;
    virtual QString text() const = 0;
    virtual float textSize() const = 0;
    virtual Flags<VisualFlags> state() const = 0;
};

#endif // IELEMENTVISUAL_H
