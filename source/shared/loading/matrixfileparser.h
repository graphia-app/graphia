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
                        auto expectedRowName = _userNodeData->valueBy(tablePositionToNodeId[rowIndex], QObject::tr("Node Name"));
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
                qDebug() << "Creating Nodes..." << tabularData->numRows();
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

                    if(success && doubleValue != 0.0)
                    {
                        auto edgeId = graphModel->mutableGraph().addEdge(tablePositionToNodeId[rowIndex],
                                                                        tablePositionToNodeId[columnIndex]);
                        _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(doubleValue));
                    }
                }
            }
        }
        return true;
    }
    static bool isType(const QUrl &url)
    {
        //Matrix Scanning
        std::string potentialDelimiters = ",;\t ";
        std::vector<std::string> potentialColumnHeaders;
        std::vector<size_t> columnAppearances(potentialDelimiters.size());

        std::ifstream matrixFile(url.toLocalFile().toStdString());
        char delimiter = '\0';

        const int LINE_SCAN_COUNT = 5;
        const int ALLOWED_COLUMN_COUNT_DELTA = 1;

        // Find the appropriate delimiter from list
        for(size_t i = 0; i < potentialDelimiters.size(); ++i)
        {
            auto testDelimiter = potentialDelimiters[i];
            aria::csv::CsvParser testParser(matrixFile);
            testParser.delimiter(testDelimiter);

            // Scan first few rows for matching columns
            size_t rowIndex = 0;
            size_t columnAppearancesMin = std::numeric_limits<size_t>::max();
            for(auto testRow : testParser)
            {
                if(rowIndex >= LINE_SCAN_COUNT)
                    break;

                columnAppearances[i] = std::max(testRow.size(), columnAppearances[i]);
                columnAppearancesMin = std::min(testRow.size(), columnAppearancesMin);

                if(columnAppearances[i] - columnAppearancesMin > ALLOWED_COLUMN_COUNT_DELTA)
                {
                    // Inconsistant column count so not a matrix
                    columnAppearances[i] = 0;
                    break;
                }

                rowIndex++;
            }

            matrixFile.clear();
            matrixFile.seekg(0, std::ios::beg);
        }
        std::vector<char> likelyDelimiters;
        size_t maxColumns = *std::max_element(columnAppearances.begin(), columnAppearances.end());
        if(maxColumns > 0)
        {
            for(size_t i = 0; i < columnAppearances.size(); ++i)
            {
                if(columnAppearances[i] >= maxColumns)
                    likelyDelimiters.push_back(potentialDelimiters[i]);
            }
        }

        if(likelyDelimiters.size() > 0)
        {
            //TO-DO: Handle multiple delimiters?
            delimiter = likelyDelimiters[0];

            // Found delimiter doesn't match the required delimiters so fail
            if(Delimiter != delimiter)
                return false;

            aria::csv::CsvParser parser(matrixFile);
            parser.delimiter(delimiter);

            bool headerMatch = true;
            bool firstColumnAllDouble = true;
            bool firstRowAllDouble = true;
            size_t rowIndex = 0;
            for(auto& row : parser)
            {
                int fieldIndex = 0;
                for (auto& field : row)
                {
                    if(rowIndex == 0)
                    {
                        bool isDouble = false;
                        QString::fromStdString(field).toDouble(&isDouble);
                        qDebug() << QString::fromStdString(field) << isDouble;
                        if(!isDouble && field != "")
                            firstRowAllDouble = false;

                        potentialColumnHeaders.push_back(field);
                    }

                    if(fieldIndex == 0)
                    {
                        if(rowIndex >= potentialColumnHeaders.size()
                                || potentialColumnHeaders[rowIndex] != field)
                        {
                            headerMatch = false;
                            bool isDouble = false;
                            if(rowIndex < potentialColumnHeaders.size() - 1)
                            {
                                QString::fromStdString(field).toDouble(&isDouble);
                                qDebug() << QString::fromStdString(field) << isDouble;
                                if(!isDouble && field != "")
                                    firstColumnAllDouble = false;
                            }
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
