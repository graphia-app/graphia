#include "minmaxnormaliser.h"

#include <limits>
#include <algorithm>

bool MinMaxNormaliser::process(std::vector<double>& data, size_t numColumns, size_t numRows,
                               const std::function<bool()>& cancelled, const ProgressFn& progress) const
{
    std::vector<double> minColumn;
    std::vector<double> maxColumn;

    minColumn.resize(numColumns, std::numeric_limits<double>::max());
    maxColumn.resize(numColumns, std::numeric_limits<double>::lowest());

    uint64_t i = 0;

    for(size_t row = 0; row < numRows; row++)
    {
        if(cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto index = (row * numColumns) + column;

            minColumn[column] = std::min(minColumn[column], data.at(index));
            maxColumn[column] = std::max(maxColumn[column], data.at(index));

            progress(static_cast<int>((i++ * 50) / data.size()));
        }
    }

    for(size_t row = 0; row < numRows; row++)
    {
        if(cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto index = (row * numColumns) + column;
            auto& value = data[index];

            auto range = maxColumn[column] - minColumn[column];
            if(range > 0.0)
                value = (value - minColumn[column]) / range;
            else
                value = 0.0;

            progress(static_cast<int>((i++ * 50) / data.size()));
        }
    }

    progress(-1);

    return true;
}
