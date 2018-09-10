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
                                             TabularData& tabularData, QRect dataRect) :
    _plugin(plugin), _urlTypeName(std::move(urlTypeName)),
    _tabularData(std::move(tabularData)), _dataRect(dataRect)
{}

static QRect findLargestDataRect(const TabularData& tabularData, size_t startColumn = 0, size_t startRow = 0)
{
    std::vector<int> heightHistogram(tabularData.numColumns());

    for(size_t column = startColumn; column < tabularData.numColumns(); column++)
    {
        for(size_t row = tabularData.numRows(); row-- > startRow; )
        {
            auto& value = tabularData.valueAt(column, row);
            if(u::isNumeric(value) || value.isEmpty())
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

    // Enforce having at least one name/attribute row/column
    if(dataRect.width() >= 2 && dataRect.left() == 0)
        dataRect.setLeft(1);

    if(dataRect.height() >= 2 && dataRect.top() == 0)
        dataRect.setTop(1);

    return dataRect;
}

bool CorrelationFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    if(_tabularData.empty() || cancelled())
        return false;

    _tabularData.setTransposed(_plugin->transpose());

    // May be set by parameters
    if(_dataRect.isEmpty())
    {
        graphModel->mutableGraph().setPhase(QObject::tr("Finding Data Points"));
        setProgress(-1);
        _dataRect = findLargestDataRect(_tabularData);
    }

    if(_dataRect.isEmpty() || cancelled())
        return false;

    _plugin->setDimensions(_dataRect.width(), _dataRect.height());

    graphModel->mutableGraph().setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadUserData(_tabularData, _dataRect.left(), _dataRect.top(), *this))
        return false;

    // We don't need this any more, so free up any memory it's consuming
    _tabularData.reset();

    if(_plugin->requiresNormalisation())
    {
        graphModel->mutableGraph().setPhase(QObject::tr("Normalisation"));
        if(!_plugin->normalise(*this))
            return false;
    }

    setProgress(-1);

    _plugin->finishDataRows();
    _plugin->createAttributes();

    graphModel->mutableGraph().setPhase(QObject::tr("Pearson Correlation"));
    auto edges = _plugin->pearsonCorrelation(url.fileName(),
        _plugin->minimumCorrelation(), *this);

    if(cancelled())
        return false;

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    graphModel->mutableGraph().clearPhase();

    return true;
}

bool CorrelationPreParser::transposed() const
{
    return _model.transposed();
}

void CorrelationPreParser::setTransposed(bool transposed)
{
    _model.setTransposed(transposed);
}

void CorrelationPreParser::setProgress(int progress)
{
    if(progress != _progress)
    {
        _progress = progress;
        emit progressChanged();
    }
}

CorrelationPreParser::CorrelationPreParser()
{
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::started, this, &CorrelationPreParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::dataRectChanged);

    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &CorrelationPreParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::onDataLoaded);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationPreParser::dataLoaded);
}

bool CorrelationPreParser::parse(const QUrl& fileUrl, const QString& fileType)
{
    QFuture<void> future = QtConcurrent::run([this, fileUrl, fileType]()
    {
        if(fileUrl.isEmpty() || fileType.isEmpty())
            return;

        if(fileType == QLatin1String("CorrelationCSV"))
        {
            CsvFileParser csvFileParser;

            csvFileParser.setProgressFn([this](int progress)
            {
                setProgress(progress);
            });

            if(!csvFileParser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(csvFileParser.tabularData()));
        }
        else if(fileType == QLatin1String("CorrelationTSV"))
        {
            TsvFileParser tsvFileParser;

            tsvFileParser.setProgressFn([this](int progress)
            {
                setProgress(progress);
            });

            if(!tsvFileParser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(tsvFileParser.tabularData()));
        }

        setProgress(-1);

        Q_ASSERT(_dataPtr != nullptr);
        if(_dataPtr != nullptr)
            _dataRect = findLargestDataRect(*_dataPtr);
    });
    _dataParserWatcher.setFuture(future);
    return true;
}

void CorrelationPreParser::autoDetectDataRectangle(size_t column, size_t row)
{
    QFuture<void> future = QtConcurrent::run([this, column, row]()
    {
        _dataRect = findLargestDataRect(*_dataPtr, column, row);
    });
    _autoDetectDataRectangleWatcher.setFuture(future);
}

void CorrelationPreParser::clearData()
{
    if(_dataPtr != nullptr)
        _dataPtr->reset();
}

void CorrelationPreParser::onDataLoaded()
{
    if(_dataPtr != nullptr)
        _model.setTabularData(*_dataPtr);

    _complete = true;
    emit completeChanged();
}

DataRectTableModel* CorrelationPreParser::tableModel()
{
    return &_model;
}
