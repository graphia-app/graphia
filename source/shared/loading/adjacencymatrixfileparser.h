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

#ifndef ADJACENCYMATRIXFILEPARSER_H
#define ADJACENCYMATRIXFILEPARSER_H

#include "shared/loading/iparser.h"

#include "shared/loading/adjacencymatrixutils.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/matlabfileparser.h"

#include "shared/utils/is_detected.h"

#include <type_traits>

template<typename> class IUserElementData;
using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

template<typename TabularDataParserType>
class AdjacencyMatrixFileParser : public IParser
{
private:
    IUserNodeData* _userNodeData = nullptr;
    IUserEdgeData* _userEdgeData = nullptr;
    TabularData _tabularData;

private:
    double _minimumAbsEdgeWeight = 0.0;
    bool _skipDuplicates = false;

    void addEdge(IGraphModel* graphModel,
        NodeId sourceNodeId, NodeId targetNodeId,
        double edgeWeight, double absEdgeWeight)
    {
        EdgeId edgeId;
        bool setWeights = true;

        if(_skipDuplicates)
            edgeId = graphModel->mutableGraph().firstEdgeIdBetween(sourceNodeId, targetNodeId);

        if(edgeId.isNull())
            edgeId = graphModel->mutableGraph().addEdge(sourceNodeId, targetNodeId);
        else
            setWeights = _userEdgeData->valueBy(edgeId, QObject::tr("Absolute Edge Weight")).toDouble() < absEdgeWeight;

        if(setWeights)
        {
            _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(edgeWeight));
            _userEdgeData->setValueBy(edgeId, QObject::tr("Absolute Edge Weight"),
                QString::number(absEdgeWeight));
        }
    }

    bool parseAdjacencyMatrix(const TabularData& tabularData, IGraphModel* graphModel)
    {
        setProgress(-1);

        if(tabularData.numRows() == 0 || tabularData.numColumns() == 0)
        {
            setFailureReason(QObject::tr("Matrix is empty."));
            return false;
        }

        bool hasColumnHeaders = false;
        bool hasRowHeaders = false;

               // Check first column for row headers
        for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
        {
            const auto& value = tabularData.valueAt(0, rowIndex);
            if(rowIndex > 0 && !value.isEmpty() && !u::isNumeric(value))
            {
                hasRowHeaders = true;
                break;
            }
        }

               // Check first row for column headers
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            const auto& value = tabularData.valueAt(columnIndex, 0);
            if(columnIndex > 0 && !value.isEmpty() && !u::isNumeric(value))
            {
                hasColumnHeaders = true;
                break;
            }
        }

        const size_t dataStartRow = hasColumnHeaders ? 1 : 0;
        const size_t dataStartColumn = hasRowHeaders ? 1 : 0;

               // Check datarect is square
        auto dataHeight = tabularData.numRows() - dataStartRow;
        auto dataWidth = tabularData.numColumns() - dataStartColumn;
        if(dataWidth != dataHeight)
        {
            setFailureReason(QObject::tr("Matrix is not square."));
            return false;
        }

        auto totalIterations = static_cast<uint64_t>(tabularData.numColumns() * tabularData.numRows());
        uint64_t progress = 0;

        std::map<size_t, NodeId> indexToNodeId;

        for(size_t rowIndex = dataStartRow; rowIndex < tabularData.numRows(); rowIndex++)
        {
            const QString rowHeader = hasRowHeaders ? tabularData.valueAt(0, rowIndex) : QString();

            for(size_t columnIndex = dataStartColumn; columnIndex < tabularData.numColumns(); columnIndex++)
            {
                const QString columnHeader = hasColumnHeaders ? tabularData.valueAt(columnIndex, 0) : QString();

                const auto& value = tabularData.valueAt(columnIndex, rowIndex);
                double edgeWeight = u::toNumber(value);

                if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
                    edgeWeight = 0.0;

                auto absEdgeWeight = std::abs(edgeWeight);

                if(absEdgeWeight <= _minimumAbsEdgeWeight)
                    continue;

                auto addNode = [this, &indexToNodeId, graphModel](size_t index, const QString& name)
                {
                    if(u::contains(indexToNodeId, index))
                        return indexToNodeId.at(index);

                    auto nodeId = graphModel->mutableGraph().addNode();
                    auto nodeName = !name.isEmpty() ? name : QObject::tr("Node %1").arg(index + 1);
                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
                    graphModel->setNodeName(nodeId, nodeName);

                    indexToNodeId[index] = nodeId;

                    return nodeId;
                };

                const NodeId sourceNodeId = addNode(columnIndex, columnHeader);
                const NodeId targetNodeId = addNode(rowIndex, rowHeader);
                addEdge(graphModel, sourceNodeId, targetNodeId, edgeWeight, absEdgeWeight);

                setProgress(static_cast<int>((progress++ * 100) / totalIterations));
            }

            if(cancelled())
            {
                setProgress(-1);
                return false;
            }
        }

        setProgress(-1);

        return true;
    }

    bool parseEdgeList(const TabularData& tabularData, IGraphModel* graphModel)
    {
        std::map<QString, NodeId> nodeIdMap;

        size_t progress = 0;
        setProgress(-1);

        for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
        {
            const auto& firstCell = tabularData.valueAt(0, rowIndex);
            const auto& secondCell = tabularData.valueAt(1, rowIndex);

            auto edgeWeight = u::toNumber(tabularData.valueAt(2, rowIndex));
            if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
                edgeWeight = 0.0;

            auto absEdgeWeight = std::abs(edgeWeight);

            if(absEdgeWeight <= _minimumAbsEdgeWeight)
                continue;

            NodeId sourceNodeId;
            NodeId targetNodeId;

            if(!u::contains(nodeIdMap, firstCell))
            {
                sourceNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(firstCell, sourceNodeId);

                auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(sourceNodeId + 1));
                _userNodeData->setValueBy(sourceNodeId, QObject::tr("Node Name"), nodeName);
                graphModel->setNodeName(sourceNodeId, nodeName);
            }
            else
                sourceNodeId = nodeIdMap[firstCell];

            if(!u::contains(nodeIdMap, secondCell))
            {
                targetNodeId = graphModel->mutableGraph().addNode();
                nodeIdMap.emplace(secondCell, targetNodeId);

                auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(targetNodeId + 1));
                _userNodeData->setValueBy(targetNodeId, QObject::tr("Node Name"), nodeName);
                graphModel->setNodeName(targetNodeId, nodeName);
            }
            else
                targetNodeId = nodeIdMap[secondCell];

            addEdge(graphModel, sourceNodeId, targetNodeId, edgeWeight, absEdgeWeight);

            if(cancelled())
            {
                setProgress(-1);
                return false;
            }

            setProgress(static_cast<int>((progress++ * 100) / tabularData.numRows()));
        }

        setProgress(-1);

        return true;
    }

public:
    AdjacencyMatrixFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
        TabularData* tabularData = nullptr) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {
        if(tabularData != nullptr)
            _tabularData = std::move(*tabularData);
    }

    void setMinimumAbsEdgeWeight(double minimumAbsEdgeWeight) { _minimumAbsEdgeWeight = minimumAbsEdgeWeight; }
    void setSkipDuplicates(bool skipDuplicates) { _skipDuplicates = skipDuplicates; }

    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        if(_tabularData.empty())
        {
            TabularDataParserType parser(this);

            if(!parser.parse(url, graphModel))
            {
                setFailureReason(parser.failureReason());
                return false;
            }

            _tabularData = std::move(parser.tabularData());
        }

        auto edgeListResult = AdjacencyMatrixUtils::isEdgeList(_tabularData);
        if(edgeListResult)
            return parseEdgeList(_tabularData, graphModel);

        auto adjacencyMatrixResult = AdjacencyMatrixUtils::isAdjacencyMatrix(_tabularData);
        if(adjacencyMatrixResult)
            return parseAdjacencyMatrix(_tabularData, graphModel);

        setFailureReason(QObject::tr("Failed to identify matrix type:\n %1\n %2")
            .arg(edgeListResult._reason, adjacencyMatrixResult._reason));

        return false;
    }

    QString log() const override
    {
        QString text;

        if(_minimumAbsEdgeWeight > 0.0)
        {
            text.append(QObject::tr("Minimum Absolute Edge Weight: %1").arg(
                u::formatNumberScientific(_minimumAbsEdgeWeight)));
        }

        if(_skipDuplicates)
        {
            if(!text.isEmpty()) text.append("\n");
            text.append(QObject::tr("Duplicate Edges Filtered"));
        }

        return text;
    }

    template<typename Parser>
    using setRowLimit_t = decltype(std::declval<Parser>().setRowLimit(0));

    static bool canLoad(const QUrl& url)
    {
        if(!TabularDataParserType::canLoad(url))
            return false;

        constexpr bool TabularDataParserHasSetRowLimit =
            std::experimental::is_detected_v<setRowLimit_t, TabularDataParserType>;

        // If TabularDataParserType has ::setRowLimit, do some additional checks
        if constexpr(TabularDataParserHasSetRowLimit)
        {
            TabularDataParserType parser;
            parser.setRowLimit(5);
            parser.parse(url);
            const auto& tabularData = parser.tabularData();

            return AdjacencyMatrixUtils::isEdgeList(tabularData) ||
                AdjacencyMatrixUtils::isAdjacencyMatrix(tabularData);
        }

        return true;
    }
};

using AdjacencyMatrixTSVFileParser =    AdjacencyMatrixFileParser<TsvFileParser>;
using AdjacencyMatrixSSVFileParser =    AdjacencyMatrixFileParser<SsvFileParser>;
using AdjacencyMatrixCSVFileParser =    AdjacencyMatrixFileParser<CsvFileParser>;
using AdjacencyMatrixTXTFileParser =    AdjacencyMatrixFileParser<TxtFileParser>;
using AdjacencyMatrixXLSXFileParser =   AdjacencyMatrixFileParser<XlsxTabularDataParser>;
using AdjacencyMatrixMatLabFileParser = AdjacencyMatrixFileParser<MatLabFileParser>;

#endif // ADJACENCYMATRIXFILEPARSER_H
