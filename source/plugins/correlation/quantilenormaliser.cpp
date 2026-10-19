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

#include "quantilenormaliser.h"

#include "shared/loading/iparser.h"

#include <QtGlobal>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <vector>

bool QuantileNormaliser::process(ContinuousDataVectors& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    auto numColumns = dataRows.at(0).size();

    std::vector<std::vector<double>> sortedColumnValues(numColumns);

    for(size_t column = 0; column < numColumns; column++)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        auto& sortedValues = sortedColumnValues[column];
        sortedValues.reserve(dataRows.size());

        // Get column values
        std::ranges::transform(dataRows, std::back_inserter(sortedValues),
        [column](const auto& dataRow)
        {
            return dataRow.valueAt(column);
        });

        std::ranges::sort(sortedValues);

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((column * 100) / numColumns));
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    std::vector<double> rowMeans(dataRows.size());

    // Populate row means
    for(size_t row = 0; row < rowMeans.size(); row++)
    {
        double meanValue = 0.0;
        for(size_t column = 0; column < numColumns; column++)
            meanValue += sortedColumnValues[column][row];

        rowMeans[row] = meanValue / static_cast<double>(numColumns);
    }

    // So that the mean of a range of row means can be found quickly
    std::vector<double> cumulativeRowMeans(rowMeans.size() + 1, 0.0);
    std::partial_sum(rowMeans.begin(), rowMeans.end(), cumulativeRowMeans.begin() + 1);

    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            const auto& sortedValues = sortedColumnValues[column];
            auto [first, last] = std::ranges::equal_range(sortedValues, dataRow.valueAt(column));
            auto lo = static_cast<size_t>(std::distance(sortedValues.begin(), first));
            auto hi = static_cast<size_t>(std::distance(sortedValues.begin(), last));
            Q_ASSERT(lo < hi && hi <= rowMeans.size());

            // Tied values are given the mean of the row means for the ranks they span
            auto value = (hi - lo) == 1 ? rowMeans[lo] :
                (cumulativeRowMeans[hi] - cumulativeRowMeans[lo]) / static_cast<double>(hi - lo);

            dataRow.setValueAt(column, value);
        }
    }

    return true;
}
