#include "correlationfileparser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "correlationplugin.h"

#include "shared/loading/tabulardata.h"

#include <QRect>

#include <vector>
#include <stack>
#include <utility>

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName,
                                             QRect dataRect) :
    _plugin(plugin), _urlTypeName(std::move(urlTypeName)), _dataRect(dataRect)
{}

static QRect findLargestDataRect(const TabularData& tabularData, size_t startColumn = 0, size_t startRow = 0)
{
    std::vector<int> heightHistogram(tabularData.numColumns());

    for(size_t column = startColumn; column < tabularData.numColumns(); column++)
    {
        for(size_t row = tabularData.numRows(); row-- > startRow; )
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
                int width = (index - lastIndex);
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
    CsvFileParser csvFileParser(this);
    TsvFileParser tsvFileParser(this);

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

    if(tabularData == nullptr || cancelled())
        return false;

    tabularData->setTransposed(_plugin->transpose());

    graphModel.mutableGraph().setPhase(QObject::tr("Finding Data Points"));
    progressFn(-1);

    // May be set by parameters
    if(_dataRect.isEmpty())
        _dataRect = findLargestDataRect(*tabularData);

    if(_dataRect.isEmpty() || cancelled())
        return false;

    _plugin->setDimensions(_dataRect.width(), _dataRect.height());

    graphModel.mutableGraph().setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadUserData(*tabularData, _dataRect.left(), _dataRect.top(), *this, progressFn))
        return false;

    if(_plugin->requiresNormalisation())
    {
        graphModel.mutableGraph().setPhase(QObject::tr("Normalisation"));
        if(!_plugin->normalise(*this, progressFn))
            return false;
    }

    progressFn(-1);

    _plugin->finishDataRows();
    _plugin->createAttributes();

    graphModel.mutableGraph().setPhase(QObject::tr("Pearson Correlation"));
    auto edges = _plugin->pearsonCorrelation(url.fileName(),
        _plugin->minimumCorrelation(), *this, progressFn);

    if(cancelled())
        return false;

    graphModel.mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this, progressFn))
        return false;

    graphModel.mutableGraph().clearPhase();

    return true;
}

size_t CorrelationPreParser::rowCount()
{
    return _data != nullptr ? _data->numRows() : 0;
}

size_t CorrelationPreParser::columnCount()
{
    return _data != nullptr ? _data->numColumns() : 0;
}

CorrelationPreParser::CorrelationPreParser()
{
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::dataRectChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::started, this, &CorrelationPreParser::isRunningChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::isRunningChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::isRunningChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &CorrelationPreParser::isRunningChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::onDataParsed);
}

bool CorrelationPreParser::parse()
{
    QFuture<void> future = QtConcurrent::run([this]()
    {
        if(_fileType.isEmpty() || _fileUrl.isEmpty())
            return;

        if(_fileType == QLatin1String("CorrelationCSV"))
        {
            if(!_csvFileParser.preParse(_fileUrl))
                return;

            _data = &(_csvFileParser.tabularData());
        }
        else if(_fileType == QLatin1String("CorrelationTSV"))
        {
            if(!_tsvFileParser.preParse(_fileUrl))
                return;

            _data = &(_tsvFileParser.tabularData());
        }

        _dataRect = findLargestDataRect(*_data);
    });
    _dataParserWatcher.setFuture(future);
    return true;
}

void CorrelationPreParser::autoDetectDataRectangle(size_t column, size_t row)
{
    QFuture<void> future = QtConcurrent::run([this, column, row]()
    {
        _dataRect = findLargestDataRect(*_data, column, row);
    });
    _autoDetectDataRectangleWatcher.setFuture(future);
}

QString CorrelationPreParser::dataAt(int column, int row)
{
    return QString::fromStdString(_data->valueAt(column, row));
}

void CorrelationPreParser::onDataParsed()
{
    if(_data != nullptr)
        _model.setTabularData(*_data);
}

DataRectTableModel* CorrelationPreParser::tableModel()
{
    return &_model;
}
