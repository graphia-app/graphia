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

#ifndef ADJACENCYMATRIXTABULARDATAPARSER_H
#define ADJACENCYMATRIXTABULARDATAPARSER_H

#include "tabulardataparser.h"

#include "shared/loading/tabulardata.h"

#include <QVariantMap>
#include <QPoint>

template<typename> class IUserElementData;
using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

class AdjacencyMatrixTabularDataParser : public TabularDataParser
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantMap graphSizeEstimate MEMBER _graphSizeEstimate NOTIFY graphSizeEstimateChanged)
    Q_PROPERTY(bool binaryMatrix MEMBER _binaryMatrix NOTIFY binaryMatrixChanged)

private:
    QVariantMap _graphSizeEstimate;
    bool _binaryMatrix = true;

    MatrixTypeResult onParseComplete() override;

signals:
    void graphSizeEstimateChanged();
    void binaryMatrixChanged();
};

#endif // ADJACENCYMATRIXTABULARDATAPARSER_H
