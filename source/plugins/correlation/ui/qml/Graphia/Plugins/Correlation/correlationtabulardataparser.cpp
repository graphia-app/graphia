/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "correlationtabulardataparser.h"

#include "plugins/correlation/correlation.h"
#include "plugins/correlation/correlationplugin.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/loading/graphsizeestimate.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"

#include "shared/utils/container.h"
#include "shared/utils/container_randomsample.h"
#include "shared/utils/string.h"
#include "shared/utils/scope_exit.h"

#include <QRect>

#include <vector>
#include <set>
#include <utility>
#include <limits>
#include <span>

using namespace Qt::Literals::StringLiterals;

static bool dataRectHasDiscreteValues(const TabularData& tabularData, const QRect& dataRect, Progressable* progressable = nullptr)
{
    for(auto column = dataRect.left(); column <= dataRect.right(); column++)
    {
        for(auto row = dataRect.top(); row <= dataRect.bottom(); row++)
        {
            const auto& value = tabularData.valueAt(
                static_cast<size_t>(column), static_cast<size_t>(row));

            if(!value.isEmpty() && !u::isNumeric(value))
                return true;
        }

        if(progressable != nullptr)
        {
            auto columnIndex = column - dataRect.left();
            progressable->setProgress((columnIndex * 100) / dataRect.width());
        }
    }

    return false;
}

static bool dataRectHasMissingValues(const TabularData& tabularData, const QRect& dataRect, Progressable* progressable = nullptr)
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

        if(progressable != nullptr)
        {
            auto columnIndex = column - dataRect.left();
            progressable->setProgress((columnIndex * 100) / dataRect.width());
        }
    }

    return false;
}

// This is designed to detect the case where a numerical dataRect contains continuous values
// It's all a bit finger in the air, but should hopefully catch the obvious cases
static bool dataRectAppearsToBeContinuous(const TabularData& tabularData, const QRect& dataRect, Progressable* progressable = nullptr)
{
    size_t numProbablyDiscreteColumns = 0;

    for(auto column = dataRect.left(); column <= dataRect.right(); column++)
    {
        std::set<QString> uniqueValues;

        for(auto row = dataRect.top(); row <= dataRect.bottom(); row++)
        {
            const auto& value = tabularData.valueAt(
                static_cast<size_t>(column), static_cast<size_t>(row));

            if(value.isEmpty())
                continue;

            // We should only be getting called on numeric dataRects
            Q_ASSERT(u::isNumeric(value));

            // If the dataRect contains at least one floating point value,
            // assume it's continuous data
            if(!u::isInteger(value))
                return true;

            uniqueValues.emplace(value);
        }

        auto percentOfValuesInColumnThatAreUnique =
            (uniqueValues.size() * 100) / static_cast<size_t>(dataRect.height());

        // If we have few values relative to the size of the column,
        // assume it's discrete data
        if(percentOfValuesInColumnThatAreUnique < 50)
            numProbablyDiscreteColumns++;

        if(progressable != nullptr)
        {
            auto columnIndex = column - dataRect.left();
            progressable->setProgress((columnIndex * 100) / dataRect.width());
        }
    }

    return numProbablyDiscreteColumns == 0;
}

static std::pair<double, double> dataRectMinMax(const TabularData& tabularData, const QRect& dataRect, Progressable* progressable = nullptr)
{
    auto min = std::numeric_limits<double>::max();
    auto max = std::numeric_limits<double>::lowest();

    for(auto column = dataRect.left(); column <= dataRect.right(); column++)
    {
        for(auto row = dataRect.top(); row <= dataRect.bottom(); row++)
        {
            const auto& value = tabularData.valueAt(
                static_cast<size_t>(column), static_cast<size_t>(row));

            if(value.isEmpty())
                continue;

            // We should only be getting called on numeric dataRects
            Q_ASSERT(u::isNumeric(value));

            auto number = u::toNumber(value);

            min = std::min(number, min);
            max = std::max(number, max);
        }

        if(progressable != nullptr)
        {
            auto columnIndex = column - dataRect.left();
            progressable->setProgress((columnIndex * 100) / dataRect.width());
        }
    }

    return {min, max};
}

template<typename ParseFn>
static bool parseUsing(const QString& fileType, const ParseFn& parseFn)
{
    if(fileType == u"CorrelationCSV"_s)
        return parseFn(CsvFileParser());

    if(fileType == u"CorrelationTSV"_s)
        return parseFn(TsvFileParser());

    if(fileType == u"CorrelationSSV"_s)
        return parseFn(SsvFileParser());

    if(fileType == u"CorrelationXLSX"_s)
        return parseFn(XlsxTabularDataParser());

    return false;
}

bool CorrelationTabularDataParser::transposed() const
{
    return _model.transposed();
}

void CorrelationTabularDataParser::setTransposed(bool transposed)
{
    // If we manage to get here while the rect detection is still running,
    // wait until it has finished before proceeding
    if(_dataRectangleFutureWatcher.isRunning())
        _dataRectangleFutureWatcher.waitForFinished();

    if(_graphSizeEstimateFutureWatcher.isRunning())
    {
        _graphSizeEstimateCancellable.cancel();
        _graphSizeEstimateFutureWatcher.waitForFinished();
    }

    if(this->transposed() != transposed)
    {
        auto transposedDataRect = _dataRect.transposed();
        transposedDataRect.moveLeft(_dataRect.y());
        transposedDataRect.moveTop(_dataRect.x());
        _dataRect = transposedDataRect;
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

QVariantMap CorrelationTabularDataParser::dataRect() const
{
    QVariantMap m;

    m.insert(u"x"_s, _dataRect.x());
    m.insert(u"y"_s, _dataRect.y());
    m.insert(u"width"_s, _dataRect.width());
    m.insert(u"height"_s, _dataRect.height());
    m.insert(u"asQRect"_s, _dataRect);

    m.insert(u"hasMissingValues"_s, _hasMissingValues);
    m.insert(u"hasDiscreteValues"_s, _hasDiscreteValues);
    m.insert(u"appearsToBeContinuous"_s, _appearsToBeContinuous);

    m.insert(u"minValue"_s, _numericalMinMax.first);
    m.insert(u"maxValue"_s, _numericalMinMax.second);

    return m;
}

CorrelationTabularDataParser::CorrelationTabularDataParser()
{
    connect(&_dataRectangleFutureWatcher, &QFutureWatcher<void>::started, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataRectangleFutureWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataRectangleFutureWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::dataRectChanged);
    connect(&_dataRectangleFutureWatcher, &QFutureWatcher<void>::finished, [this]
    {
        // An estimate was started while the data rectangle was being calculated
        if(!_graphSizeEstimateQueuedParameters.isEmpty())
            estimateGraphSize(_graphSizeEstimateQueuedParameters);
    });

    connect(&_dataParserWatcher, &QFutureWatcher<void>::started, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::busyChanged);
    connect(&_dataParserWatcher, &QFutureWatcher<void>::finished, this, &CorrelationTabularDataParser::onDataLoaded);

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::started,
        this, &CorrelationTabularDataParser::graphSizeEstimateInProgressChanged);
    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished,
        this, &CorrelationTabularDataParser::graphSizeEstimateInProgressChanged);

    connect(&_graphSizeEstimateFutureWatcher, &QFutureWatcher<QVariantMap>::finished, [this]
    {
        _graphSizeEstimate = _graphSizeEstimateFutureWatcher.result();
        emit graphSizeEstimateChanged();

        // Another estimate was queued while we were busy
        if(!_graphSizeEstimateQueuedParameters.isEmpty())
            estimateGraphSize(_graphSizeEstimateQueuedParameters);
    });
}

CorrelationTabularDataParser::~CorrelationTabularDataParser()
{
    _graphSizeEstimateFutureWatcher.waitForFinished();
    _dataRectangleFutureWatcher.waitForFinished();
    _dataParserWatcher.waitForFinished();
}

bool CorrelationTabularDataParser::parse(const QUrl& fileUrl, const QString& fileType)
{
    const QFuture<void> future = QtConcurrent::run([this, fileUrl, fileType]()
    {
        if(fileUrl.isEmpty() || fileType.isEmpty())
            return;

        parseUsing(fileType, [fileUrl, this](auto&& parser)
        {
            _cancellableParser = &parser;
            auto atExit = std::experimental::make_scope_exit([this] { _cancellableParser = nullptr; });

            parser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!parser.parse(fileUrl))
                return false;

            _dataPtr = std::make_shared<TabularData>(std::move(parser.tabularData()));

            _dataHasNumericalRect = !_dataPtr->findLargestNumericalDataRect(this).isEmpty();
            emit dataHasNumericalRectChanged();

            return true;
        });

        setProgress(-1);
    });

    _dataParserWatcher.setFuture(future);
    return true;
}

void CorrelationTabularDataParser::cancelParse()
{
    if(_cancellableParser != nullptr)
        _cancellableParser.load()->cancel();

    cancel();
}

void CorrelationTabularDataParser::waitForDataRectangleFuture()
{
    if(_dataRectangleFutureWatcher.isRunning())
    {
        qDebug() << "CorrelationTabularDataParser QFutureWatcher "
            "still running while attempting more processing";
        _dataRectangleFutureWatcher.waitForFinished();
    }
}

void CorrelationTabularDataParser::autoDetectDataRectangle()
{
    Q_ASSERT(_dataPtr != nullptr);
    if(_dataPtr == nullptr)
        return;

    waitForDataRectangleFuture();

    const QFuture<void> future = QtConcurrent::run([this]()
    {
        auto numericalDataRect = _dataPtr->findLargestNumericalDataRect(this);

        if(numericalDataRect.isEmpty())
        {
            // A numerical rectangle can't be found and there isn't an existing
            // selected rectangle, so search for a non-numerical rectangle instead
            if(_dataRect.isEmpty())
                _dataRect = _dataPtr->findLargestNonNumericalDataRect();

            _hasDiscreteValues = true;
            _appearsToBeContinuous = false;
        }
        else
        {
            _dataRect = numericalDataRect;
            _hasDiscreteValues = false;
            _appearsToBeContinuous = dataRectAppearsToBeContinuous(*_dataPtr, _dataRect, this);
            _numericalMinMax = dataRectMinMax(*_dataPtr, _dataRect, this);
        }

        _hasMissingValues = dataRectHasMissingValues(*_dataPtr, _dataRect, this);

        setProgress(-1);
    });
    _dataRectangleFutureWatcher.setFuture(future);
}

void CorrelationTabularDataParser::setDataRectangle(size_t column, size_t row)
{
    Q_ASSERT(_dataPtr != nullptr);
    if(_dataPtr == nullptr)
        return;

    waitForDataRectangleFuture();

    column = std::max(size_t{1}, column);

    const QFuture<void> future = QtConcurrent::run([column, row, this]()
    {
        _dataRect =
        {
            static_cast<int>(column), static_cast<int>(row),
            static_cast<int>(_dataPtr->numColumns() - column),
            static_cast<int>(_dataPtr->numRows() - row)
        };

        _hasDiscreteValues = dataRectHasDiscreteValues(*_dataPtr, _dataRect, this);

        if(!_hasDiscreteValues)
        {
            _appearsToBeContinuous = dataRectAppearsToBeContinuous(*_dataPtr, _dataRect, this);
            _numericalMinMax = dataRectMinMax(*_dataPtr, _dataRect, this);
        }
        else
        {
            _appearsToBeContinuous = false;
            _numericalMinMax = {};
        }

        _hasMissingValues = dataRectHasMissingValues(*_dataPtr, _dataRect, this);

        setProgress(-1);
    });
    _dataRectangleFutureWatcher.setFuture(future);
}

void CorrelationTabularDataParser::clearData()
{
    if(_dataPtr != nullptr)
        _dataPtr->reset();
}

static std::vector<size_t> randomRowIndices(size_t first, size_t numRows, size_t numSamples)
{
    std::vector<size_t> rowIndices(numRows - first);
    std::iota(rowIndices.begin(), rowIndices.end(), first);
    rowIndices = u::randomSample(rowIndices, numSamples);
    std::sort(rowIndices.begin(), rowIndices.end());

    return rowIndices;
}

ContinuousDataVectors CorrelationTabularDataParser::sampledContinuousDataRows(
    size_t numSampleRows, const QVariantMap& parameters)
{
    if(_dataRect.isEmpty())
        return {};

    auto missingDataType = normaliseQmlEnum<MissingDataType>(parameters[u"missingDataType"_s].toInt());
    auto replacementValue = parameters[u"missingDataValue"_s].toDouble();
    auto scalingType = normaliseQmlEnum<ScalingType>(parameters[u"scaling"_s].toInt());
    auto normaliseType = normaliseQmlEnum<NormaliseType>(parameters[u"normalise"_s].toInt());
    auto clippingType = normaliseQmlEnum<ClippingType>(parameters[u"clippingType"_s].toInt());
    auto clippingValue = parameters[u"clippingValue"_s].toDouble();

    ContinuousDataVectors dataRows;
    std::vector<double> rowData;

    auto rowIndices = randomRowIndices(static_cast<size_t>(_dataRect.y()), _dataPtr->numRows(), numSampleRows);
    rowData.reserve(rowIndices.size() * static_cast<size_t>(_dataRect.width()));

    for(const size_t rowIndex : rowIndices)
    {
        auto startColumn = static_cast<size_t>(_dataRect.x());
        auto finishColumn = startColumn + static_cast<size_t>(_dataRect.width());
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
                    qDebug() << u"WARNING: non-numeric value at (%1, %2): %3"_s
                        .arg(columnIndex).arg(rowIndex).arg(value);
                }
            }
            else
            {
                transformedValue = CorrelationFileParser::imputeValue(
                    missingDataType, replacementValue,
                    *_dataPtr, _dataRect, columnIndex, rowIndex);
            }

            rowData.emplace_back(transformedValue);
        }
    }

    CorrelationFileParser::clipValues(clippingType, clippingValue,
        static_cast<size_t>(_dataRect.width()), rowData);

    auto epsilon = CorrelationFileParser::epsilonFor(rowData);
    std::transform(rowData.begin(), rowData.end(), rowData.begin(),
    [scalingType, epsilon](double value)
    {
        return CorrelationFileParser::scaleValue(scalingType, value, epsilon);
    });

    NodeId nodeId(0);

    for(size_t row = 0; row < rowIndices.size(); row++)
    {
        dataRows.emplace_back(rowData, row, static_cast<size_t>(_dataRect.width()), nodeId);
        ++nodeId;
    }

    CorrelationFileParser::normalise(normaliseType, dataRows);

    for(auto& dataRow : dataRows)
        dataRow.update();

    return dataRows;
}

DiscreteDataVectors CorrelationTabularDataParser::sampledDiscreteDataRows(
    size_t numSampleRows, const QVariantMap&)
{
    if(_dataRect.isEmpty())
        return {};

    DiscreteDataVectors dataRows;
    std::vector<QString> rowData;
    rowData.reserve(_dataPtr->numColumns() - static_cast<size_t>(_dataRect.x()));

    NodeId nodeId(0);

    auto rowIndices = randomRowIndices(static_cast<size_t>(_dataRect.y()), _dataPtr->numRows(), numSampleRows);
    for(const size_t rowIndex : rowIndices)
    {
        rowData.clear();

        auto startColumn = static_cast<size_t>(_dataRect.x());
        auto finishColumn = startColumn + static_cast<size_t>(_dataRect.width());
        for(auto columnIndex = startColumn; columnIndex < finishColumn; columnIndex++)
        {
            if(_graphSizeEstimateCancellable.cancelled())
                return {};

            const auto& value = _dataPtr->valueAt(columnIndex, rowIndex);
            rowData.emplace_back(value);
        }

        dataRows.emplace_back(rowData, nodeId);
        ++nodeId;
    }

    for(auto& dataRow : dataRows)
        dataRow.update();

    return dataRows;
}

void CorrelationTabularDataParser::estimateGraphSize(const QVariantMap& parameters)
{
    Q_ASSERT(!parameters.isEmpty());

    if(_dataPtr == nullptr)
        return;

    if(_graphSizeEstimateFutureWatcher.isRunning() || _dataRectangleFutureWatcher.isRunning())
    {
        _graphSizeEstimateQueuedParameters = parameters;
        return;
    }

    const QFuture<QVariantMap> future = QtConcurrent::run([this, parameters]
    {
        Q_ASSERT(!parameters.isEmpty());

        auto maximumK = static_cast<size_t>(parameters[u"maximumK"_s].toUInt());
        auto correlationFilterType = normaliseQmlEnum<CorrelationFilterType>(parameters[u"correlationFilterType"_s].toInt());
        auto correlationDataType = normaliseQmlEnum<CorrelationDataType>(parameters[u"correlationDataType"_s].toInt());
        auto continuousCorrelationType = normaliseQmlEnum<CorrelationType>(parameters[u"continuousCorrelationType"_s].toInt());
        auto discreteCorrelationType = normaliseQmlEnum<CorrelationType>(parameters[u"discreteCorrelationType"_s].toInt());

        if(_dataPtr->numRows() == 0)
            return QVariantMap();

        Q_ASSERT(static_cast<size_t>(_dataRect.x() + _dataRect.width()) <= _dataPtr->numColumns());
        Q_ASSERT(static_cast<size_t>(_dataRect.y() + _dataRect.height()) <= _dataPtr->numRows());

        const size_t maxSampleRows = 1400;
        const auto numSampleRows = std::min(maxSampleRows, _dataPtr->numRows());
        EdgeList sampleEdges;

        switch(correlationDataType)
        {
        default:
        case CorrelationDataType::Continuous:
        {
            auto correlation = ContinuousCorrelation::create(continuousCorrelationType, correlationFilterType);
            auto dataRows = sampledContinuousDataRows(numSampleRows, parameters);

            if(correlation == nullptr || dataRows.empty())
                return QVariantMap();

            sampleEdges = correlation->edgeList(dataRows, parameters, &_graphSizeEstimateCancellable);
            break;
        }

        case CorrelationDataType::Discrete:
        {
            auto correlation = DiscreteCorrelation::create(discreteCorrelationType, correlationFilterType);
            auto dataRows = sampledDiscreteDataRows(numSampleRows, parameters);

            if(correlation == nullptr || dataRows.empty())
                return QVariantMap();

            sampleEdges = correlation->edgeList(dataRows, parameters, &_graphSizeEstimateCancellable);
            break;
        }
        }

        switch(correlationFilterType)
        {
        default:
        case CorrelationFilterType::Threshold:
            return graphSizeEstimateThreshold(sampleEdges, numSampleRows, _dataPtr->numRows());
        case CorrelationFilterType::Knn:
            return graphSizeEstimateKnn(sampleEdges, maximumK, numSampleRows, _dataPtr->numRows());
        }
    });

    _graphSizeEstimateQueuedParameters.clear();
    _graphSizeEstimateCancellable.uncancel();

    _graphSizeEstimateFutureWatcher.setFuture(future);
}

void CorrelationTabularDataParser::onDataLoaded()
{
    if(_dataPtr != nullptr)
    {
        _model.setTabularData(*_dataPtr);
        emit dataLoaded();
    }
    else
    {
        _failed = true;
        emit failedChanged();
    }

    _complete = true;
    emit completeChanged();
}

QAbstractTableModel* CorrelationTabularDataParser::tableModel()
{
    return &_model;
}
