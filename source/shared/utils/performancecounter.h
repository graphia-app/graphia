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

#ifndef PERFORMANCECOUNTER_H
#define PERFORMANCECOUNTER_H

#include <chrono>
#include <deque>
#include <functional>

class PerformanceCounter
{
public:
    using ReportFn = std::function<void(float)>;
    explicit PerformanceCounter(std::chrono::seconds interval);

    void setReportFn(ReportFn f) { _f = std::move(f); }

    void tick();
    float ticksPerSecond();

private:
    static const int MAX_SAMPLES = 16;
    std::deque<std::chrono::time_point<std::chrono::high_resolution_clock>> _samples;

    std::chrono::time_point<std::chrono::high_resolution_clock> _lastReport;
    std::chrono::seconds _interval;
    ReportFn _f = [](float) {};
};

#endif // PERFORMANCECOUNTER_H
