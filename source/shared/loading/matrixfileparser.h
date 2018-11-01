#ifndef MATRIXFILEPARSER_H
#define MATRIXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "shared/plugins/userelementdata.h"

#include <csv/parser.hpp>

template<const char Delimiter>
class MatrixFileParser : public IParser
{
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    MatrixFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        TextDelimitedTabularDataParser<Delimiter> fileParser(this);

        if(!fileParser.parse(url, graphModel))
            return false;

        TabularData& tabularData = fileParser.tabularData();
        std::map<size_t, NodeId> rowToNodeId;
        std::map<size_t, NodeId> columnToNodeId;

        if(tabularData.numRows() > 0 && tabularData.numColumns() > 0)
        {
            bool hasColumnHeaders = false;
            bool hasRowHeaders = false;
            size_t dataStartRow = 0;
            size_t dataStartColumn = 0;

            // Check first column for row headers
            for(size_t rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
            {
                const auto& stringValue = tabularData.valueAt(0, rowIndex);
                // Not a header if I can convert to double
                if(!u::isNumeric(stringValue))
                {
                    hasRowHeaders = true;
                    break;
                }
            }
            // Check first row for column headers
            for(size_t columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
            {
                const auto& stringValue = tabularData.valueAt(columnIndex, 0);
                if(!u::isNumeric(stringValue))
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
                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
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
                        auto expectedRowName = _userNodeData->valueBy(rowToNodeId.at(rowIndex),
                                                                      QObject::tr("Node Name"));
                        const auto& actualRowName = tabularData.valueAt(0, rowIndex);

                        if(expectedRowName.toString() != actualRowName)
                            return false;
                    }
                    else
                    {
                        // Add row headers as nodes
                        auto nodeId = graphModel->mutableGraph().addNode();
                        _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
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

                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                              QObject::tr("Node %1").arg(rowIndex + 1));
                    rowToNodeId[rowIndex] = nodeId;
                    columnToNodeId[rowIndex] = nodeId;
                }
            }

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
                        _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"),
                                                  QString::number(doubleValue));
                    }
                }
            }
        }
        return true;
    }

    static bool canLoad(const QUrl& url)
    {
        // To check for a matrix first check the selected delimiter is valid for the file.
        // Then scan for headers. A matrix can optionally have column or row headers. Or none.
        // A matrix data rect must be square.
        if(TextDelimitedTabularDataParser<Delimiter>::canLoad(url))
        {
            std::ifstream matrixFile(url.toLocalFile().toStdString());

            auto parser = std::make_unique<aria::csv::CsvParser>(matrixFile);
            parser->delimiter(Delimiter);

            std::vector<std::string> potentialColumnHeaders;

            bool headerMatch = true;
            bool firstColumnAllDouble = true;
            bool firstRowAllDouble = true;
            size_t rowIndex = 0;

            const int LINE_SCAN_COUNT = 5;

            for(auto& row : *parser)
            {
                if(row.size() < 2)
                    return false;

                int columnIndex = 0;
                for(auto& field : row)
                {
                    if(rowIndex == 0)
                    {
                        if(!u::isNumeric(field) && field.empty() && columnIndex > 0)
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
                            if(!u::isNumeric(field) && field.empty())
                                firstColumnAllDouble = false;
                        }
                    }
                    else if (rowIndex > 0)
                    {
                        // Check non header elements are doubles
                        // This will prevent loading obviously non-matrix files
                        // We could handle non-double matrix symbols in future (X, -, I, O etc)
                        if(!u::isNumeric(field) && field.empty())
                            return false;
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
