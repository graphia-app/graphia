#include "correlationfileparser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "correlationplugin.h"

#include "shared/loading/tabulardata.h"

#include <QRect>

#include <vector>
#include <stack>
#include <utility>

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName) :
    _plugin(plugin), _urlTypeName(std::move(urlTypeName))
{}

static QRect findLargestDataRect(const TabularData& tabularData)
{
    std::vector<int> heightHistogram(tabularData.numColumns());

    for(size_t column = 0; column < tabularData.numColumns(); column++)
    {
        for(size_t row = tabularData.numRows() - 1; row > 0; --row)
        {
            auto& value = tabularData.valueAt(column, row);
            if(u::isNumeric(value) || value.empty())
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
                int width = (index - lastIndex) + 1;
                int area = width * height;
                if(area > (dataRect.width() * dataRect.height()))
                {
                    dataRect.setLeft(lastIndex);
                    dataRect.setTop(static_cast<int>(tabularData.numRows()) - height);
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
            dataRect.setTop(static_cast<int>(tabularData.numRows()) - height);
            dataRect.setWidth(width);
            dataRect.setHeight(height);
        }
    }

    return dataRect;
}

bool CorrelationFileParser::parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn)
{
    CsvFileParser csvFileParser;
    TsvFileParser tsvFileParser;

    TabularData* tabularData = nullptr;
    if(_urlTypeName == QLatin1String("CorrelationCSV"))
    {
        if(!csvFileParser.parse(url, graphModel, progressFn))
            return false;

        tabularData = &(csvFileParser.tabularData());
    }
    else if(_urlTypeName == QLatin1String("CorrelationTSV"))
    {
        if(!tsvFileParser.parse(url, graphModel, progressFn))
            return false;

        tabularData = &(tsvFileParser.tabularData());
    }

    if(tabularData == nullptr)
        return false;

    tabularData->setTransposed(_plugin->transpose());

    graphModel.mutableGraph().setPhase(QObject::tr("Finding Data Points"));
    progressFn(-1);
    auto dataRect = findLargestDataRect(*tabularData);

    if(dataRect.isEmpty())
        return false;

    _plugin->setDimensions(dataRect.width(), dataRect.height());

    auto cancelFn = [this]{ return cancelled(); };

    graphModel.mutableGraph().setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadUserData(*tabularData, dataRect.left(), dataRect.top(), cancelFn, progressFn))
        return false;

    if(_plugin->requiresNormalisation())
    {
        graphModel.mutableGraph().setPhase(QObject::tr("Normalisation"));
        if(!_plugin->normalise(cancelFn, progressFn))
            return false;
    }

    progressFn(-1);

    _plugin->finishDataRows();
    _plugin->createAttributes();

    graphModel.mutableGraph().setPhase(QObject::tr("Pearson Correlation"));
    auto edges = _plugin->pearsonCorrelation(_plugin->minimumCorrelation(), cancelFn, progressFn);

    if(cancelled())
        return false;

    graphModel.mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, cancelFn, progressFn))
        return false;

    graphModel.mutableGraph().clearPhase();

    return true;
}
