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

#include "correlationfileparser.h"

#include "correlation.h"
#include "correlationplugin.h"
#include "featurescaling.h"
#include "quantilenormaliser.h"
#include "softmaxnormaliser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"

#include "shared/utils/container.h"
#include "shared/utils/string.h"

#include <QRect>

#include <vector>
#include <set>
#include <utility>
#include <limits>
#include <span>

using namespace Qt::Literals::StringLiterals;

CorrelationFileParser::CorrelationFileParser(CorrelationPluginInstance* plugin, const QString& urlTypeName,
                                             TabularData& tabularData, QRect dataRect) :
    _plugin(plugin), _urlTypeName(urlTypeName),
    _tabularData(std::move(tabularData)), _dataRect(dataRect)
{}

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
    case NormaliseType::Softmax:
    {
        const SoftmaxNormaliser normaliser;
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

bool CorrelationFileParser::parse(const QUrl& fileUrl, IGraphModel*)
{
    if(_tabularData.empty())
    {
        const bool success = parseUsing(_urlTypeName, [this, &fileUrl](auto&& parser)
        {
            parser.setProgressFn([this](int progress) { setProgress(progress); });

            if(!parser.parse(fileUrl))
            {
                setFailureReason(parser.failureReason());
                return false;
            }

            _tabularData = std::move(parser.tabularData());

            return true;
        });

        if(!success)
            return false;
    }

    if(_tabularData.empty() || cancelled())
        return false;

    _tabularData.setTransposed(_plugin->transpose());

    if(_dataRect.isEmpty())
    {
        setProgress(-1);
        _dataRect = _tabularData.findLargestNumericalDataRect(this);

        if(_dataRect.isEmpty())
            _dataRect = _tabularData.findLargestNonNumericalDataRect(this);
    }

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

    setPhase(QObject::tr("Attributes"));
    if(!_plugin->loadUserData(_tabularData, _dataRect, *this))
        return false;

    // We don't need this any more, so free up any memory it's consuming
    _tabularData.reset();

    setProgress(-1);
    _plugin->finishDataRows();

    if(_plugin->requiresNormalisation())
    {
        setPhase(QObject::tr("Normalisation"));
        _plugin->normalise(this);
    }

    if(cancelled())
        return false;

    setProgress(-1);

    setPhase(QObject::tr("Correlation"));
    auto edges = _plugin->correlation(*this);

    if(cancelled())
        return false;

    _plugin->createAttributes();

    setPhase(QObject::tr("Building Graph"));
    if(!_plugin->createEdges(edges, *this))
        return false;

    clearPhase();

    return true;
}

QString CorrelationFileParser::log() const
{
    return _plugin->log();
}
