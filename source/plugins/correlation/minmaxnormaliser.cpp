#include "minmaxnormaliser.h"

#include "shared/loading/iparser.h"
#include "shared/utils/cancellable.h"

#include <limits>
#include <algorithm>

bool MinMaxNormaliser::process(std::vector<CorrelationDataRow>& dataRows,
                               IParser* parser) const
{
    if(dataRows.empty())
        return true;

    std::vector<double> minColumn;
    std::vector<double> maxColumn;

    auto numColumns = dataRows.at(0)._numColumns;

    minColumn.resize(numColumns, std::numeric_limits<double>::max());
    maxColumn.resize(numColumns, std::numeric_limits<double>::lowest());

    uint64_t i = 0;

    for(const auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            minColumn[column] = std::min(minColumn[column], dataRow._data.at(column));
            maxColumn[column] = std::max(maxColumn[column], dataRow._data.at(column));

            if(parser != nullptr)
                parser->setProgress(static_cast<int>((i * 50) / dataRows.size()));
        }

        i++;
    }

    for(auto& dataRow : dataRows)
    {
        if(parser != nullptr && parser->cancelled())
            return false;

        for(size_t column = 0; column < numColumns; column++)
        {
            auto& value = dataRow._data.at(column);

            auto range = maxColumn[column] - minColumn[column];
            if(range > 0.0)
                value = (value - minColumn[column]) / range;
            else
                value = 0.0;

            if(parser != nullptr)
                parser->setProgress(static_cast<int>((i * 50) / dataRows.size()));
        }

        i++;
    }

    if(parser != nullptr)
        parser->setProgress(-1);

    return true;
}
