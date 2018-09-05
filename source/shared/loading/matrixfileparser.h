#ifndef MATRIXFILEPARSER_H
#define MATRIXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "shared/plugins/userelementdata.h"
#include "thirdparty/csv/parser.hpp"

template<const char Delimiter>
class MatrixFileParser : public IParser
{
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    MatrixFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

public:
    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        TextDelimitedTabularDataParser<Delimiter> fileParser(this);
        TabularData* tabularData = nullptr;

        if(!fileParser.parse(url, graphModel))
            return false;

        tabularData = &(fileParser.tabularData());
        std::map<size_t, NodeId> tablePositionToNodeId;

        bool hasColumnHeaders = true;
        bool hasRowHeaders = true;
        size_t dataStartRow = 0;
        size_t dataStartColumn = 0;

        if(tabularData->numRows() > 0 && tabularData->numColumns() > 0)
        {
            // Check first column for row headers
            for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
            {
                auto stringValue = tabularData->valueAt(0, rowIndex);
                bool success = false;
                stringValue.toDouble(&success);
                // Not a header if I can convert to double
                if(success)
                {
                    hasRowHeaders = false;
                    break;
                }
            }
            // Check first row for column headers
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

            // Populate Nodes from headers
            if(hasColumnHeaders)
            {
                dataStartRow = 1;
                for(size_t columnIndex = hasRowHeaders ? 1 : 0; columnIndex < tabularData->numColumns();
                    columnIndex++)
                {
                    // Add column headers as nodes
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
                        // Nodes have already been added
                        // Check row and column match (they should!)
                        auto expectedRowName = _userNodeData->valueBy(tablePositionToNodeId.at(rowIndex),
                                                                      QObject::tr("Node Name"));
                        auto actualRowName = tabularData->valueAt(0, rowIndex);

                        if(expectedRowName.toString() != actualRowName)
                            return false;
                    }
                    else
                    {
                        // Add row headers as nodes
                        auto nodeId = graphModel->mutableGraph().addNode();
                        _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                                  tabularData->valueAt(0, rowIndex));
                        tablePositionToNodeId[rowIndex] = nodeId;
                    }
                }
            }

            // Check datarect is square
            if(tabularData->numRows() - dataStartRow != tabularData->numColumns() - dataStartColumn)
                return false;

            // Generate Node names if there are no headers
            if(!hasColumnHeaders && !hasRowHeaders)
            {
                // "Node 1, Node 2..."
                for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
                {
                    auto nodeId = graphModel->mutableGraph().addNode();

                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                              QObject::tr("Node %1").arg(rowIndex + 1));
                    tablePositionToNodeId[rowIndex] = nodeId;
                }
            }

            // Generate Edges from dataset
            for(size_t rowIndex = dataStartRow; rowIndex < tabularData->numRows(); rowIndex++)
            {
                for(size_t columnIndex = dataStartColumn; columnIndex < tabularData->numColumns();
                    columnIndex++)
                {
                    // Edges
                    auto qStringValue = tabularData->valueAt(columnIndex, rowIndex);
                    bool success = false;
                    double doubleValue = qStringValue.toDouble(&success);
                    NodeId targetNode, sourceNode;

                    // Headers shift the index/node map slightly
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
                        auto edgeId = graphModel->mutableGraph().addEdge(sourceNode, targetNode);
                        _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"),
                                                  QString::number(doubleValue));
                    }
                }
            }
        }
        return true;
    }
    static bool isType(const QUrl& url)
    {
        // To check for a matrix first check the selected delimiter is valid for the file.
        // Then scan for headers. A matrix can optionally have column or row headers. Or none.
        // A matrix data rect must be square.
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

                int columnIndex = 0;
                for(auto& field : row)
                {
                    if(rowIndex == 0)
                    {
                        bool isDouble = false;
                        QString::fromStdString(field).toDouble(&isDouble);
                        if(!isDouble && field != "" && columnIndex > 0)
                            firstRowAllDouble = false;

                        potentialColumnHeaders.push_back(field);
                    }

                    if(columnIndex == 0)
                    {
                        if(rowIndex >= potentialColumnHeaders.size() ||
                           potentialColumnHeaders[rowIndex] != field)
                        {
                            headerMatch = false;
                        }

                        // The first entry could be headers so don't enforce check for a double
                        if(rowIndex > 0)
                        {
                            bool isDouble = false;
                            QString::fromStdString(field).toDouble(&isDouble);
                            if(!isDouble && field != "")
                                firstColumnAllDouble = false;
                        }
                    }
                    columnIndex++;
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

using MatrixFileTSVParser = MatrixFileParser<'\t'>;
using MatrixFileSSVParser = MatrixFileParser<';'>;
using MatrixFileCSVParser = MatrixFileParser<','>;

#endif // MATRIXFILEPARSER_H
