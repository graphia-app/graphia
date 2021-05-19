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

#ifndef TRANSFORMINFO_H
#define TRANSFORMINFO_H

#include "ui/alert.h"

#include <vector>

class TransformInfo
{
private:
    std::vector<Alert> _alerts;

public:
    template<typename... Args>
    void addAlert(Args&&... args)
    {
        _alerts.emplace_back(std::forward<Args>(args)...);
    }

    auto alerts() const { return _alerts; }
};

using TransformInfosMap = std::map<int, TransformInfo>;

#endif // TRANSFORMINFO_H
