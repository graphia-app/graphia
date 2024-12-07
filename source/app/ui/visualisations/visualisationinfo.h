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

#ifndef VISUALISATIONINFO_H
#define VISUALISATIONINFO_H

#include "app/ui/alert.h"

#include "shared/utils/statistics.h"

#include <vector>
#include <limits>

#include <QString>

class VisualisationInfo
{
private:
    std::vector<Alert> _alerts;
    u::Statistics _statistics;
    double _mappedMinimum = std::numeric_limits<double>::max();
    double _mappedMaximum = std::numeric_limits<double>::lowest();
    std::vector<QString> _stringValues;
    size_t _numApplications = 0;

public:
    template<typename... Args>
    void addAlert(Args&&... args)
    {
        _alerts.emplace_back(std::forward<Args>(args)...);
    }

    auto alerts() const { return _alerts; }

    const u::Statistics& statistics() const { return _statistics; }
    void setStatistics(const u::Statistics& statistics) { _statistics = statistics; }

    void setMappedMinimum(double mappedMinimum) { _mappedMinimum = mappedMinimum; }
    double mappedMinimum() const { return _mappedMinimum; }

    void setMappedMaximum(double mappedMaximum) { _mappedMaximum = mappedMaximum; }
    double mappedMaximum() const { return _mappedMaximum; }

    void addStringValue(const QString& value) { _stringValues.emplace_back(value); }
    auto stringValues() const { return _stringValues; }

    void setNumApplications(size_t numApplications) { _numApplications = numApplications; }
    size_t numApplications() const { return _numApplications; }
};

using VisualisationInfosMap = std::map<int, VisualisationInfo>;

#endif // VISUALISATIONINFO_H
