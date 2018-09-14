#include "quantilenormaliser.h"

#include "shared/loading/iparser.h"
#include "shared/utils/cancellable.h"

#include <set>
#include <algorithm>

#include <QtGlobal>

bool QuantileNormaliser::process(std::vector<CorrelationDataRow>& dataRows, IParser* parser) const
{
    if(dataRows.empty())
        return true;

    auto numColumns = dataRows.at(0)._numColumns;

    std::vector<std::vector<double>> sortedColumnValues(numColumns);
    std::vector<size_t> ranking(dataRows.size() * numColumns);

    uint64_t j = 0;
    size_t row = 0;

    for(size_t column = 0; column < numColumns; column++)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        std::vector<double> columnValues;
        columnValues.reserve(dataRows.size());

        // Get column values
        for(const auto& dataRow : dataRows)
            columnValues.push_back(dataRow._data.at(column));

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
                if(uniqueValue == dataRow._data.at(column))
                    ranking[index] = i;

                i++;
            }

            row++;

            if(parser != nullptr)
                parser->setProgress(static_cast<int>((j++ * 100) / dataRows.size()));
        }

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

            dataRow._data[column] = rowMeans[rank];
        }

        row++;
    }

    return true;
}
