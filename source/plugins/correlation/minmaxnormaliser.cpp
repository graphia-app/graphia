#include "minmaxnormaliser.h"

MinMaxNormaliser::MinMaxNormaliser(const TabularData &data, size_t firstDataColumn, size_t firstDataRow)
    : _data(data), _firstDataColumn(firstDataColumn), _firstDataRow(firstDataRow)
{
    _minColumn.resize(data.numColumns() - _firstDataColumn, std::numeric_limits<double>::max());
    _maxColumn.resize(data.numColumns() - _firstDataColumn, std::numeric_limits<double>::lowest());
    for(size_t column = _firstDataColumn; column < _data.numColumns(); column++)
    {
        size_t index = column - _firstDataColumn;
        for(size_t row = _firstDataRow; row < _data.numRows(); row++)
        {
            _minColumn[index] = std::min(_minColumn[index], data.valueAsQString(column, row).toDouble());
            _maxColumn[index] = std::max(_maxColumn[index], data.valueAsQString(column, row).toDouble());
        }
    }
}

double MinMaxNormaliser::value(size_t column, size_t row)
{
    double value = _data.valueAsQString(column, row).toDouble();
    size_t index = column - _firstDataColumn;
    if(_maxColumn[index] - _minColumn[index] != 0)
        return (value - _minColumn[index]) / (_maxColumn[index] - _minColumn[index]);
    else
        return 0;
}
