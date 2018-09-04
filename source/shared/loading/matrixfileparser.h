#ifndef MATRIXFILEPARSER_H
#define MATRIXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "shared/plugins/userelementdata.h"
#include "thirdparty/csv/parser.hpp"

template<const char Delimiter> class MatrixFileParser : public IParser
{
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;
public:
    MatrixFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData)
        : _userNodeData(userNodeData), _userEdgeData(userEdgeData) {}

public:
    bool parse(const QUrl &url, IGraphModel *graphModel) override
    {
        TextDelimitedTabularDataParser<Delimiter> fileParser(this);
        TabularData* tabularData = nullptr;

        if(!fileParser.parse(url, graphModel))
            return false;

        tabularData = &(fileParser.tabularData());
        std::map<size_t, NodeId> tablePositionToNodeId;

        bool hasColumnHeaders = false;
        bool hasRowHeaders = false;
        size_t dataStartRow = 0;
        size_t dataStartColumn = 0;

        // Prepass for headers
        if(tabularData->numRows() > 0 && tabularData->numColumns() > 0)
        {
            // Set temporarily to true
            hasRowHeaders = true;
            hasColumnHeaders = true;
            for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
            {
                auto stringValue = tabularData->valueAt(0, rowIndex);

                bool success = false;
                stringValue.toDouble(&success);
                if(success)
                {
                    // Probably doesnt if I can convert the header to double
                    hasRowHeaders = false;
                    break;
                }
            }
            // Column Headers
            for(size_t columnIndex = 0; columnIndex < tabularData->numColumns(); columnIndex++)
            {
                auto stringValue = tabularData->valueAt(columnIndex, 0);

                bool success = false;
                stringValue.toDouble(&success);
                if(success)
                {
                    // Probably doesnt have headers if I can convert the header to double
                    hasColumnHeaders = false;
                    break;
                }
            }

            if(hasColumnHeaders)
            {
                dataStartRow = 1;
                for(size_t columnIndex = hasRowHeaders ? 1 : 0; columnIndex < tabularData->numColumns(); columnIndex++)
                {
                    // Add Node names
                    auto nodeId = graphModel->mutableGraph().addNode();
                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                              tabularData->valueAt(columnIndex, 0));

                    tablePositionToNodeId[columnIndex] = nodeId;
                }
            }
            if(hasRowHeaders)
            {
                dataStartColumn = 1;
                for(size_t rowIndex = hasColumnHeaders ? 1 : 0; rowIndex < tabularData->numRows(); rowIndex++)
                {
                    if(hasColumnHeaders)
                    {
                        // Check row and column match (they should!)
                        auto expectedRowName = _userNodeData->valueBy(tablePositionToNodeId.at(rowIndex), QObject::tr("Node Name"));
                        auto actualRowName = tabularData->valueAt(0, rowIndex);

                        if(expectedRowName.toString() != actualRowName)
                            return false;
                    }
                    else
                    {
                        // Add Node names
                        auto nodeId = graphModel->mutableGraph().addNode();
                        _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                                  tabularData->valueAt(0, rowIndex));
                        tablePositionToNodeId[rowIndex] = nodeId;
                    }
                }
            }

            // No headers start from top left
            if(!hasColumnHeaders && !hasRowHeaders)
            {
                // Populate with "Node 1, Node 2..."
                for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
                {
                    // Node names
                    auto nodeId = graphModel->mutableGraph().addNode();

                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                              QObject::tr("Node %1").arg(rowIndex + 1));
                    tablePositionToNodeId[rowIndex] = nodeId;
                }
            }

            for(size_t rowIndex = dataStartRow; rowIndex < tabularData->numRows(); rowIndex++)
            {
                for(size_t columnIndex = dataStartColumn; columnIndex < tabularData->numColumns(); columnIndex++)
                {
                    // Edges
                    auto qStringValue = tabularData->valueAt(columnIndex, rowIndex);
                    bool success = false;
                    double doubleValue = qStringValue.toDouble(&success);
                    NodeId targetNode, sourceNode;

                    // Headers shift the map slightly
                    if(hasColumnHeaders && hasRowHeaders)
                    {
                        targetNode = tablePositionToNodeId.at(columnIndex);
                        sourceNode = tablePositionToNodeId.at(rowIndex);
                    }
                    else
                    {
                        targetNode = tablePositionToNodeId.at(columnIndex - dataStartColumn);
                        sourceNode = tablePositionToNodeId.at(rowIndex - dataStartRow);
                    }

                    if(success && doubleValue != 0.0)
                    {
                        auto edgeId = graphModel->mutableGraph().addEdge(sourceNode,
                                                                        targetNode);
                        _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(doubleValue));
                    }
                }
            }
        }
        return true;
    }
    static bool isType(const QUrl &url)
    {
        if(TextDelimitedTabularDataParser<Delimiter>::isType(url))
        {
            std::ifstream matrixFile(url.toLocalFile().toStdString());

            aria::csv::CsvParser parser(matrixFile);
            parser.delimiter(Delimiter);

            std::vector<std::string> potentialColumnHeaders;

            bool headerMatch = true;
            bool firstColumnAllDouble = true;
            bool firstRowAllDouble = true;
            size_t rowIndex = 0;

            const int LINE_SCAN_COUNT = 5;

            for(auto& row : parser)
            {
                if(row.size() < 2)
                    return false;

                int fieldIndex = 0;
                for (auto& field : row)
                {
                    if(rowIndex == 0)
                    {
                        bool isDouble = false;
                        QString::fromStdString(field).toDouble(&isDouble);
                        if(!isDouble && field != "" && fieldIndex > 0)
                            firstRowAllDouble = false;

                        potentialColumnHeaders.push_back(field);
                    }

                    if(fieldIndex == 0)
                    {
                        if(rowIndex >= potentialColumnHeaders.size()
                                || potentialColumnHeaders[rowIndex] != field)
                        {
                            headerMatch = false;
                        }

                        // The first entry could be headers so don't check for a double
                        if(rowIndex > 0)
                        {
                            bool isDouble = false;
                            QString::fromStdString(field).toDouble(&isDouble);
                            if(!isDouble && field != "")
                                firstColumnAllDouble = false;
                        }
                    }
                    fieldIndex++;
                }
                rowIndex++;

                if(rowIndex == LINE_SCAN_COUNT)
                    break;
            }

            if(headerMatch || firstColumnAllDouble || firstRowAllDouble)
                return true;
        }

        return false;
    }
};

#endif // MATRIXFILEPARSER_H
