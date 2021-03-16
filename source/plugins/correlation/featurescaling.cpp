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

#include "featurescaling.h"

#include "shared/loading/iparser.h"
#include "shared/utils/cancellable.h"

#include <limits>
#include <algorithm>
#include <cmath>

struct StandardNormalisationValues
{
    std::vector<double>* mins = nullptr;
    std::vector<double>* maxs = nullptr;
    std::vector<double>* ranges = nullptr;
    std::vector<double>* means = nullptr;
    std::vector<double>* stddevs = nullptr;
};

static bool calcStandardValues(const ContinuousDataRows& dataRows,
    const StandardNormalisationValues& values, IParser* parser = nullptr)
{
    if(dataRows.empty())
        return true;

    auto numColumns = dataRows.at(0).numColumns();

    if(values.mins != nullptr)
        values.mins->resize(numColumns, std::numeric_limits<double>::max());

    if(values.maxs != nullptr)
        values.maxs->resize(numColumns, std::numeric_limits<double>::lowest());

    if(values.ranges != nullptr)
        values.ranges->resize(numColumns);

    if(values.means != nullptr)
        values.means->resize(numColumns, 0.0);

    if(values.stddevs != nullptr)
        values.stddevs->resize(numColumns, 0.0);

    uint64_t i = 0;

    for(const auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto value = dataRow.valueAt(column);

            if(values.mins != nullptr)
                (*values.mins)[column] = std::min((*values.mins)[column], value);

            if(values.maxs != nullptr)
                (*values.maxs)[column] = std::max((*values.maxs)[column], value);

            if(values.means != nullptr)
                (*values.means)[column] += (value / static_cast<double>(numColumns));
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

        i++;
    }

    if(values.ranges != nullptr && values.mins != nullptr && values.maxs != nullptr)
    {
        for(size_t column = 0; column < numColumns; column++)
            (*values.ranges)[column] = (*values.maxs)[column] - (*values.mins)[column];
    }

    if(values.stddevs != nullptr && values.means != nullptr)
    {
        i = 0;

        for(const auto& dataRow : dataRows)
        {
            if(parser != nullptr && parser->cancelled())
                return false;

            for(size_t column = 0; column < numColumns; column++)
            {
                auto deviation = dataRow.valueAt(column) - (*values.means)[column];
                deviation *= deviation;
                (*values.stddevs)[column] += (deviation / static_cast<double>(numColumns));
            }

            if(parser != nullptr)
                parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

            i++;
        }

        // Variance -> Std. Deviations
        for(size_t column = 0; column < numColumns; column++)
            (*values.stddevs)[column] = std::sqrt((*values.stddevs)[column]);
    }

    return true;
}

static bool normalise(ContinuousDataRows& dataRows,
    const std::vector<double>& subtractors,
    const std::vector<double>& denominators,
    IParser* parser)
{
    auto numColumns = dataRows.at(0).numColumns();

    uint64_t i = 0;

    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            if(denominators[column] > 0.0)
            {
                dataRow.setValueAt(column,
                    (dataRow.valueAt(column) - subtractors[column]) / denominators[column]);
            }
            else
                dataRow.setValueAt(column, 0.0);
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

        i++;
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    return true;
}

bool MinMaxNormaliser::process(ContinuousDataRows& dataRows,
    IParser* parser) const
{
    if(dataRows.empty())
        return true;

    std::vector<double> mins;
    std::vector<double> maxs;
    std::vector<double> ranges;

    if(!calcStandardValues(dataRows, {&mins, &maxs, &ranges}, parser))
        return false;

    return normalise(dataRows, mins, ranges, parser);
}

bool MeanNormaliser::process(ContinuousDataRows& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    std::vector<double> mins;
    std::vector<double> maxs;
    std::vector<double> ranges;
    std::vector<double> means;

    if(!calcStandardValues(dataRows, {&mins, &maxs, &ranges, &means}, parser))
        return false;

    return normalise(dataRows, means, ranges, parser);
}

bool StandardisationNormaliser::process(ContinuousDataRows& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    std::vector<double> mins;
    std::vector<double> maxs;
    std::vector<double> ranges;
    std::vector<double> means;
    std::vector<double> stddevs;

    if(!calcStandardValues(dataRows, {&mins, &maxs, &ranges, &means, &stddevs}, parser))
        return false;

    return normalise(dataRows, means, stddevs, parser);
}

bool UnitScalingNormaliser::process(ContinuousDataRows& dataRows, IParser* parser) const
{
    auto numColumns = dataRows.at(0).numColumns();

    std::vector<double> vectorLengthColumn(numColumns);

    uint64_t i = 0;

    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto value = dataRow.valueAt(column);
            vectorLengthColumn[column] += (value * value);
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

        i++;
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    for(size_t column = 0; column < numColumns; column++)
        vectorLengthColumn[column] = std::sqrt(vectorLengthColumn[column]);

    i = 0;

    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto scaledValue = dataRow.valueAt(column) / vectorLengthColumn[column];
            dataRow.setValueAt(column, scaledValue);
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

        i++;
    }

    return true;
}
