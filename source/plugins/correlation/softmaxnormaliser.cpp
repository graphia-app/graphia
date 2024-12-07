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

#include "softmaxnormaliser.h"

#include "shared/loading/iparser.h"
#include "shared/utils/cancellable.h"

#include <cmath>

bool SoftmaxNormaliser::process(ContinuousDataVectors& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    auto numColumns = dataRows.at(0).size();

    std::vector<double> maxs;
    maxs.resize(numColumns, std::numeric_limits<double>::lowest());

    uint64_t i = 0;

    for(const auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto value = dataRow.valueAt(column);
            maxs[column] = std::max(maxs[column], value);
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((i * 100) / dataRows.size()));

        i++;
    }

    for(size_t column = 0; column < numColumns; column++)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        double sum = 0.0;
        for(const auto& dataRow : dataRows)
        {
            auto value = dataRow.valueAt(column) - maxs.at(column);
            auto expValue = std::exp(value);
            sum += expValue;
        }

        for(auto& dataRow : dataRows)
        {
            auto value = dataRow.valueAt(column) - maxs.at(column);
            auto expValue = std::exp(value);
            auto newValue = expValue / sum;
            dataRow.setValueAt(column, newValue);
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((column * 100) / numColumns));
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    return true;
}
