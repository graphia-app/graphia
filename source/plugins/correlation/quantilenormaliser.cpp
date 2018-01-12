#include "quantilenormaliser.h"

#include "shared/utils/cancellable.h"

#include <set>
#include <algorithm>

#include <QtGlobal>

bool QuantileNormaliser::process(std::vector<double>& data, size_t numColumns, size_t numRows,
                                 Cancellable& cancellable, const ProgressFn& progress) const
{
    std::vector<std::vector<double>> sortedColumnValues(numColumns);
    std::vector<size_t> ranking(data.size());

    uint64_t j = 0;

    for(size_t column = 0; column < numColumns; column++)
    {
        if(cancellable.cancelled())
            return false;

        std::vector<double> columnValues;
        columnValues.reserve(numRows);

        // Get column values
        for(size_t row = 0; row < numRows; row++)
        {
            auto index = (row * numColumns) + column;
            columnValues.push_back(data.at(index));
        }

        // Sort
        auto sortedValues = columnValues;
        std::sort(sortedValues.begin(), sortedValues.end());
        std::set<double> uniqueSortedValues(sortedValues.begin(), sortedValues.end());

        for(size_t row = 0; row < numRows; row++)
        {
            auto index = (row * numColumns) + column;

            // Set the ranking
            size_t i = 0;
            for(auto uniqueValue : uniqueSortedValues)
            {
                if(uniqueValue == data.at(index))
                    ranking[index] = i;

                i++;
            }

            progress(static_cast<int>((j++ * 100) / data.size()));
        }

        // Copy Result to sortedColumns
        sortedColumnValues[column] = sortedValues;
    }

    progress(-1);

    std::vector<double> rowMeans(numRows);

    // Populate row means
    for(size_t row = 0; row < numRows; row++)
    {
        double meanValue = 0.0;
        for(size_t column = 0; column < numColumns; column++)
            meanValue += sortedColumnValues[column][row];

        rowMeans[row] = meanValue / static_cast<double>(numColumns);
    }

    for(size_t row = 0; row < numRows; row++)
    {
        if(cancellable.cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto index = (row * numColumns) + column;
            auto rank = ranking[index];
            Q_ASSERT(rank < rowMeans.size());

            data[index] = rowMeans[rank];
        }
    }

    return true;
}
