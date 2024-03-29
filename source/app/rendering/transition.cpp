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

#include "transition.h"

#include "maths/interpolation.h"
#include "shared/utils/utils.h"

#include <QDebug>

#include <algorithm>

bool Transition::update(float dTime)
{
    if(!active())
        return false;

    _elapsed += dTime;

    float f = _duration > 0.0f ? std::clamp(_elapsed / _duration, 0.0f, 1.0f) : 1.0f;

    switch(_type)
    {
    case Type::Linear:
    default:
        f = Interpolation::linear(0.0f, 1.0f, f);
        break;

    case Type::EaseInEaseOut:
        f = Interpolation::easeInEaseOut(0.0f, 1.0f, f);
        break;

    case Type::Power:
        f = Interpolation::power(0.0f, 1.0f, f);
        break;

    case Type::InversePower:
        f = Interpolation::inversePower(0.0f, 1.0f, f);
        break;
    }

    _function(f);

    if(!active())
    {
        _finishing = true;
        for(auto& finishedFunction : _finishedFunctions)
            finishedFunction();
        _finishing = false;

        if(!_suppressSignals)
            emit finished();

        return false;
    }

    return true;
}

void Transition::cancel()
{
    if(active())
        _elapsed = _duration;
}

void Transition::willBeImmediatelyReused()
{
    _suppressSignals = true;
}

