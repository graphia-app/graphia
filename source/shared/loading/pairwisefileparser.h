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

#ifndef PAIRWISEFILEPARSER_H
#define PAIRWISEFILEPARSER_H

#include "shared/loading/iparser.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"

#include <QString>

#include <map>

template<typename> class IUserElementData;
using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

template<typename TabularDataParser>
class PairwiseFileParser : public IParser
{
private:
    IUserNodeData* _userNodeData = nullptr;
    IUserEdgeData* _userEdgeData = nullptr;

public:
    PairwiseFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {
        // Add this up front, so that it appears first in the attribute table
        userNodeData->add(QObject::tr("Node Name"));
    }

    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        TabularDataParser parser(this);

        if(!parser.parse(url, graphModel))
            return false;

        const auto& data = parser.tabularData();
        setProgress(-1);

        std::map<QString, NodeId> nodeIdMap;

        for(size_t rowIndex = 0; rowIndex < data.numRows(); rowIndex++)
        {
            auto source = data.valueAt(0, rowIndex);
            auto target = data.valueAt(1, rowIndex);

            NodeId firstNodeId;
            NodeId secondNodeId;

            if(!u::contains(nodeIdMap, source))
            {
                firstNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(source, firstNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->setValueBy(firstNodeId, QObject::tr("Node Name"), source);
                    graphModel->setNodeName(firstNodeId, source);
                }
            }
            else
                firstNodeId = nodeIdMap[source];

            if(!u::contains(nodeIdMap, target))
            {
                secondNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(target, secondNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->setValueBy(secondNodeId, QObject::tr("Node Name"), target);
                    graphModel->setNodeName(secondNodeId, target);
                }
            }
            else
                secondNodeId = nodeIdMap[target];

            auto edgeId = graphModel->mutableGraph().addEdge(firstNodeId, secondNodeId);

            if(data.numColumns() == 3)
            {
                // We have an edge weight too
                double edgeWeight = data.valueAt(2, rowIndex).toDouble();

                if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
                    edgeWeight = 1.0;

                _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(edgeWeight));
            }

            setProgress(static_cast<int>((rowIndex * 100) / data.numRows()));
        }

        return true;
    }

    static bool canLoad(const QUrl& url)
    {
        if(!TabularDataParser::canLoad(url))
            return false;

        TabularDataParser parser;
        parser.setRowLimit(5);
        parser.parse(url);
        const auto& tabularData = parser.tabularData();

        if(tabularData.numColumns() == 2)
            return true;

        if(tabularData.numColumns() == 3)
        {
            auto thirdColumnType = tabularData.typeIdentity(2, 0).type();

            return thirdColumnType == TypeIdentity::Type::Float ||
                thirdColumnType == TypeIdentity::Type::Int;
        }

        return false;
    }
};

using PairwiseTSVFileParser =   PairwiseFileParser<TsvFileParser>;
using PairwiseSSVFileParser =   PairwiseFileParser<SsvFileParser>;
using PairwiseCSVFileParser =   PairwiseFileParser<CsvFileParser>;
using PairwiseXLSXFileParser =  PairwiseFileParser<XlsxTabularDataParser>;

#endif // PAIRWISEFILEPARSER_H
