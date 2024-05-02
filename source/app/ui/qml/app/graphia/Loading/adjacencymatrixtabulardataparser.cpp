/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "adjacencymatrixtabulardataparser.h"

#include "shared/graph/edgelist.h"

#include "shared/loading/graphsizeestimate.h"
#include "shared/loading/adjacencymatrixutils.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/userelementdata.h"

#include "shared/utils/string.h"

#include <QString>
#include <QQmlEngine>

#include <map>

MatrixTypeResult AdjacencyMatrixTabularDataParser::onParseComplete()
{
    const auto& data = tabularData();
    QPoint topLeft;
    EdgeList edgeList;

    setProgress(-1);

    MatrixTypeResult result;

    if((result = AdjacencyMatrixUtils::isEdgeList(data))) // NOLINT bugprone-assignment-in-if-condition
    {
        for(size_t rowIndex = 0; rowIndex < data.numRows(); rowIndex++)
        {
            const NodeId source = data.valueAt(0, rowIndex).toInt();
            const NodeId target = data.valueAt(1, rowIndex).toInt();
            const double weight = data.valueAt(2, rowIndex).toDouble();
            const EdgeListEdge edge{source, target, weight};

            edgeList.emplace_back(edge);

            setProgress(static_cast<int>((rowIndex * 100) / data.numRows()));
        }
    }
    else if((result = AdjacencyMatrixUtils::isAdjacencyMatrix(data, &topLeft))) // NOLINT bugprone-assignment-in-if-condition
    {
        for(auto rowIndex = static_cast<size_t>(topLeft.y()); rowIndex < data.numRows(); rowIndex++)
        {
            for(auto columnIndex = static_cast<size_t>(topLeft.x()); columnIndex < data.numColumns(); columnIndex++)
            {
                const double weight = data.valueAt(columnIndex, rowIndex).toDouble();

                if(weight == 0.0)
                    continue;

                edgeList.emplace_back(EdgeListEdge{NodeId(rowIndex), NodeId(columnIndex), weight});
            }

            setProgress(static_cast<int>((rowIndex * 100) / data.numRows()));
        }
    }
    else
        return result;

    setProgress(-1);

    _binaryMatrix = true;
    double lastSeenWeight = 0.0;
    for(const auto& edge : edgeList)
    {
        if(lastSeenWeight != 0.0 && edge._weight != lastSeenWeight)
        {
            _binaryMatrix = false;
            break;
        }

        lastSeenWeight = edge._weight;
    }

    emit binaryMatrixChanged();

    _graphSizeEstimate = graphSizeEstimateThreshold(edgeList);
    emit graphSizeEstimateChanged();

    return result;
}
