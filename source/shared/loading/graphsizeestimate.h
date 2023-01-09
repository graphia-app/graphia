/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef GRAPHSIZEESTIMATE_H
#define GRAPHSIZEESTIMATE_H

#include "shared/graph/edgelist.h"

#include <QVariantMap>

#include <limits>

QVariantMap graphSizeEstimateThreshold(EdgeList edgeList,
    size_t numSampleNodes = std::numeric_limits<size_t>::max(),
    size_t maxNodes = std::numeric_limits<size_t>::max());

QVariantMap graphSizeEstimateKnn(EdgeList edgeList, size_t k,
     size_t numSampleNodes = std::numeric_limits<size_t>::max(),
     size_t maxNodes = std::numeric_limits<size_t>::max());

#endif // GRAPHSIZEESTIMATE_H
