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

#ifndef GRAPHRENDERERTYPES_H
#define GRAPHRENDERERTYPES_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(TextAlignment,
    Right,
    Left,
    Centre,
    Top,
    Bottom);

DEFINE_QML_ENUM(TextState,
    Off,
    Selected,
    All,
    Focused);

DEFINE_QML_ENUM(EdgeVisualType,
    Cylinder,
    Arrow);

DEFINE_QML_ENUM(Projection,
    Unset = -1,
    Perspective,
    Orthographic,
    TwoDee);

DEFINE_QML_ENUM(Shading,
    Unset = -1,
    Smooth,
    Flat);

#endif // GRAPHRENDERERTYPES_H
