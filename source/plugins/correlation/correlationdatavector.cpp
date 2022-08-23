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

#include "correlationdatavector.h"

#include "shared/utils/container.h"

#include <algorithm>
#include <cmath>

void ContinuousDataVector::update()
{
    _statistics = u::findStatisticsFor(_data);
}

void ContinuousDataVector::generateRanking() const
{
    _rankingVector = std::make_shared<ContinuousDataVector>(u::rankingOf(_data), _nodeId, _cost);
    _rankingVector->update();
}

const ContinuousDataVector* ContinuousDataVector::ranking() const
{
    return _rankingVector.get();
}
