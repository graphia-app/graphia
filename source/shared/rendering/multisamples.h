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

#ifndef MULTISAMPLES_H
#define MULTISAMPLES_H

#include "shared/utils/preferences.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
int multisamples()
{
#ifdef OPENGL_ES
    return 1;
#else
    if(u::getPref(u"visuals/disableMultisampling"_s).toBool())
        return 1;

    // This is the place where the default number of multisamples is set
    return 4;
#endif
}
} // namespace

#endif // MULTISAMPLES_H
