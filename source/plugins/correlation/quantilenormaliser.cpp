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

#include "quantilenormaliser.h"

#include "shared/loading/iparser.h"
#include "shared/utils/cancellable.h"

#include <set>
#include <algorithm>

#include <QtGlobal>

bool QuantileNormaliser::process(ContinuousDataRows& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    auto numColumns = dataRows.at(0).numColumns();

    std::vector<std::vector<double>> sortedColumnValues(numColumns);
    std::vector<size_t> ranking(dataRows.size() * numColumns);

    size_t row = 0;

    for(size_t column = 0; column < numColumns; column++)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        std::vector<double> columnValues;
        columnValues.reserve(dataRows.size());

        // Get column values
        std::transform(dataRows.begin(), dataRows.end(), std::back_inserter(columnValues),
        [column](const auto& dataRow)
        {
            return dataRow.valueAt(column);
        });

        // Sort
        auto sortedValues = columnValues;
        std::sort(sortedValues.begin(), sortedValues.end());
        auto uniqueSortedValues = sortedValues;
        uniqueSortedValues.erase(std::unique(uniqueSortedValues.begin(), uniqueSortedValues.end()),
            uniqueSortedValues.end());

        row = 0;
        for(const auto& dataRow : dataRows)
        {
            auto index = (row * numColumns) + column;

            // Set the ranking
            size_t i = 0;
            for(auto uniqueValue : uniqueSortedValues)
            {
                if(uniqueValue == dataRow.valueAt(column))
                    ranking[index] = i;

                i++;
            }

            row++;
        }

        if(parser != nullptr)
            parser->setProgress(static_cast<int>((column * 100) / numColumns));

        // Copy Result to sortedColumns
        sortedColumnValues[column] = sortedValues;
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    std::vector<double> rowMeans(dataRows.size());

    // Populate row means
    for(row = 0; row < rowMeans.size(); row++)
    {
        double meanValue = 0.0;
        for(size_t column = 0; column < numColumns; column++)
            meanValue += sortedColumnValues[column][row];

        rowMeans[row] = meanValue / static_cast<double>(numColumns);
    }

    row = 0;
    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto index = (row * numColumns) + column;
            auto rank = ranking[index];
            Q_ASSERT(rank < rowMeans.size());

            dataRow.setValueAt(column, rowMeans[rank]);
        }

        row++;
    }

    return true;
}
