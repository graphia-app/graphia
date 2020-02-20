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

#ifndef VISUALISATIONINFO_H
#define VISUALISATIONINFO_H

#include "ui/alert.h"

#include <vector>
#include <limits>

#include <QString>

class VisualisationInfo
{
private:
    std::vector<Alert> _alerts;
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
    std::vector<QString> _stringValues;

public:
    template<typename... Args>
    void addAlert(Args&&... args)
    {
        _alerts.emplace_back(std::forward<Args>(args)...);
    }

    auto alerts() const { return _alerts; }

    auto min() const { return _min; }
    auto max() const { return _max; }
    void setMin(double min) { _min = min; }
    void setMax(double max) { _max = max; }

    void resetRange()
    {
        _min = std::numeric_limits<double>::max();
        _max = std::numeric_limits<double>::lowest();
    }

    bool hasRange() const { return _min <= _max; }

    void addStringValue(const QString& value) { _stringValues.emplace_back(value); }
    auto stringValues() const { return _stringValues; }
};

using VisualisationInfosMap = std::map<int, VisualisationInfo>;

#endif // VISUALISATIONINFO_H
