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

// The static parts of CorrelationFileParser, which transform the data before
// correlation; these are separate so that they can be built into correlationcore

#include "correlationfileparser.h"

#include "plugins/correlation/featurescaling.h"
#include "plugins/correlation/quantilenormaliser.h"
#include "plugins/correlation/softmaxnormaliser.h"

#include "shared/utils/container.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <vector>

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
        auto top = static_cast<size_t>(dataRect.y());
        auto bottom = static_cast<size_t>(dataRect.y()) + static_cast<size_t>(dataRect.height());

        // Calculate column averages
        double averageValue = 0.0;
        size_t rowCount = 0;
        for(size_t avgRowIndex = top; avgRowIndex < bottom; avgRowIndex++)
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

            // A pointer and size are passed to span here because Apple libc++ doesn't implement
            // the generalised iterator constructor; the last row's end is the data's, so a
            // pointer to it can't be taken by dereferencing it
            auto sortedIndices = u::sortedIndicesOf(std::span(&*it, width));
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

bool CorrelationFileParser::normalise(NormaliseType normaliseType,
    ContinuousDataVectors& dataRows, IParser* parser)
{
    bool success = true;

    switch(normaliseType)
    {
    case NormaliseType::MinMax:
    {
        const MinMaxNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Mean:
    {
        const MeanNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Standardisation:
    {
        const StandardisationNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::UnitScaling:
    {
        const UnitScalingNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Quantile:
    {
        const QuantileNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    case NormaliseType::Softmax:
    {
        const SoftmaxNormaliser normaliser;
        success = normaliser.process(dataRows, parser);
        break;
    }
    default:
        break;
    }

    if(!success)
        return false;

    if(normaliseType != NormaliseType::None)
    {
        for(auto& dataRow : dataRows)
            dataRow.update();
    }

    return true;
}

double CorrelationFileParser::epsilonFor(const std::vector<double>& data)
{
    double minValue = std::numeric_limits<double>::max();
    bool anyPositive = false;
    for(const auto& value : data)
    {
        if(value > 0.0 && value < minValue)
        {
            minValue = value;
            anyPositive = true;
        }
    }

    // With no positive values, there's nothing for it to obscure
    if(!anyPositive)
        return std::nextafter(0.0, 1.0);

    // This is picked to be appropriately small for log scaling
    // data, but not so small that it obscures the values themselves
    return minValue * 0.5;
}
