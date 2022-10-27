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

#include "scopetimer.h"

#include "shared/utils/container.h"

#include <QDebug>

#include <algorithm>
#include <numeric>
#include <cmath>

ScopeTimer::ScopeTimer(const QString& name, size_t numSamples) :
    _name(name), _numSamples(numSamples)
{
    _elapsedTimer.start();
}

ScopeTimer::~ScopeTimer()
{
    if(_elapsedTimer.isValid())
        stop();
}

void ScopeTimer::stop()
{
    auto elapsed = _elapsedTimer.nsecsElapsed();
    ScopeTimerManager::instance()->submit(_name, elapsed, _numSamples);
    _elapsedTimer.invalidate();
}

void ScopeTimerManager::submit(const QString& name, qint64 elapsed, size_t numSamples)
{
    std::unique_lock<std::mutex> lock(_mutex);

    auto& result = _results[name];

    while(result.size() >= numSamples)
        result.pop_front();

    result.push_back(elapsed);
}

void ScopeTimerManager::reportToQDebug() const
{
    std::unique_lock<std::mutex> lock(_mutex);

    for(const auto& result : _results)
    {
        const auto& name = result.first;
        const auto& samples = result.second;

        if(samples.size() > 1)
        {
            auto sum = std::accumulate(samples.begin(), samples.end(), 0LL);
            double mean = static_cast<double>(sum) / static_cast<double>(samples.size());
            auto minMax = std::minmax_element(samples.begin(), samples.end());

            double stdDev = std::accumulate(samples.begin(), samples.end(), 0.0,
            [mean](auto partial, double value)
            {
                return partial + ((value - mean) * (value - mean));
            });

            stdDev /= static_cast<double>(samples.size());
            stdDev = std::sqrt(stdDev);

            mean /= 1000000.0;
            auto min = static_cast<double>(*minMax.first) / 1000000.0;
            auto max = static_cast<double>(*minMax.second) / 1000000.0;
            stdDev /= 1000000.0;

            auto last = static_cast<double>(samples.back()) / 1000000.0;

            qDebug() << name << QStringLiteral("%1/%2/%3/%4/%5 ms (mean/min/max/stddev/last)")
                .arg(mean).arg(min).arg(max).arg(stdDev).arg(last);
        }
        else
            qDebug() << name << (static_cast<double>(samples.front()) / 1000000.0) << "ms";
    }
}
