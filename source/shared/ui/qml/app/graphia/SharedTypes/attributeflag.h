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

#ifndef ATTRIBUTEFLAG_H
#define ATTRIBUTEFLAG_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(AttributeFlag,
    None                    = 0x0,

    // Automatically set the range
    AutoRange               = 0x1,

    // Visualise on a per-component basis, by default
    VisualiseByComponent    = 0x2,

    // Indicates this is a dynamically created attribute; set automatically
    Dynamic                 = 0x4,

    // Track the set of shared values held by the attribute
    FindShared              = 0x8,

    // Can't be used during transform
    DisableDuringTransform  = 0x10,

    // Can be searched by the various find methods
    Searchable              = 0x20);

#endif // ATTRIBUTEFLAG_H
