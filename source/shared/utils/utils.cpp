/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "utils.h"

#include <QCoreApplication>

int u::smallestPowerOf2GreaterThan(int x)
{
    if(x < 0)
        return 0;

    auto xu = static_cast<uint64_t>(x);
    xu--;
    xu |= xu >> 1;
    xu |= xu >> 2;
    xu |= xu >> 4;
    xu |= xu >> 8;
    xu |= xu >> 16;
    return static_cast<int>(xu + 1);
}

float u::normaliseAngle(float radians)
{
    while(radians > Constants::Pi())
        radians -= Constants::TwoPi();

    while(radians <= -Constants::Pi())
        radians += Constants::TwoPi();

    return radians;
}

static void initQtResources()
{
    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(js);
}

Q_COREAPP_STARTUP_FUNCTION(initQtResources)
