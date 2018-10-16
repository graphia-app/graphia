#include "correlationfileparser.h"

#include "correlationplugin.h"
#include "minmaxnormaliser.h"
#include "quantilenormaliser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/utils/iterator_range.h"
#include "shared/utils/threadpool.h"

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

double CorrelationFileParser::imputeValue(MissingDataType missingDataType,
    double replacementValue, const TabularData& tabularData,
    size_t firstDataColumn, size_t firstDataRow,
    size_t columnIndex, size_t rowIndex)
{
    double imputedValue = 0.0;

    switch(missingDataType)
    {
    case MissingDataType::Constant:
    {
        imputedValue = replacementValue;
        break;
    }
    case MissingDataType::ColumnAverage:
    {
        // Calculate column averages
        double averageValue = 0.0;
        size_t rowCount = 0;
        for(size_t avgRowIndex = firstDataRow; avgRowIndex < tabularData.numRows(); avgRowIndex++)
        {
            const auto& value = tabularData.valueAt(columnIndex, avgRowIndex);
            if(!value.isEmpty())
            {
                averageValue += value.toDouble();
                rowCount++;
            }
        }

        if(rowCount > 0)
            averageValue /= rowCount;

        imputedValue = averageValue;
        break;
    }
    case MissingDataType::RowInterpolation:
    {
        double rightValue = 0.0;
        double leftValue = 0.0;
        size_t leftDistance = 0;
        size_t rightDistance = 0;
        bool rightValueFound = false;
        bool leftValueFound = false;

        // Find right value
        for(size_t rightColumn = columnIndex; rightColumn < tabularData.numColumns(); rightColumn++)
        {
            const auto& value = tabularData.valueAt(rightColumn, rowIndex);
            if(!value.isEmpty())
            {
                rightValue = value.toDouble();
                rightValueFound = true;
                rightDistance = (rightColumn > columnIndex) ? rightColumn - columnIndex : columnIndex - rightColumn;
                break;
            }
        }
        // Find left value
        for(size_t leftColumn = columnIndex; leftColumn-- != firstDataColumn;)
        {
            const auto& value = tabularData.valueAt(leftColumn, rowIndex);
            if(!value.isEmpty())
            {
                leftValue = value.toDouble();
                leftValueFound = true;
                leftDistance = (leftColumn > columnIndex) ? leftColumn - columnIndex : columnIndex - leftColumn;
                break;
            }
        }

        // Lerp the result if possible, otherwise just set to found value
        if(leftValueFound && rightValueFound)
        {
            size_t totalDistance = leftDistance + rightDistance;
            double tween = leftDistance / static_cast<double>(totalDistance);
            // https://devblogs.nvidia.com/lerp-faster-cuda/
            double lerpedValue = std::fma(tween, rightValue, std::fma(-tween, leftValue, leftValue));
            imputedValue = lerpedValue;
        }
        else if(leftValueFound && !rightValueFound)
            imputedValue = leftValue;
        else if(!leftValueFound && rightValueFound)
            imputedValue = rightValue;
        else // Nothing on the row, just zero it
            imputedValue = 0.0;
        break;
    }
    default:
        break;
    }

    return imputedValue;
}

double CorrelationFileParser::scaleValue(ScalingType scalingType, double value)
{
    // LogY(x+c) where c is EPSILON
    // This prevents LogY(0) which is -inf
    // Log2(0+c) = -1057
    // Document this!
    const double EPSILON = std::nextafter(0.0, 1.0);

    switch(scalingType)
    {
    case ScalingType::Log2:
        return std::log2(value + EPSILON);
    case ScalingType::Log10:
        return std::log10(value + EPSILON);
    case ScalingType::AntiLog2:
        return std::pow(2.0, value);
    case ScalingType::AntiLog10:
        return std::pow(10.0, value);
    case ScalingType::ArcSin:
        return std::asin(value);
    default:
        break;
    }
    return value;
}

void CorrelationFileParser::normalise(NormaliseType normaliseType,
    std::vector<CorrelationDataRow>& dataRows, IParser* parser)
{
    switch(normaliseType)
    {
    case NormaliseType::MinMax:
    {
        MinMaxNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Quantile:
    {
        QuantileNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    default:
        break;
    }

    if(normaliseType != NormaliseType::None)
    {
        for(auto& dataRow : dataRows)
            dataRow.update();
    }
}

std::vector<CorrelationEdge> CorrelationFileParser::pearsonCorrelation(
    const std::vector<CorrelationDataRow>& rows,
    double minimumThreshold, IParser* parser)
{
    if(rows.empty())
        return {};

    size_t numColumns = std::distance(rows.front().cbegin(), rows.front().cend());

    if(parser != nullptr)
        parser->setProgress(-1);

    uint64_t totalCost = 0;
    for(const auto& row : rows)
        totalCost += row.computeCostHint();

    std::atomic<uint64_t> cost(0);

    auto results = ThreadPool(QStringLiteral("PearsonCor")).concurrent_for(rows.begin(), rows.end(),
    [&](std::vector<CorrelationDataRow>::const_iterator rowAIt)
    {
        const auto& rowA = *rowAIt;
        std::vector<CorrelationEdge> edges;

        if(parser != nullptr && parser->cancelled())
            return edges;

        for(const auto& rowB : make_iterator_range(rowAIt + 1, rows.end()))
        {
            double productSum = std::inner_product(rowA.cbegin(), rowA.cend(), rowB.cbegin(), 0.0);
            double numerator = (numColumns * productSum) - (rowA.sum() * rowB.sum());
            double denominator = rowA.variability() * rowB.variability();

            double r = numerator / denominator;

            if(std::isfinite(r) && r >= minimumThreshold)
                edges.push_back({rowA.nodeId(), rowB.nodeId(), r});
        }

        cost += rowA.computeCostHint();

        if(parser != nullptr)
            parser->setProgress((cost * 100) / totalCost);

        return edges;
    });

    if(parser != nullptr)
    {
        // Returning the results might take time
        parser->setProgress(-1);
    }

    std::vector<CorrelationEdge> edges;
    edges.reserve(std::distance(results.begin(), results.end()));
    edges.insert(edges.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));

    return edges;
}

bool CorrelationFileParser::parse(const QUrl&, IGraphModel* graphModel)
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

    setProgress(-1);
    _plugin->finishDataRows();

    if(_plugin->requiresNormalisation())
    {
        graphModel->mutableGraph().setPhase(QObject::tr("Normalisation"));
        _plugin->normalise(this);
    }

    if(cancelled())
        return false;

    setProgress(-1);
    _plugin->createAttributes();

    graphModel->mutableGraph().setPhase(QObject::tr("Pearson Correlation"));
    auto edges = _plugin->pearsonCorrelation(_plugin->minimumCorrelation(), *this);

    if(cancelled())
        return false;

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    graphModel->mutableGraph().clearPhase();

    return true;
}

bool TabularDataParser::transposed() const
{
    return _model.transposed();
}

void TabularDataParser::setTransposed(bool transposed)
{
    _model.setTransposed(transposed);
    emit transposedChanged();
}

void TabularDataParser::setProgress(int progress)
{
    if(progress != _progress)
    {
        _progress = progress;
        emit progressChanged();
    }
}

TabularDataParser::TabularDataParser()
{
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::started, this, &TabularDataParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::dataRectChanged);

    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &TabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::onDataLoaded);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &TabularDataParser::dataLoaded);

    connect(this, &TabularDataParser::dataRectChanged, this, [this] { estimateGraphSize(); });
    connect(this, &TabularDataParser::parameterChanged, this, [this] { estimateGraphSize(); });

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::started,
        this, &TabularDataParser::graphSizeEstimateInProgressChanged);
    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished,
        this, &TabularDataParser::graphSizeEstimateInProgressChanged);

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished, [this]
    {
        _graphSizeEstimate = _graphSizeEstimateFutureWatcher.result();
        emit graphSizeEstimateChanged();

        // Another estimate was queued while we were busy
        if(_graphSizeEstimateQueued)
            estimateGraphSize();
    });
}

bool TabularDataParser::parse(const QUrl& fileUrl, const QString& fileType)
{
    QFuture<void> future = QtConcurrent::run([this, fileUrl, fileType]()
    {
        if(fileUrl.isEmpty() || fileType.isEmpty())
            return;

        if(fileType == QLatin1String("CorrelationCSV"))
        {
            CsvFileParser csvFileParser;
            csvFileParser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!csvFileParser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(csvFileParser.tabularData()));
        }
        else if(fileType == QLatin1String("CorrelationTSV"))
        {
            TsvFileParser tsvFileParser;
            tsvFileParser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!tsvFileParser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(tsvFileParser.tabularData()));
        }

        setProgress(-1);
    });

    _dataParserWatcher.setFuture(future);
    return true;
}

void TabularDataParser::autoDetectDataRectangle(size_t column, size_t row)
{
    QFuture<void> future = QtConcurrent::run([this, column, row]()
    {
        Q_ASSERT(_dataPtr != nullptr);
        if(_dataPtr != nullptr)
            _dataRect = findLargestDataRect(*_dataPtr, column, row);
    });
    _autoDetectDataRectangleWatcher.setFuture(future);
}

void TabularDataParser::clearData()
{
    if(_dataPtr != nullptr)
        _dataPtr->reset();
}

std::vector<CorrelationDataRow> TabularDataParser::sampledDataRows(size_t numSamples)
{
    std::vector<CorrelationDataRow> dataRows;

    std::vector<double> rowData;
    rowData.reserve(_dataPtr->numColumns() - _dataRect.x());

    // Choose numSamples random row indices from tabularData
    std::vector<size_t> rowIndices(_dataPtr->numRows() - _dataRect.y());
    std::iota(rowIndices.begin(), rowIndices.end(), _dataRect.y());
    rowIndices = u::randomSample(rowIndices, numSamples);
    std::sort(rowIndices.begin(), rowIndices.end());

    NodeId nodeId(0);

    for(size_t rowIndex : rowIndices)
    {
        rowData.clear();

        for(auto columnIndex = static_cast<size_t>(_dataRect.x());
            columnIndex < _dataPtr->numColumns(); columnIndex++)
        {
            const auto& value = _dataPtr->valueAt(columnIndex, rowIndex);
            double transformedValue = 0.0;

            if(!value.isEmpty())
            {
                bool success = false;
                transformedValue = value.toDouble(&success);
                Q_ASSERT(success);
            }
            else
            {
                transformedValue = CorrelationFileParser::imputeValue(
                    static_cast<MissingDataType>(_missingDataType), _replacementValue,
                    *_dataPtr, _dataRect.x(), _dataRect.y(), columnIndex, rowIndex);
            }

            transformedValue = CorrelationFileParser::scaleValue(
                static_cast<ScalingType>(_scalingType), transformedValue);

            rowData.emplace_back(transformedValue);
        }

        dataRows.emplace_back(rowData, nodeId);
        ++nodeId;
    }

    CorrelationFileParser::normalise(static_cast<NormaliseType>(_normaliseType), dataRows);

    return dataRows;
}

void TabularDataParser::estimateGraphSize()
{
    if(_dataPtr == nullptr)
        return;

    if(_graphSizeEstimateFutureWatcher.isRunning())
    {
        _graphSizeEstimateQueued = true;
        return;
    }

    _graphSizeEstimateQueued = false;

    QFuture<QVariantMap> future = QtConcurrent::run(
    [this]
    {
        const size_t maxSampleRows = 1400;
        const auto numSampleRows = std::min(maxSampleRows, _dataPtr->numRows());
        size_t percent = numSampleRows * 100 / _dataPtr->numRows();
        percent = percent < 1 ? 1 : percent;
        auto percentSq = percent * percent;

        auto dataRows = sampledDataRows(numSampleRows);
        auto sampleEdges = CorrelationFileParser::pearsonCorrelation(dataRows, _minimumCorrelation);

        if(sampleEdges.empty())
            return QVariantMap();

        std::sort(sampleEdges.begin(), sampleEdges.end(),
            [](const auto& a, const auto& b) { return a._r > b._r; });

        const auto numEstimateSamples = 100;
        const auto sampleQuantum = (1.0 - _minimumCorrelation) / (numEstimateSamples - 1);
        auto sampleCutoff = 1.0;

        QVector<double> keys;
        QVector<double> estimatedNumNodes;
        QVector<double> estimatedNumEdges;

        keys.reserve(static_cast<int>(sampleEdges.size()));
        estimatedNumNodes.reserve(static_cast<int>(sampleEdges.size()));
        estimatedNumEdges.reserve(static_cast<int>(sampleEdges.size()));

        size_t numSampledEdges = 0;
        NodeIdSet nonSingletonNodes;

        for(const auto& sampleEdge : sampleEdges)
        {
            nonSingletonNodes.insert(sampleEdge._source);
            nonSingletonNodes.insert(sampleEdge._target);
            numSampledEdges++;

            if(sampleEdge._r <= sampleCutoff)
            {
                keys.append(sampleEdge._r);
                auto numNodes = (nonSingletonNodes.size() * 100) / percent;
                auto numEdges = (numSampledEdges * 10000) / percentSq;
                estimatedNumNodes.append(numNodes);
                estimatedNumEdges.append(numEdges);

                sampleCutoff -= sampleQuantum;
            }
        }

        keys.append(sampleEdges.back()._r);
        auto numNodes = (nonSingletonNodes.size() * 100) / percent;
        auto numEdges = (numSampledEdges * 10000) / percentSq;
        estimatedNumNodes.append(numNodes);
        estimatedNumEdges.append(numEdges);

        std::reverse(keys.begin(), keys.end());
        std::reverse(estimatedNumNodes.begin(), estimatedNumNodes.end());
        std::reverse(estimatedNumEdges.begin(), estimatedNumEdges.end());

        QVariantMap map;
        map.insert(QStringLiteral("keys"), QVariant::fromValue(keys));
        map.insert(QStringLiteral("numNodes"), QVariant::fromValue(estimatedNumNodes));
        map.insert(QStringLiteral("numEdges"), QVariant::fromValue(estimatedNumEdges));
        return map;
    });

    _graphSizeEstimateFutureWatcher.setFuture(future);
}

QVariantMap TabularDataParser::graphSizeEstimate() const
{
    return _graphSizeEstimate;
}

void TabularDataParser::onDataLoaded()
{
    if(_dataPtr != nullptr)
        _model.setTabularData(*_dataPtr);

    _complete = true;
    emit completeChanged();
}

DataRectTableModel* TabularDataParser::tableModel()
{
    return &_model;
}
