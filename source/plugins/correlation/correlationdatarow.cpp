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

#include "correlationdatarow.h"

#include "shared/utils/container.h"

#include <algorithm>
#include <cmath>

void ContinuousDataRow::update()
{
    _statistics = u::findStatisticsFor(_data);
}

void ContinuousDataRow::generateRanking() const
{
    _rankingRow = std::make_shared<ContinuousDataRow>(u::rankingOf(_data), _nodeId, _cost);
}

const ContinuousDataRow* ContinuousDataRow::ranking() const
{
    return _rankingRow.get();
}
