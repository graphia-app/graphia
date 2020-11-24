/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "correlationfileparser.h"

#include "correlationplugin.h"
#include "featurescaling.h"
#include "quantilenormaliser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"

#include "shared/utils/container.h"
#include "shared/utils/container_randomsample.h"

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
            const auto& value = tabularData.valueAt(column, row);
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

static bool dataRectHasMissingValues(const TabularData& tabularData, const QRect& dataRect)
{
    for(auto column = dataRect.left(); column <= dataRect.right(); column++)
    {
        for(auto row = dataRect.top(); row <= dataRect.bottom(); row++)
        {
            const auto& value = tabularData.valueAt(
                static_cast<size_t>(column), static_cast<size_t>(row));

            if(value.isEmpty())
                return true;
        }
    }

    return false;
}

double CorrelationFileParser::imputeValue(MissingDataType missingDataType,
    double replacementValue, const TabularData& tabularData,
    const QRect& dataRect, size_t columnIndex, size_t rowIndex)
{
    double imputedValue = 0.0;

    size_t left = dataRect.x();
    size_t right = dataRect.x() + dataRect.width();

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
        for(size_t avgRowIndex = left; avgRowIndex < right; avgRowIndex++)
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
        for(size_t rightColumn = columnIndex; rightColumn < right; rightColumn++)
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
        for(size_t leftColumn = columnIndex; leftColumn-- != left;)
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
    case NormaliseType::Mean:
    {
        MeanNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Standarisation:
    {
        StandardisationNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::UnitScaling:
    {
        UnitScalingNormaliser normaliser;
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
    if(!_plugin->loadUserData(_tabularData, _dataRect, *this))
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

    graphModel->mutableGraph().setPhase(QObject::tr("Correlation"));
    auto edges = _plugin->correlation(_plugin->minimumCorrelation(), *this);

    if(cancelled())
        return false;

    _plugin->createAttributes();

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    graphModel->mutableGraph().clearPhase();

    return true;
}

bool CorrelationTabularDataParser::transposed() const
{
    return _model.transposed();
}

void CorrelationTabularDataParser::setTransposed(bool transposed)
{
    // If we manage to get here while the rect detection is still running,
    // wait until it has finished before proceeding
    if(_autoDetectDataRectangleWatcher.isRunning())
        _autoDetectDataRectangleWatcher.waitForFinished();

    if(_graphSizeEstimateFutureWatcher.isRunning())
    {
        _graphSizeEstimateCancellable.cancel();
        _graphSizeEstimateFutureWatcher.waitForFinished();
    }

    if(this->transposed() != transposed)
    {
        auto transDataRect = _dataRect.transposed();
        transDataRect.moveLeft(_dataRect.y());
        transDataRect.moveTop(_dataRect.x());
        _dataRect = transDataRect;
    }

    _model.setTransposed(transposed);
    emit transposedChanged();
    emit dataRectChanged();
}

void CorrelationTabularDataParser::setProgress(int progress)
{
    if(progress != _progress)
    {
        _progress = progress;
        emit progressChanged();
    }
}

CorrelationTabularDataParser::CorrelationTabularDataParser()
{
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::started, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::dataRectChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::hasMissingValuesChanged);
    connect(&_autoDetectDataRectangleWatcher, &QFutureWatcher<void>::finished, [this]
    {
        // An estimate was started while the data rectangle was being calculated
        if(_graphSizeEstimateQueued)
            estimateGraphSize();
    });

    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::onDataLoaded);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::dataLoaded);

    connect(this, &CorrelationTabularDataParser::dataRectChanged, this, [this] { estimateGraphSize(); });
    connect(this, &CorrelationTabularDataParser::parameterChanged, this, [this] { estimateGraphSize(); });

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::started,
        this, &CorrelationTabularDataParser::graphSizeEstimateInProgressChanged);
    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished,
        this, &CorrelationTabularDataParser::graphSizeEstimateInProgressChanged);

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished, [this]
    {
        const auto& graphSizeEstimate = _graphSizeEstimateFutureWatcher.result();

        if(!graphSizeEstimate.empty())
        {
            _graphSizeEstimate = graphSizeEstimate;
            emit graphSizeEstimateChanged();
        }

        // Another estimate was queued while we were busy
        if(_graphSizeEstimateQueued)
            estimateGraphSize();
    });
}

CorrelationTabularDataParser::~CorrelationTabularDataParser()
{
    _graphSizeEstimateFutureWatcher.waitForFinished();
    _autoDetectDataRectangleWatcher.waitForFinished();
    _dataParserWatcher.waitForFinished();
}

bool CorrelationTabularDataParser::parse(const QUrl& fileUrl, const QString& fileType)
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
        else if(fileType == QLatin1String("CorrelationXLSX"))
        {
            XlsxTabularDataParser xlsxFileParser;
            xlsxFileParser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!xlsxFileParser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(xlsxFileParser.tabularData()));
        }

        setProgress(-1);
    });

    _dataParserWatcher.setFuture(future);
    return true;
}

void CorrelationTabularDataParser::autoDetectDataRectangle(size_t column, size_t row)
{
    QFuture<void> future = QtConcurrent::run([this, column, row]()
    {
        Q_ASSERT(_dataPtr != nullptr);
        if(_dataPtr != nullptr)
        {
            _dataRect = findLargestDataRect(*_dataPtr, column, row);
            _hasMissingValues = dataRectHasMissingValues(*_dataPtr, _dataRect);
        }
    });
    _autoDetectDataRectangleWatcher.setFuture(future);
}

void CorrelationTabularDataParser::clearData()
{
    if(_dataPtr != nullptr)
        _dataPtr->reset();
}

std::vector<CorrelationDataRow> CorrelationTabularDataParser::sampledDataRows(size_t numSamples)
{
    if(_dataRect.isEmpty())
        return {};

    std::vector<CorrelationDataRow> dataRows;

    Q_ASSERT(static_cast<size_t>(_dataRect.x() + _dataRect.width() - 1) < _dataPtr->numColumns());
    Q_ASSERT(static_cast<size_t>(_dataRect.y() + _dataRect.height() - 1) < _dataPtr->numRows());

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

        auto startColumn = static_cast<size_t>(_dataRect.x());
        auto finishColumn = startColumn + _dataRect.width();
        for(auto columnIndex = startColumn; columnIndex < finishColumn; columnIndex++)
        {
            if(_graphSizeEstimateCancellable.cancelled())
                return {};

            const auto& value = _dataPtr->valueAt(columnIndex, rowIndex);
            double transformedValue = 0.0;

            if(!value.isEmpty())
            {
                bool success = false;
                transformedValue = value.toDouble(&success);

                if(!success)
                {
                    qDebug() << QStringLiteral("WARNING: non-numeric value at (%1, %2): %3")
                        .arg(columnIndex).arg(rowIndex).arg(value);
                }
            }
            else
            {
                transformedValue = CorrelationFileParser::imputeValue(
                    static_cast<MissingDataType>(_missingDataType), _replacementValue,
                    *_dataPtr, _dataRect, columnIndex, rowIndex);
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

void CorrelationTabularDataParser::estimateGraphSize()
{
    if(_dataPtr == nullptr)
        return;

    if(_graphSizeEstimateFutureWatcher.isRunning() || _autoDetectDataRectangleWatcher.isRunning())
    {
        _graphSizeEstimateQueued = true;
        return;
    }

    _graphSizeEstimateQueued = false;
    _graphSizeEstimateCancellable.uncancel();

    QFuture<QVariantMap> future = QtConcurrent::run([this]
    {
        if(_dataPtr->numRows() == 0)
            return QVariantMap();

        const size_t maxSampleRows = 1400;
        const auto numSampleRows = std::min(maxSampleRows, _dataPtr->numRows());
        size_t percent = numSampleRows * 100 / _dataPtr->numRows();
        percent = percent < 1 ? 1 : percent;
        auto percentSq = percent * percent;

        auto dataRows = sampledDataRows(numSampleRows);

        if(dataRows.empty())
            return QVariantMap();

        auto correlation = Correlation::create(static_cast<CorrelationType>(_correlationType));
        auto sampleEdges = correlation->process(dataRows, _minimumCorrelation,
            static_cast<CorrelationPolarity>(_correlationPolarity), &_graphSizeEstimateCancellable);

        if(sampleEdges.empty())
            return QVariantMap();

        std::sort(sampleEdges.begin(), sampleEdges.end(),
            [](const auto& a, const auto& b) { return std::abs(a._r) > std::abs(b._r); });

        const auto smallestSampledCorrelation = sampleEdges.back()._r;
        const auto numEstimateSamples = 100;
        const auto sampleQuantum = (1.0 - smallestSampledCorrelation) / (numEstimateSamples - 1);
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

            if(std::abs(sampleEdge._r) <= sampleCutoff)
            {
                keys.append(std::abs(sampleEdge._r));
                auto numNodes = (nonSingletonNodes.size() * 100) / percent;
                auto numEdges = (numSampledEdges * 10000) / percentSq;

                // Cap the estimates at their theoretical maxima, in order
                // to avoid confusing the user with an estimate that's larger
                // than the source dataset
                numNodes = std::min(numNodes, _dataPtr->numRows());
                numEdges = std::min(numEdges, _dataPtr->numRows() * _dataPtr->numRows());

                estimatedNumNodes.append(numNodes);
                estimatedNumEdges.append(numEdges);

                sampleCutoff -= sampleQuantum;
            }
        }

        keys.append(std::abs(sampleEdges.back()._r));
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

QVariantMap CorrelationTabularDataParser::graphSizeEstimate() const
{
    return _graphSizeEstimate;
}

void CorrelationTabularDataParser::onDataLoaded()
{
    if(_dataPtr != nullptr)
        _model.setTabularData(*_dataPtr);

    _complete = true;
    emit completeChanged();
}

DataRectTableModel* CorrelationTabularDataParser::tableModel()
{
    return &_model;
}
