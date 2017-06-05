#include "quantilenormaliser.h"

#include <set>

QuantileNormaliser::QuantileNormaliser(const TabularData &data,
                                       size_t firstDataColumn, size_t firstDataRow)
    : _data(data), _firstDataColumn(firstDataColumn),
      _firstDataRow(firstDataRow),
      _numDataColumns(_data.numColumns() - _firstDataColumn),
      _numDataRows(_data.numRows() - _firstDataRow),
      _rowMeans(_data.numRows() - _firstDataRow),
      _ranking(_data.numColumns() - _firstDataColumn, std::vector<double>(_data.numRows() - _firstDataRow))
{
    std::vector<std::vector<double>> sortedColumnValues(_numDataColumns);

    for(size_t column = 0; column < _numDataColumns; column++)
    {
        std::vector<double> columnValues;

        // Get column values
        for(size_t row = _firstDataRow; row < _data.numRows(); row++)
            columnValues.push_back(_data.valueAsQString(column + _firstDataColumn, row).toDouble());

        // Sort
        auto sortedValues = columnValues;
        std::sort(sortedValues.begin(), sortedValues.end());
        std::set<double> uniqueSortedValues(sortedValues.begin(), sortedValues.end());

        for(size_t row = 0; row < _numDataRows; row++)
        {
            // Set the ranking
            double dataValue = _data.valueAsQString(column + _firstDataColumn,
                                                    row + firstDataRow).toDouble();
            int i = 0;
            for(auto uniqueValue : uniqueSortedValues)
            {
                if (uniqueValue == dataValue)
                    _ranking[column][row] = i;
                i++;
            }
        }
        // Copy Result to sortedColumns
        sortedColumnValues[column] = sortedValues;
    }

    // Populate row means
    for(size_t row = 0; row < _numDataRows; row++)
    {
        double meanValue = 0;
        for(size_t column = 0; column < _numDataColumns; column++)
            meanValue += sortedColumnValues[column][row];
        _rowMeans[row] = meanValue / static_cast<double>(_numDataColumns);
    }
}

double QuantileNormaliser::value(size_t column, size_t row)
{
    auto rank = _ranking[column - _firstDataColumn][row - _firstDataRow];
    return _rowMeans[rank];
}
