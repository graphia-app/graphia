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

#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>

namespace u
{
struct Statistics
{
    std::vector<double> _values;

    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
    double _range = 0.0;
    size_t _largestIndex = 0u;

    double _sum = 0.0;
    double _sumSq = 0.0;
    double _sumAllSq = 0.0;
    double _variability = 0.0;
    double _magnitude = 0.0;

    double _mean = 0.0;
    double _variance = 0.0;
    double _stddev = 0.0;
    double _coefVar = 0.0;

    double inverse(double value) const
    {
        return (_range - (value - _min)) + _min;
    }
};

template<typename C, typename Fn>
Statistics findStatisticsFor(const C& container,
    const Fn& fn, bool storeValues = false)
{
    Statistics s;

    bool allPositive = true;

    size_t column = 0u;
    double largestValue = 0.0;
    auto n = static_cast<double>(container.size());

    std::vector<double> values;
    values.reserve(static_cast<size_t>(std::distance(std::begin(container), std::end(container))));
    for(const auto& element : container)
        values.emplace_back(fn(element));

    for(auto value : values)
    {
        allPositive = allPositive && !std::signbit(value);

        s._sum += value;
        s._sumSq += value * value;
        s._mean += value / n;
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
    s._variability = std::sqrt((n * s._sumSq) - s._sumAllSq);
    s._magnitude = std::sqrt(s._sumSq);

    double sum = 0.0;
    for(auto value : values)
    {
        double x = (value - s._mean);
        x *= x;
        sum += x;
    }

    s._variance = sum / n;
    s._stddev = std::sqrt(s._variance);
    s._coefVar = (allPositive && s._mean > 0.0) ? s._stddev / s._mean : std::nan("1");

    if(storeValues)
        s._values = std::move(values);

    return s;
}

template<typename C>
Statistics findStatisticsFor(const C& container, bool storeValues = false)
{
    return findStatisticsFor(container, [](typename C::const_reference& t) { return t; }, storeValues);
}

template<typename C>
double medianOf(const C& container)
{
    if(container.empty())
        return 0.0;

    std::vector<double> v{container.begin(), container.end()};

    auto mid = v.size() / 2;
    std::nth_element(v.begin(), v.begin() +
        static_cast<std::vector<double>::difference_type>(mid), v.end());
    auto median = v.at(mid);

    if(v.size() % 2 == 0)
    {
        auto max = *std::max_element(v.begin(), v.begin() +
            static_cast<std::vector<double>::difference_type>(mid));
        median = (max + median) / 2.0;
    }

    return median;
}

} // namespace u
#endif // STATISTICS_H
