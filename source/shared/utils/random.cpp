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

#include "random.h"

#include <random>

static std::random_device randomh_rd;
static std::mt19937 randomh_mt19937(randomh_rd());

float u::rand(float low, float high)
{
    std::uniform_real_distribution<> distribution(low, high);
    return static_cast<float>(distribution(randomh_mt19937));
}

int u::rand(int low, int high)
{
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(randomh_mt19937);
}

QVector2D u::randQVector2D(float low, float high)
{
    return {rand(low, high), rand(low, high)};
}

QVector3D u::randQVector3D(float low, float high)
{
    return {rand(low, high), rand(low, high), rand(low, high)};
}

QColor u::randQColor()
{
    const int r = rand(0, 255);
    const int g = rand(0, 255);
    const int b = rand(0, 255);

    return {r, g, b};
}
