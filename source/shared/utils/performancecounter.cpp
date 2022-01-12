/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "performancecounter.h"

PerformanceCounter::PerformanceCounter(std::chrono::seconds interval) :
    _interval(interval)
{}

void PerformanceCounter::tick()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> now =
            std::chrono::high_resolution_clock::now();

    _samples.emplace_back(now);

    while(_samples.size() > MAX_SAMPLES)
        _samples.pop_front();

    if(now > _lastReport + _interval)
    {
        _f(ticksPerSecond());
        _lastReport = now;
    }
}

float PerformanceCounter::ticksPerSecond()
{
    float mean = 0.0f;
    size_t numSamples = _samples.size();

    for(size_t index = 0; index < numSamples - 1; index++)
    {
        auto start = _samples.at(index);
        auto end = _samples.at(index + 1);

        std::chrono::duration<float> duration = end - start;

        mean += duration.count() / static_cast<float>(numSamples);
    }

    if(mean == 0.0f)
        return 0.0f;

    return 1.0f / mean;
}
