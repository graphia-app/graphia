#include "correlationfileparser.h"

#include "shared/graph/imutablegraph.h"
#include "../correlationplugin.h"

#include "shared/loading/tabulardata.h"

#include "shared/utils/utils.h"

#include <QRect>

#include <vector>
#include <stack>

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin) :
    _plugin(plugin)
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

    if(!csvFileParser.parse(url, graph, progress))
        return false;

    auto& tabularData = csvFileParser.tabularData();

    graph.setPhase(QObject::tr("Finding Data Points"));
    progress(-1);
    auto dataRect = findLargestDataRect(tabularData);

    _plugin->setDimensions(dataRect.width(), dataRect.height());

    graph.setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadAttributes(tabularData, dataRect.left(), dataRect.top(), [this]{ return cancelled(); }, progress))
        return false;

    graph.setPhase(QObject::tr("Pearson Correlation"));
    auto edges = _plugin->pearsonCorrelation(0.7, [this]{ return cancelled(); }, progress);

    if(cancelled())
        return false;

    graph.setPhase(QObject::tr("Building Graph"));
    _plugin->createEdges(edges, progress);

    graph.clearPhase();

    return true;
}
