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

#ifndef ELEMENTVISUAL_H
#define ELEMENTVISUAL_H

#include "shared/ui/visualisations/ielementvisual.h"

struct ElementVisual : IElementVisual
{
    float _size = -1.0f;
    QColor _outerColor;
    QColor _innerColor;
    QString _text;
    float _textSize = -1.0f;
    QColor _textColor;
    Flags<VisualFlags> _state = VisualFlags::None;

    float size() const override { return _size; }
    QColor outerColor() const override { return _outerColor; }
    QColor innerColor() const override { return _innerColor; }
    QString text() const override { return _text; }
    float textSize() const override { return _textSize; }
    QColor textColor() const override { return _textColor; }
    Flags<VisualFlags> state() const override { return _state; }
};

#endif // ELEMENTVISUAL_H
