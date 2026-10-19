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

#include "pairwisefileparser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/utils/container.h"

#include <QString>
#include <QtGlobal>

#include <cstddef>
#include <map>
#include <utility>
#include <vector>

PairwiseParser::PairwiseParser(IUserNodeData* userNodeData,
    IUserEdgeData* userEdgeData, TabularData* tabularData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    if(tabularData != nullptr)
        _tabularData = std::move(*tabularData);

    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(nodeNameAttribute());
}

bool PairwiseParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    if(_tabularData.empty() && !parseTabularData(url, graphModel))
        return false;

    setProgress(-1);

    size_t sourceNodeColumn = 0;
    size_t targetNodeColumn = 0;

    struct AttributeColumn
    {
        size_t _column;
        PairwiseColumnType _type;
        QString _name;
    };

    std::vector<AttributeColumn> _attributeColumns;

    for(const auto& [column, configuration] : _columnsConfiguration)
    {
        Q_ASSERT(column < _tabularData.numColumns());

        switch(configuration._type)
        {
        case PairwiseColumnType::Unused: break;
        case PairwiseColumnType::SourceNode: sourceNodeColumn = column; break;
        case PairwiseColumnType::TargetNode: targetNodeColumn = column; break;
        default:
            _attributeColumns.emplace_back(AttributeColumn
            {
                ._column = column,
                ._type = configuration._type,
                ._name = configuration._name
            });
            break;
        }
    }

    std::map<QString, NodeId> nodeIdMap;

    auto nodeIdFor = [&](const QString& name)
    {
        // A row with only one node name creates that node, without an edge
        if(name.isEmpty())
            return NodeId();

        if(u::contains(nodeIdMap, name))
            return nodeIdMap.at(name);

        auto nodeId = graphModel->mutableGraph().addNode();
        nodeIdMap.emplace(name, nodeId);

        if(_userNodeData != nullptr)
        {
            _userNodeData->setValueBy(nodeId, nodeNameAttribute(), name);
            graphModel->setNodeName(nodeId, name);
        }

        return nodeId;
    };

    for(size_t rowIndex = _firstRowIsHeader ? 1 : 0; rowIndex < _tabularData.numRows(); rowIndex++)
    {
        auto sourceNodeId = nodeIdFor(_tabularData.valueAt(sourceNodeColumn, rowIndex));
        auto targetNodeId = nodeIdFor(_tabularData.valueAt(targetNodeColumn, rowIndex));

        EdgeId edgeId;
        if(!sourceNodeId.isNull() && !targetNodeId.isNull())
            edgeId = graphModel->mutableGraph().addEdge(sourceNodeId, targetNodeId);

        for(const auto& attributeColumn : _attributeColumns)
        {
            auto value = _tabularData.valueAt(attributeColumn._column, rowIndex);

            switch(attributeColumn._type)
            {
            case PairwiseColumnType::EdgeAttribute:
                if(_userEdgeData != nullptr && !edgeId.isNull())
                    _userEdgeData->setValueBy(edgeId, attributeColumn._name, value);
                break;
            case PairwiseColumnType::SourceNodeAttribute:
                if(_userNodeData != nullptr && !sourceNodeId.isNull())
                    _userNodeData->setValueBy(sourceNodeId, attributeColumn._name, value);
                break;
            case PairwiseColumnType::TargetNodeAttribute:
                if(_userNodeData != nullptr && !targetNodeId.isNull())
                    _userNodeData->setValueBy(targetNodeId, attributeColumn._name, value);
                break;

            default: break;
            }
        }

        setProgress(static_cast<int>((rowIndex * 100) / _tabularData.numRows()));
    }

    return true;
}
