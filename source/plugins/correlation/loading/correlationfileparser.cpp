#include "correlationfileparser.h"

#include "shared/graph/imutablegraph.h"
#include "../correlationplugin.h"

#include "shared/loading/tabulardata.h"

#include "shared/utils/utils.h"

#include <QRect>

#include <vector>
#include <stack>

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* correlationPluginInstance) :
    _correlationPluginInstance(correlationPluginInstance)
{}

static QRect findLargestDataRect(const TabularData& tabularData)
{
    std::vector<int> heightHistogram(tabularData.numColumns());

    for(int column = 0; column < tabularData.numColumns(); column++)
    {
        for(int row = tabularData.numRows() - 1; row >= 0; row--)
        {
            auto& value = tabularData.valueAt(column, row);
            if(u::isNumeric(value))
                heightHistogram.at(column)++;
            else
                break;
        }
    }

    std::stack<int> heights;
    std::stack<int> indexes;
    QRect dataRect;

    for(int index = 0; index < static_cast<int>(heightHistogram.size()); index++)
    {
        if(heights.empty() || heightHistogram[index] > heights.top())
        {
            heights.push(heightHistogram[index]);
            indexes.push(index);
        }
        else if(heightHistogram[index] < heights.top())
        {
            int lastIndex = 0;

            while(!heights.empty() && heightHistogram[index] < heights.top())
            {
                lastIndex = indexes.top(); indexes.pop();
                int height = heights.top(); heights.pop();
                int width = (index - lastIndex);
                int area = width * height;
                if(area > (dataRect.width() * dataRect.height()))
                {
                    dataRect.setLeft(lastIndex);
                    dataRect.setTop(tabularData.numRows() - height);
                    dataRect.setWidth(width);
                    dataRect.setHeight(height);
                }
            }

            heights.push(heightHistogram[index]);
            indexes.push(lastIndex);
        }
    }

    while(!heights.empty())
    {
        int lastIndex = indexes.top(); indexes.pop();
        int height = heights.top(); heights.pop();
        int width = (static_cast<int>(heightHistogram.size()) - lastIndex);
        int area = width * height;
        if(area > (dataRect.width() * dataRect.height()))
        {
            dataRect.setLeft(lastIndex);
            dataRect.setTop(tabularData.numRows() - height);
            dataRect.setWidth(width);
            dataRect.setHeight(height);
        }
    }

    return dataRect;
}

bool CorrelationFileParser::parse(const QUrl& url, IMutableGraph& graph, const IParser::ProgressFn& progress)
{
    CsvFileParser csvFileParser;

    if(csvFileParser.parse(url, graph, progress))
    {
        auto& tabularData = csvFileParser.tabularData();
        int numDataPoints = tabularData.numColumns() * tabularData.numRows();

        graph.setPhase(QObject::tr("Finding Data Points"));
        progress(-1);
        auto dataRect = findLargestDataRect(tabularData);

        int numDataColumns = dataRect.width();
        int numDataRows = dataRect.height();

        _correlationPluginInstance->_dataColumnNames.resize(dataRect.width());
        _correlationPluginInstance->_data.resize(dataRect.width() * dataRect.height());

        graph.setPhase(QObject::tr("Attributes"));
        int percentComplete = 0;

        for(int row = 0; row < tabularData.numRows(); row++)
        {
            for(int column = 0; column < tabularData.numColumns(); column++)
            {
                if(cancelled())
                {
                    graph.clearPhase();
                    return false;
                }

                int newPercentComplete = ((column + (row * tabularData.numColumns())) * 100) /
                        numDataPoints;

                if(newPercentComplete > percentComplete)
                {
                    percentComplete = newPercentComplete;
                    progress(newPercentComplete);
                }

                QString value = tabularData.valueAtQString(column, row);
                int dataColumn = column - dataRect.left();
                int dataRow = row - dataRect.top();

                if(row == 0)
                {
                    if(dataColumn < 0)
                    {
                        // Row attribute names
                        _correlationPluginInstance->_rowAttributes.emplace(
                                    value, std::vector<QString>(numDataRows));
                    }
                    else
                    {
                        // Data column names
                        _correlationPluginInstance->_dataColumnNames.at(dataColumn) = value;
                    }
                }
                else if(dataRow < 0)
                {
                    if(column == 0)
                    {
                        // Column attribute names
                        _correlationPluginInstance->_columnAttributes.emplace(
                                    value, std::vector<QString>(numDataColumns));
                    }
                    else if(dataColumn >= 0)
                    {
                        // Column attributes
                        QString columnAttributeName = tabularData.valueAtQString(0, row);
                        _correlationPluginInstance->_columnAttributes[columnAttributeName].at(dataColumn) = value;
                    }
                }
                else
                {
                    if(dataColumn < 0)
                    {
                        // Row attributes
                        QString rowAttributeName = tabularData.valueAtQString(column, 0);
                        _correlationPluginInstance->_rowAttributes[rowAttributeName].at(dataRow) = value;
                    }
                    else
                    {
                        // Data
                        int dataIndex = dataColumn + (dataRow * numDataColumns);
                        _correlationPluginInstance->_data.at(dataIndex) = value.toDouble();
                    }
                }
            }
        }

        graph.clearPhase();

        return true;
    }

    return false;
}
