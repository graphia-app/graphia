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

#include "adjacencymatrixfileparser.h"

#include "shared/graph/edgelist.h"

#include "shared/loading/graphsizeestimate.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/userelementdata.h"

#include "shared/utils/string.h"
#include "shared/utils/static_block.h"

#include <QString>
#include <QQmlEngine>

#include <map>

namespace
{
void addEdge(IGraphModel* graphModel, IUserEdgeData* userEdgeData,
    NodeId sourceNodeId, NodeId targetNodeId,
    double edgeWeight, double absEdgeWeight,
    bool skipDuplicates)
{
    EdgeId edgeId;
    bool setWeights = true;

    if(skipDuplicates)
        edgeId = graphModel->mutableGraph().firstEdgeIdBetween(sourceNodeId, targetNodeId);

    if(edgeId.isNull())
        edgeId = graphModel->mutableGraph().addEdge(sourceNodeId, targetNodeId);
    else
        setWeights = userEdgeData->valueBy(edgeId, QObject::tr("Absolute Edge Weight")).toDouble() < absEdgeWeight;

    if(setWeights)
    {
        userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(edgeWeight));
        userEdgeData->setValueBy(edgeId, QObject::tr("Absolute Edge Weight"),
            QString::number(absEdgeWeight));
    }
}

bool parseAdjacencyMatrix(const TabularData& tabularData, IParser& parser,
    IGraphModel* graphModel, IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
    double minimumAbsEdgeWeight, bool skipDuplicates)
{
    parser.setProgress(-1);

    if(tabularData.numRows() == 0 || tabularData.numColumns() == 0)
    {
        parser.setFailureReason(QObject::tr("Matrix is empty."));
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
        parser.setFailureReason(QObject::tr("Matrix is not square."));
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

            if(absEdgeWeight <= minimumAbsEdgeWeight)
                continue;

            auto addNode = [&indexToNodeId, graphModel, userNodeData](size_t index, const QString& name)
            {
                if(u::contains(indexToNodeId, index))
                    return indexToNodeId.at(index);

                auto nodeId = graphModel->mutableGraph().addNode();
                auto nodeName = !name.isEmpty() ? name : QObject::tr("Node %1").arg(index + 1);
                userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
                graphModel->setNodeName(nodeId, nodeName);

                indexToNodeId[index] = nodeId;

                return nodeId;
            };

            const NodeId sourceNodeId = addNode(columnIndex, columnHeader);
            const NodeId targetNodeId = addNode(rowIndex, rowHeader);
            addEdge(graphModel, userEdgeData, sourceNodeId, targetNodeId, edgeWeight, absEdgeWeight, skipDuplicates);

            parser.setProgress(static_cast<int>((progress++ * 100) / totalIterations));
        }

        if(parser.cancelled())
        {
            parser.setProgress(-1);
            return false;
        }
    }

    parser.setProgress(-1);

    return true;
}

bool parseEdgeList(const TabularData& tabularData, IParser& parser,
    IGraphModel* graphModel, IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
    double minimumAbsEdgeWeight, bool skipDuplicates)
{
    std::map<QString, NodeId> nodeIdMap;

    size_t progress = 0;
    parser.setProgress(-1);

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        const auto& firstCell = tabularData.valueAt(0, rowIndex);
        const auto& secondCell = tabularData.valueAt(1, rowIndex);

        auto edgeWeight = u::toNumber(tabularData.valueAt(2, rowIndex));
        if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
            edgeWeight = 0.0;

        auto absEdgeWeight = std::abs(edgeWeight);

        if(absEdgeWeight <= minimumAbsEdgeWeight)
            continue;

        NodeId sourceNodeId;
        NodeId targetNodeId;

        if(!u::contains(nodeIdMap, firstCell))
        {
            sourceNodeId = graphModel->mutableGraph().addNode();
            nodeIdMap.emplace(firstCell, sourceNodeId);

            auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(sourceNodeId + 1));
            userNodeData->setValueBy(sourceNodeId, QObject::tr("Node Name"), nodeName);
            graphModel->setNodeName(sourceNodeId, nodeName);
        }
        else
            sourceNodeId = nodeIdMap[firstCell];

        if(!u::contains(nodeIdMap, secondCell))
        {
            targetNodeId = graphModel->mutableGraph().addNode();
            nodeIdMap.emplace(secondCell, targetNodeId);

            auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(targetNodeId + 1));
            userNodeData->setValueBy(targetNodeId, QObject::tr("Node Name"), nodeName);
            graphModel->setNodeName(targetNodeId, nodeName);
        }
        else
            targetNodeId = nodeIdMap[secondCell];

        addEdge(graphModel, userEdgeData, sourceNodeId, targetNodeId, edgeWeight, absEdgeWeight, skipDuplicates);

        if(parser.cancelled())
        {
            parser.setProgress(-1);
            return false;
        }

        parser.setProgress(static_cast<int>((progress++ * 100) / tabularData.numRows()));
    }

    parser.setProgress(-1);

    return true;
}
} // namespace

QmlTabularDataParser::MatrixTypeResult AdjacencyMatrixTabularDataParser::onParseComplete()
{
    const auto& data = tabularData();
    QPoint topLeft;
    EdgeList edgeList;

    setProgress(-1);

    MatrixTypeResult result;

    if((result = isEdgeList(data))) // NOLINT bugprone-assignment-in-if-condition
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
    else if((result = isAdjacencyMatrix(data, &topLeft))) // NOLINT bugprone-assignment-in-if-condition
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

QmlTabularDataParser::MatrixTypeResult AdjacencyMatrixTabularDataParser::isAdjacencyMatrix(
    const TabularData& tabularData, QPoint* topLeft, size_t maxRows)
{
    // A matrix can optionally have column or row headers. Or none.
    // A matrix data rect must be square.
    std::vector<QString> potentialColumnHeaders;

    bool headerMatch = true;
    bool firstColumnAllDouble = true;
    bool firstRowAllDouble = true;

    if(tabularData.numColumns() < 2)
        return {false, tr("Matrix must have at least 2 rows/columns.")};

    for(size_t rowIndex = 0; rowIndex < std::min(tabularData.numRows(), maxRows); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            if(rowIndex == 0)
            {
                if(!u::isNumeric(value) && !value.isEmpty() && columnIndex > 0)
                    firstRowAllDouble = false;

                potentialColumnHeaders.push_back(value);
            }

            if(columnIndex == 0)
            {
                if(rowIndex >= potentialColumnHeaders.size() ||
                    potentialColumnHeaders[rowIndex] != value)
                {
                    headerMatch = false;
                }

                // The first entry could be headers so don't enforce check for a double
                if(rowIndex > 0)
                {
                    if(!u::isNumeric(value) && !value.isEmpty())
                        firstColumnAllDouble = false;
                }
            }
            else if(rowIndex > 0)
            {
                // Check non header elements are doubles
                // This will prevent loading obviously non-matrix files
                // We could handle non-double matrix symbols in future (X, -, I, O etc)
                if(!u::isNumeric(value) && !value.isEmpty())
                    return {false, tr("Matrix has non-numeric values.")};
            }
        }
    }

    auto numDataColumns = tabularData.numColumns() - (firstColumnAllDouble ? 1 : 0);
    auto numDataRows = tabularData.numRows() - (firstRowAllDouble ? 1 : 0);

    // Note we can't test for equality as we may not be seeing all the rows (due to a row limit)
    if(numDataColumns < numDataRows)
        return {false, tr("Matrix is not square.")};

    if(topLeft != nullptr)
    {
        topLeft->setX(firstColumnAllDouble ? 1 : 0);
        topLeft->setY(firstRowAllDouble ? 1 : 0);
    }

    if(headerMatch || firstColumnAllDouble || firstRowAllDouble)
        return {true, {}};

    return {false, !headerMatch ? tr("Matrix headers don't match.") :
        tr("Matrix doesn't have numeric first row or column.")};
}

QmlTabularDataParser::MatrixTypeResult AdjacencyMatrixTabularDataParser::isEdgeList(
    const TabularData& tabularData, size_t maxRows)
{
    if(tabularData.numColumns() != 3)
        return {false, tr("Edge list doesn't have 3 columns.")};

    for(size_t rowIndex = 0; rowIndex < std::min(tabularData.numRows(), maxRows); rowIndex++)
    {
        if(!u::isInteger(tabularData.valueAt(0, rowIndex)))
            return {false, tr("1st edge list column has a non-integral value.")};

        if(!u::isInteger(tabularData.valueAt(1, rowIndex)))
            return {false, tr("2nd edge list column has a non-integral value.")};

        if(!u::isNumeric(tabularData.valueAt(2, rowIndex)))
            return {false, tr("3rd edge list column has a non-numeric value.")};
    }

    return {true, {}};
}

bool AdjacencyMatrixTabularDataParser::parse(const TabularData& tabularData, IParser& parser,
    IGraphModel* graphModel, IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) const
{
    auto edgeListResult = isEdgeList(tabularData);
    if(edgeListResult)
    {
        return parseEdgeList(tabularData, parser,
            graphModel, userNodeData, userEdgeData,
            _minimumAbsEdgeWeight, _skipDuplicates);
    }

    auto adjacencyMatrixResult = isAdjacencyMatrix(tabularData);
    if(adjacencyMatrixResult)
    {
        return parseAdjacencyMatrix(tabularData, parser,
            graphModel, userNodeData, userEdgeData,
            _minimumAbsEdgeWeight, _skipDuplicates);
    }

    parser.setFailureReason(tr("Failed to identify matrix type:\n %1\n %2")
        .arg(edgeListResult._reason, adjacencyMatrixResult._reason));

    return false;
}

static_block
{
    qmlRegisterType<AdjacencyMatrixTabularDataParser>(APP_URI, APP_MAJOR_VERSION,
        APP_MINOR_VERSION, "AdjacencyMatrixTabularDataParser");
}
