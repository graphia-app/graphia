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

        _correlationPluginInstance->_numColumns = dataRect.width();
        _correlationPluginInstance->_numRows = dataRect.height();

        _correlationPluginInstance->_dataColumnNames.resize(dataRect.width());
        _correlationPluginInstance->_data.resize(dataRect.width() * dataRect.height());

        graph.setPhase(QObject::tr("Attributes"));
        int percentComplete = 0;

        for(int rowIndex = 0; rowIndex < tabularData.numRows(); rowIndex++)
        {
            for(int columnIndex = 0; columnIndex < tabularData.numColumns(); columnIndex++)
            {
                if(cancelled())
                {
                    graph.clearPhase();
                    return false;
                }

                int newPercentComplete = ((columnIndex + (rowIndex * tabularData.numColumns())) * 100) /
                        numDataPoints;

                if(newPercentComplete > percentComplete)
                {
                    percentComplete = newPercentComplete;
                    progress(newPercentComplete);
                }

                QString value = tabularData.valueAtQString(columnIndex, rowIndex);
                int dataColumnIndex = columnIndex - dataRect.left();
                int dataRowIndex = rowIndex - dataRect.top();

                if(rowIndex == 0)
                {
                    if(dataColumnIndex < 0)
                    {
                        // Row attribute names
                        _correlationPluginInstance->_rowAttributes.emplace(
                                    value, std::vector<QString>(_correlationPluginInstance->_numRows));
                    }
                    else
                    {
                        // Data column names
                        _correlationPluginInstance->_dataColumnNames.at(dataColumnIndex) = value;
                    }
                }
                else if(dataRowIndex < 0)
                {
                    if(columnIndex == 0)
                    {
                        // Column attribute names
                        _correlationPluginInstance->_columnAttributes.emplace(
                                    value, std::vector<QString>(_correlationPluginInstance->_numColumns));
                    }
                    else if(dataColumnIndex >= 0)
                    {
                        // Column attributes
                        QString columnAttributeName = tabularData.valueAtQString(0, rowIndex);
                        _correlationPluginInstance->_columnAttributes[columnAttributeName].at(dataColumnIndex) = value;
                    }
                }
                else
                {
                    if(dataColumnIndex < 0)
                    {
                        // Row attributes
                        QString rowAttributeName = tabularData.valueAtQString(columnIndex, 0);
                        _correlationPluginInstance->_rowAttributes[rowAttributeName].at(dataRowIndex) = value;
                    }
                    else
                    {
                        // Data
                        int dataIndex = dataColumnIndex + (dataRowIndex * _correlationPluginInstance->_numColumns);

                        if(dataColumnIndex == 0)
                        {
                            CorrelationPluginInstance::Row dataRow;
                            dataRow._begin = _correlationPluginInstance->_data.cbegin() + dataIndex;
                            dataRow._end = _correlationPluginInstance->_data.cbegin() + dataIndex +
                                    _correlationPluginInstance->_numColumns;

                            _correlationPluginInstance->_dataRows.emplace_back(dataRow);
                        }

                        _correlationPluginInstance->_data.at(dataIndex) = value.toDouble();
                    }
                }
            }
        }

        // Calculate sums and mean
        progress(-1);
        graph.setPhase(QObject::tr("Summing"));
        for(auto& row : _correlationPluginInstance->_dataRows)
        {
            for(auto value : row)
            {
                row._sum += value;
                row._sumSq += value * value;
                row._mean += value / _correlationPluginInstance->_numColumns;
            }
        }

        // Calculate variance and stddev
        graph.setPhase(QObject::tr("Standard Deviation"));
        for(auto& row : _correlationPluginInstance->_dataRows)
        {
            double sum = 0.0;
            for(auto value : row)
            {
                double x = (value - row._mean);
                x *= x;
                sum += x;
            }

            row._variance = sum / _correlationPluginInstance->_numColumns;
            row._stddev = std::sqrt(row._variance);
        }

        graph.clearPhase();

        return true;
    }

    return false;
}
