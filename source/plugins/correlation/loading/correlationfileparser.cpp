/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
#include "correlation.h"
#include "featurescaling.h"
#include "quantilenormaliser.h"

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
#include <stack>
#include <set>
#include <utility>
#include <limits>
#include <span>

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin, const QString& urlTypeName,
                                             TabularData& tabularData, QRect dataRect) :
    _plugin(plugin), _urlTypeName(urlTypeName),
    _tabularData(std::move(tabularData)), _dataRect(dataRect)
{}

template<typename Fn>
static QRect findLargestDataRect(const TabularData& tabularData, Fn predicate,
    size_t startColumn = 0, size_t startRow = 0)
{
    std::vector<size_t> heightHistogram(tabularData.numColumns());

    for(size_t column = startColumn; column < tabularData.numColumns(); column++)
    {
        for(size_t row = tabularData.numRows(); row-- > startRow; )
        {
            const auto& value = tabularData.valueAt(column, row);
            if(predicate(value) || value.isEmpty())
                heightHistogram.at(column)++;
            else
                break;
        }
    }

    std::stack<size_t> heights;
    std::stack<size_t> indexes;
    QRect dataRect;

    for(size_t index = 0; index < heightHistogram.size(); index++)
    {
        if(heights.empty() || heightHistogram[index] > heights.top())
        {
            heights.push(heightHistogram[index]);
            indexes.push(index);
        }
        else if(heightHistogram[index] < heights.top())
        {
            size_t lastIndex = 0;

            while(!heights.empty() && heightHistogram[index] < heights.top())
            {
                lastIndex = indexes.top(); indexes.pop();
                auto height = heights.top(); heights.pop();
                auto width = (index - lastIndex);
                auto area = width * height;
                if(area > (static_cast<size_t>(dataRect.width()) * static_cast<size_t>(dataRect.height())))
                {
                    dataRect.setLeft(static_cast<int>(lastIndex));
                    dataRect.setTop(static_cast<int>(tabularData.numRows() - height));
                    dataRect.setWidth(static_cast<int>(width));
                    dataRect.setHeight(static_cast<int>(height));
                }
            }

            heights.push(heightHistogram[index]);
            indexes.push(lastIndex);
        }
    }

    while(!heights.empty())
    {
        auto lastIndex = indexes.top(); indexes.pop();
        auto height = heights.top(); heights.pop();
        auto width = heightHistogram.size() - lastIndex;
        auto area = width * height;
        if(area > (static_cast<size_t>(dataRect.width()) * static_cast<size_t>(dataRect.height())))
        {
            dataRect.setLeft(static_cast<int>(lastIndex));
            dataRect.setTop(static_cast<int>(tabularData.numRows() - height));
            dataRect.setWidth(static_cast<int>(width));
            dataRect.setHeight(static_cast<int>(height));
        }
    }

    // Enforce having at least one name/attribute row/column
    if(dataRect.width() >= 2 && dataRect.left() == 0)
        dataRect.setLeft(1);

    if(dataRect.height() >= 2 && dataRect.top() == 0)
        dataRect.setTop(1);

    const int bottomMargin = dataRect.top() - (static_cast<int>(tabularData.numRows()) - dataRect.height());
    const int rightMargin = dataRect.left() - (static_cast<int>(tabularData.numColumns()) - dataRect.width());

    if(bottomMargin != 0 || rightMargin != 0)
        return {};

    return dataRect;
}

static QRect findLargestNumericalDataRect(const TabularData& tabularData,
    size_t startColumn = 0, size_t startRow = 0)
{
    return findLargestDataRect(tabularData,
        [](const auto& value) { return u::isNumeric(value); }, startColumn, startRow);
}

static QRect findLargestNonNumericalDataRect(const TabularData& tabularData,
    size_t startColumn = 0, size_t startRow = 0)
{
    return findLargestDataRect(tabularData,
        [](const auto& value) { return !u::isNumeric(value); }, startColumn, startRow);
}

static bool dataRectHasDiscreteValues(const TabularData& tabularData, const QRect& dataRect)
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
    }

    return false;
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

// This is designed to detect the case where a numerical dataRect contains continuous values
// It's all a bit finger in the air, but should hopefully catch the obvious cases
static bool dataRectAppearsToBeContinuous(const TabularData& tabularData, const QRect& dataRect)
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
    }

    return numProbablyDiscreteColumns == 0;
}

static std::pair<double, double> dataRectMinMax(const TabularData& tabularData, const QRect& dataRect)
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
    }

    return {min, max};
}

double CorrelationFileParser::imputeValue(MissingDataType missingDataType,
    double replacementValue, const TabularData& tabularData,
    const QRect& dataRect, size_t columnIndex, size_t rowIndex)
{
    double imputedValue = 0.0;

    auto left = static_cast<size_t>(dataRect.x());
    auto right = static_cast<size_t>(dataRect.x()) + static_cast<size_t>(dataRect.width());

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
            averageValue /= static_cast<double>(rowCount);

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
            const double tween = static_cast<double>(leftDistance) / static_cast<double>(leftDistance + rightDistance);
            // https://devblogs.nvidia.com/lerp-faster-cuda/
            const double lerpedValue = std::fma(tween, rightValue, std::fma(-tween, leftValue, leftValue));
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

void CorrelationFileParser::clipValues(ClippingType clippingType, double clippingValue,
    size_t width, std::vector<double>& data)
{
    Q_ASSERT(data.size() % width == 0);

    switch(clippingType)
    {
    case ClippingType::Constant:
        for(auto& value : data)
            value = std::min(value, clippingValue);
        break;

    case ClippingType::Winsorization:
    {
        auto it = data.begin();
        while(it != data.end())
        {
            using dt = std::vector<double>::difference_type;
            auto end = it + static_cast<dt>(width);

            //FIXME: pointers are passed to span here because Apple libc++
            // doesn't implement the generalised iterator constructor
            auto sortedIndices = u::sortedIndicesOf(std::span(&*it, &*end));
            auto metaIndex = static_cast<size_t>((clippingValue * static_cast<double>(width - 1)) / 100.0);
            auto clipIndex = sortedIndices.at(metaIndex);
            auto clipValue = *(it + static_cast<dt>(clipIndex));

            auto begin = sortedIndices.begin() + static_cast<dt>(metaIndex + 1);
            for(auto i = begin; i != sortedIndices.end(); i++)
                *(it + static_cast<dt>(*i)) = clipValue;

            it += static_cast<dt>(width);
        }

        break;
    }

    default:
        break;
    }
}

double CorrelationFileParser::scaleValue(ScalingType scalingType, double value, double epsilon)
{
    switch(scalingType)
    {
    case ScalingType::Log2:
        return std::log2(value + epsilon);
    case ScalingType::Log10:
        return std::log10(value + epsilon);
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
    ContinuousDataVectors& dataRows, IParser* parser)
{
    switch(normaliseType)
    {
    case NormaliseType::MinMax:
    {
        const MinMaxNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Mean:
    {
        const MeanNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Standarisation:
    {
        const StandardisationNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::UnitScaling:
    {
        const UnitScalingNormaliser normaliser;
        normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Quantile:
    {
        const QuantileNormaliser normaliser;
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

double CorrelationFileParser::epsilonFor(const std::vector<double>& data)
{
    if(data.empty())
        return std::nextafter(0.0, 1.0);

    double minValue = std::numeric_limits<double>::max();
    for(const auto& value : data)
    {
        if(value > 0.0 && value < minValue)
            minValue = value;
    }

    // This is picked to be appropriately small for log scaling
    // data, but not so small that it obscures the values themselves
    return minValue * 0.5;
}

bool CorrelationFileParser::parse(const QUrl&, IGraphModel* graphModel)
{
    if(_tabularData.empty() || cancelled())
        return false;

    _tabularData.setTransposed(_plugin->transpose());

    if(_dataRect.isEmpty() || cancelled())
        return false;

    size_t numContinuousColumns = 0;
    size_t numDiscreteColumns = 0;

    switch(_plugin->dataType())
    {
    default:
    case CorrelationDataType::Continuous:
        numContinuousColumns = static_cast<size_t>(_dataRect.width());
        break;

    case CorrelationDataType::Discrete:
        numDiscreteColumns = static_cast<size_t>(_dataRect.width());
        break;
    }

    _plugin->setDimensions(numContinuousColumns, numDiscreteColumns, static_cast<size_t>(_dataRect.height()));

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
    auto edges = _plugin->correlation(*this);

    if(cancelled())
        return false;

    _plugin->createAttributes();

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    graphModel->mutableGraph().clearPhase();

    return true;
}

QString CorrelationFileParser::log() const
{
    return _plugin->log();
}

bool CorrelationTabularDataParser::transposed() const
{
    return _model.transposed();
}

// NOLINTNEXTLIME readability-make-member-function-const
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

    m.insert(QStringLiteral("x"), _dataRect.x());
    m.insert(QStringLiteral("y"), _dataRect.y());
    m.insert(QStringLiteral("width"), _dataRect.width());
    m.insert(QStringLiteral("height"), _dataRect.height());
    m.insert(QStringLiteral("asQRect"), _dataRect);

    m.insert(QStringLiteral("hasMissingValues"), _hasMissingValues);
    m.insert(QStringLiteral("hasDiscreteValues"), _hasDiscreteValues);
    m.insert(QStringLiteral("appearsToBeContinuous"), _appearsToBeContinuous);

    m.insert(QStringLiteral("minValue"), _numericalMinMax.first);
    m.insert(QStringLiteral("maxValue"), _numericalMinMax.second);

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

CorrelationTabularDataParser::~CorrelationTabularDataParser() // NOLINT modernize-use-equals-default
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

        auto parseUsing = [fileUrl, this](auto&& parser)
        {
            _cancellableParser = &parser;
            auto atExit = std::experimental::make_scope_exit([this] { _cancellableParser = nullptr; });

            parser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!parser.parse(fileUrl))
                return;

            _dataPtr = std::make_shared<TabularData>(std::move(parser.tabularData()));

            // findLargestNumericalDataRect can take a while for big datasets
            setProgress(-1);

            _dataHasNumericalRect = !findLargestNumericalDataRect(*_dataPtr).isEmpty();
            emit dataHasNumericalRectChanged();
        };

        if(fileType == QStringLiteral("CorrelationCSV"))
            parseUsing(CsvFileParser());
        else if(fileType == QStringLiteral("CorrelationTSV"))
            parseUsing(TsvFileParser());
        else if(fileType == QStringLiteral("CorrelationSSV"))
            parseUsing(SsvFileParser());
        else if(fileType == QStringLiteral("CorrelationXLSX"))
            parseUsing(XlsxTabularDataParser());

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
        auto numericalDataRect = findLargestNumericalDataRect(*_dataPtr);

        if(numericalDataRect.isEmpty())
        {
            // A numerical rectangle can't be found and there isn't an existing
            // selected rectangle, so search for a non-numerical rectangle instead
            if(_dataRect.isEmpty())
                _dataRect = findLargestNonNumericalDataRect(*_dataPtr);

            _hasDiscreteValues = true;
            _appearsToBeContinuous = false;
        }
        else
        {
            _dataRect = numericalDataRect;
            _hasDiscreteValues = false;
            _appearsToBeContinuous = dataRectAppearsToBeContinuous(*_dataPtr, _dataRect);
            _numericalMinMax = dataRectMinMax(*_dataPtr, _dataRect);
        }

        _hasMissingValues = dataRectHasMissingValues(*_dataPtr, _dataRect);
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

        _hasDiscreteValues = dataRectHasDiscreteValues(*_dataPtr, _dataRect);

        if(!_hasDiscreteValues)
        {
            _appearsToBeContinuous = dataRectAppearsToBeContinuous(*_dataPtr, _dataRect);
            _numericalMinMax = dataRectMinMax(*_dataPtr, _dataRect);
        }
        else
        {
            _appearsToBeContinuous = false;
            _numericalMinMax = {};
        }

        _hasMissingValues = dataRectHasMissingValues(*_dataPtr, _dataRect);
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

    auto missingDataType = NORMALISE_QML_ENUM(MissingDataType, parameters[QStringLiteral("missingDataType")].toInt());
    auto replacementValue = parameters[QStringLiteral("missingDataValue")].toDouble();
    auto scalingType = NORMALISE_QML_ENUM(ScalingType, parameters[QStringLiteral("scaling")].toInt());
    auto normaliseType = NORMALISE_QML_ENUM(NormaliseType, parameters[QStringLiteral("normalise")].toInt());
    auto clippingType = NORMALISE_QML_ENUM(ClippingType, parameters[QStringLiteral("clippingType")].toInt());
    auto clippingValue = parameters[QStringLiteral("clippingValue")].toDouble();

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
                    qDebug() << QStringLiteral("WARNING: non-numeric value at (%1, %2): %3")
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

        auto maximumK = static_cast<size_t>(parameters[QStringLiteral("maximumK")].toUInt());
        auto correlationFilterType = NORMALISE_QML_ENUM(CorrelationFilterType, parameters[QStringLiteral("correlationFilterType")].toInt());
        auto correlationDataType = NORMALISE_QML_ENUM(CorrelationDataType, parameters[QStringLiteral("correlationDataType")].toInt());
        auto continuousCorrelationType = NORMALISE_QML_ENUM(CorrelationType, parameters[QStringLiteral("continuousCorrelationType")].toInt());
        auto discreteCorrelationType = NORMALISE_QML_ENUM(CorrelationType, parameters[QStringLiteral("discreteCorrelationType")].toInt());

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
