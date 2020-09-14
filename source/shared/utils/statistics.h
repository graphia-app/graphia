/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include <limits>
#include <cmath>

namespace u
{
struct Statistics
{
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
    double _range = 0.0;
    size_t _largestIndex = 0u;

    double _sum = 0.0;
    double _sumSq = 0.0;
    double _sumAllSq = 0.0;
    double _variability = 0.0;

    double _mean = 0.0;
    double _variance = 0.0;
    double _stddev = 0.0;
    double _coefVar = 0.0;
};

template<typename T, typename Fn,
    template<typename, typename...> class C, typename... Args>
Statistics findStatisticsFor(const C<T, Args...>& container, Fn&& fn)
{
    Statistics s;

    bool allPositive = true;

    size_t column = 0u;
    double largestValue = 0.0;

    for(const auto& element : container)
    {
        double value = fn(element);

        allPositive = allPositive && !std::signbit(value);

        s._sum += value;
        s._sumSq += value * value;
        s._mean += value / container.size();
        s._min = std::min(s._min, value);
        s._max = std::max(s._max, value);

        if(std::abs(value) > std::abs(largestValue))
        {
            s._largestIndex = column;
            largestValue = value;
        }

        column++;
    }

    s._range = s._max - s._min;
    s._sumAllSq = s._sum * s._sum;
    s._variability = std::sqrt((container.size() * s._sumSq) - s._sumAllSq);

    double sum = 0.0;
    for(const auto& element : container)
    {
        double value = fn(element);
        double x = (value - s._mean);
        x *= x;
        sum += x;
    }

    s._variance = sum / container.size();
    s._stddev = std::sqrt(s._variance);
    s._coefVar = (allPositive && s._mean > 0.0) ? s._stddev / s._mean : std::nan("1");

    return s;
}

template<typename T,
    template<typename, typename...> class C, typename... Args>
Statistics findStatisticsFor(const C<T, Args...>& container)
{
    return findStatisticsFor(container, [](const T& t) { return t; });
}

} // namespace u
#endif // STATISTICS_H
