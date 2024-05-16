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

#ifndef VALUETYPE_H
#define VALUETYPE_H

#include "shared/utils/qmlenum.h"

DEFINE_QML_ENUM(
    Q_OBJECT, ValueType,
    Unknown     = 0x1,
    Int         = 0x2,
    Float       = 0x4,
    String      = 0x8,
    StringList  = 0x10,
    Numerical = Int|Float,
    Identifier = Int|String,
    All = Int|Float|String|StringList,
    Attribute   = 0x20 /* Used to indicate transform attribute parameters */);

#endif // VALUETYPE_H
