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

#include "adjacencymatrixfileparser.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/userelementdata.h"

#include "shared/utils/string.h"

#include <QString>

#include <map>

namespace
{
bool parseAdjacencyMatrix(const TabularData& tabularData, Progressable& progressable,
    IGraphModel* graphModel, UserNodeData* userNodeData, UserEdgeData* userEdgeData)
{
    progressable.setProgress(-1);

    if(tabularData.numRows() == 0 || tabularData.numColumns() == 0)
        return false;

    std::map<size_t, NodeId> rowToNodeId;
    std::map<size_t, NodeId> columnToNodeId;
    bool hasColumnHeaders = false;
    bool hasRowHeaders = false;
    size_t dataStartRow = 0;
    size_t dataStartColumn = 0;

    // Check first column for row headers
    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        const auto& stringValue = tabularData.valueAt(0, rowIndex);
        // Not a header if I can convert to double
        if(rowIndex > 0 && !stringValue.isEmpty() && !u::isNumeric(stringValue))
        {
            hasRowHeaders = true;
            break;
        }
    }

    // Check first row for column headers
    for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
    {
        const auto& stringValue = tabularData.valueAt(columnIndex, 0);
        if(columnIndex > 0 && !stringValue.isEmpty() && !u::isNumeric(stringValue))
        {
            // Probably doesnt have headers if I can convert the header to double
            hasColumnHeaders = true;
            break;
        }
    }

    dataStartRow = hasColumnHeaders ? 1 : 0;
    dataStartColumn = hasRowHeaders ? 1 : 0;

    // Populate Nodes from headers
    if(hasColumnHeaders)
    {
        for(size_t columnIndex = dataStartColumn; columnIndex < tabularData.numColumns();
            columnIndex++)
        {
            // Add column headers as nodes
            auto nodeId = graphModel->mutableGraph().addNode();
            userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                     tabularData.valueAt(columnIndex, 0));

            columnToNodeId[columnIndex] = nodeId;
            rowToNodeId[dataStartRow + (columnIndex - dataStartColumn)] = nodeId;
        }
    }

    if(hasRowHeaders)
    {
        for(size_t rowIndex = dataStartRow; rowIndex < tabularData.numRows(); rowIndex++)
        {
            if(hasColumnHeaders)
            {
                // Nodes have already been added
                // Check row and column match (they should!)
                auto expectedRowName = userNodeData->valueBy(rowToNodeId.at(rowIndex),
                                                             QObject::tr("Node Name"));
                const auto& actualRowName = tabularData.valueAt(0, rowIndex);

                if(expectedRowName.toString() != actualRowName)
                    return false;
            }
            else
            {
                // Add row headers as nodes
                auto nodeId = graphModel->mutableGraph().addNode();
                userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                         tabularData.valueAt(0, rowIndex));
                rowToNodeId[rowIndex] = nodeId;
                columnToNodeId[dataStartColumn + (rowIndex - dataStartRow)] = nodeId;
            }
        }
    }

    // Check datarect is square
    auto dataHeight = tabularData.numRows() - dataStartRow;
    auto dataWidth = tabularData.numColumns() - dataStartColumn;
    if(dataWidth != dataHeight)
        return false;

    // Generate Node names if there are no headers
    if(!hasColumnHeaders && !hasRowHeaders)
    {
        // "Node 1, Node 2..."
        for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
        {
            auto nodeId = graphModel->mutableGraph().addNode();

            userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                     QObject::tr("Node %1").arg(rowIndex + 1));
            rowToNodeId[rowIndex] = nodeId;
            columnToNodeId[rowIndex] = nodeId;
        }
    }

    auto totalIterations = static_cast<uint64_t>(tabularData.numColumns() * tabularData.numRows());
    uint64_t progress = 0;

    // Generate Edges from dataset
    for(size_t rowIndex = dataStartRow; rowIndex < tabularData.numRows(); rowIndex++)
    {
        for(size_t columnIndex = dataStartColumn; columnIndex < tabularData.numColumns();
            columnIndex++)
        {
            // Edges
            const auto& qStringValue = tabularData.valueAt(columnIndex, rowIndex);
            bool success = false;
            double doubleValue = qStringValue.toDouble(&success);
            NodeId targetNode, sourceNode;

            targetNode = columnToNodeId.at(columnIndex);
            sourceNode = rowToNodeId.at(rowIndex);

            if(success && doubleValue != 0.0)
            {
                auto edgeId = graphModel->mutableGraph().addEdge(sourceNode, targetNode);
                userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"),
                                         QString::number(doubleValue));
            }

            progressable.setProgress(static_cast<int>((progress++ * 100) / totalIterations));
        }
    }

    progressable.setProgress(-1);

    return true;
}

bool parseEdgeList(const TabularData& tabularData, Progressable& progressable,
    IGraphModel* graphModel, UserNodeData* userNodeData, UserEdgeData* userEdgeData)
{
    std::map<QString, NodeId> nodeIdMap;

    size_t progress = 0;
    progressable.setProgress(-1);

    for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
    {
        const auto& firstCell = tabularData.valueAt(0, rowIndex);
        const auto& secondCell = tabularData.valueAt(1, rowIndex);
        auto edgeWeight = u::toNumber(tabularData.valueAt(2, rowIndex));

        NodeId firstNodeId;
        NodeId secondNodeId;

        if(!u::contains(nodeIdMap, firstCell))
        {
            firstNodeId = graphModel->mutableGraph().addNode();
            nodeIdMap.emplace(firstCell, firstNodeId);

            auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(firstNodeId + 1));
            userNodeData->setValueBy(firstNodeId, QObject::tr("Node Name"), nodeName);
            graphModel->setNodeName(firstNodeId, nodeName);
        }
        else
            firstNodeId = nodeIdMap[firstCell];

        if(!u::contains(nodeIdMap, secondCell))
        {
            secondNodeId = graphModel->mutableGraph().addNode();
            nodeIdMap.emplace(secondCell, secondNodeId);

            auto nodeName = QObject::tr("Node %1").arg(static_cast<int>(secondNodeId + 1));
            userNodeData->setValueBy(secondNodeId, QObject::tr("Node Name"), nodeName);
            graphModel->setNodeName(secondNodeId, nodeName);
        }
        else
            secondNodeId = nodeIdMap[secondCell];

        auto edgeId = graphModel->mutableGraph().addEdge(firstNodeId, secondNodeId);

        if(std::isnan(edgeWeight) || !std::isfinite(edgeWeight))
            edgeWeight = 1.0;

        userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(edgeWeight));

        progressable.setProgress(static_cast<int>((progress++ * 100) / tabularData.numRows()));
    }

    progressable.setProgress(-1);

    return true;
}
}

bool AdjacencyTabularDataMatrixParser::isAdjacencyMatrix(const TabularData& tabularData, size_t maxRows)
{
    // A matrix can optionally have column or row headers. Or none.
    // A matrix data rect must be square.
    std::vector<QString> potentialColumnHeaders;

    bool headerMatch = true;
    bool firstColumnAllDouble = true;
    bool firstRowAllDouble = true;

    if(tabularData.numColumns() < 2)
        return false;

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
                    return false;
            }
        }
    }

    auto numDataColumns = tabularData.numColumns() - (firstColumnAllDouble ? 1 : 0);
    auto numDataRows = tabularData.numRows() - (firstRowAllDouble ? 1 : 0);

    // Note we can't test for equality as we may not be seeing all the rows (due to a row limit)
    if(numDataColumns < numDataRows)
        return false;

    return headerMatch || firstColumnAllDouble || firstRowAllDouble;
}

bool AdjacencyTabularDataMatrixParser::isEdgeList(const TabularData& tabularData, size_t maxRows)
{
    if(tabularData.numColumns() != 3)
        return false;

    for(size_t rowIndex = 0; rowIndex < std::min(tabularData.numRows(), maxRows); rowIndex++)
    {
        for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
        {
            const auto& value = tabularData.valueAt(columnIndex, rowIndex);

            if(!u::isNumeric(value))
                return false;
        }
    }

    return true;
}

bool AdjacencyTabularDataMatrixParser::parse(const TabularData& tabularData, Progressable& progressable,
    IGraphModel* graphModel, UserNodeData* userNodeData, UserEdgeData* userEdgeData)
{
    if(isEdgeList(tabularData))
    {
        return parseEdgeList(tabularData, progressable,
            graphModel, userNodeData, userEdgeData);
    }
    else if(isAdjacencyMatrix(tabularData))
    {
        return parseAdjacencyMatrix(tabularData, progressable,
            graphModel, userNodeData, userEdgeData);
    }

    return false;
}
