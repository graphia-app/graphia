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

#ifndef PAIRWISEFILEPARSER_H
#define PAIRWISEFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/iuserelementdata.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/pairwisecolumntype.h"

#include <QString>

#include <map>
#include <cmath>

template<typename TabularDataParser>
class PairwiseFileParser : public IParser
{
private:
    IUserNodeData* _userNodeData = nullptr;
    IUserEdgeData* _userEdgeData = nullptr;
    TabularData _tabularData;

    bool _firstRowIsHeader = false;
    PairwiseColumnsConfiguration _columnsConfiguration =
    {
        {0, {PairwiseColumnType::SourceNode, {}}},
        {1, {PairwiseColumnType::TargetNode, {}}},
    };

public:
    void setFirstRowIsHeader(bool firstRowIsHeader) { _firstRowIsHeader = firstRowIsHeader; }
    void setColumnsConfiguration(const PairwiseColumnsConfiguration& columnsConfiguration)
    {
        _columnsConfiguration = columnsConfiguration;
    }

    PairwiseFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
        TabularData* tabularData = nullptr) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {
        if(tabularData != nullptr)
            _tabularData = std::move(*tabularData);

        // Add this up front, so that it appears first in the attribute table
        userNodeData->add(QObject::tr("Node Name"));
    }

    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        if(_tabularData.empty())
        {
            TabularDataParser parser(this);

            if(!parser.parse(url, graphModel))
                return false;

            _tabularData = std::move(parser.tabularData());
        }

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
                _attributeColumns.emplace_back(AttributeColumn{column, configuration._type, configuration._name});
                break;
            }
        }

        std::map<QString, NodeId> nodeIdMap;

        for(size_t rowIndex = _firstRowIsHeader ? 1 : 0; rowIndex < _tabularData.numRows(); rowIndex++)
        {
            auto source = _tabularData.valueAt(sourceNodeColumn, rowIndex);
            auto target = _tabularData.valueAt(targetNodeColumn, rowIndex);

            NodeId sourceNodeId;
            NodeId targetNodeId;

            if(!u::contains(nodeIdMap, source))
            {
                sourceNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(source, sourceNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->setValueBy(sourceNodeId, QObject::tr("Node Name"), source);
                    graphModel->setNodeName(sourceNodeId, source);
                }
            }
            else
                sourceNodeId = nodeIdMap[source];

            if(!u::contains(nodeIdMap, target))
            {
                targetNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(target, targetNodeId);

                if(_userNodeData != nullptr)
                {
                    _userNodeData->setValueBy(targetNodeId, QObject::tr("Node Name"), target);
                    graphModel->setNodeName(targetNodeId, target);
                }
            }
            else
                targetNodeId = nodeIdMap[target];

            auto edgeId = graphModel->mutableGraph().addEdge(sourceNodeId, targetNodeId);

            for(const auto& attributeColumn : _attributeColumns)
            {
                auto value = _tabularData.valueAt(attributeColumn._column, rowIndex);

                switch(attributeColumn._type)
                {
                case PairwiseColumnType::EdgeAttribute:
                    _userEdgeData->setValueBy(edgeId, attributeColumn._name, value); break;
                case PairwiseColumnType::SourceNodeAttribute:
                    _userNodeData->setValueBy(sourceNodeId, attributeColumn._name, value); break;
                case PairwiseColumnType::TargetNodeAttribute:
                    _userNodeData->setValueBy(targetNodeId, attributeColumn._name, value); break;

                default: break;
                }
            }

            setProgress(static_cast<int>((rowIndex * 100) / _tabularData.numRows()));
        }

        return true;
    }

    static bool canLoad(const QUrl& url)
    {
        return TabularDataParser::canLoad(url);
    }
};

using PairwiseCSVFileParser =   PairwiseFileParser<CsvFileParser>;
using PairwiseSSVFileParser =   PairwiseFileParser<SsvFileParser>;
using PairwiseTSVFileParser =   PairwiseFileParser<TsvFileParser>;
using PairwiseTXTFileParser =   PairwiseFileParser<TxtFileParser>;
using PairwiseXLSXFileParser =  PairwiseFileParser<XlsxTabularDataParser>;

#endif // PAIRWISEFILEPARSER_H
