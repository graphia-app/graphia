#include "tabulardata.h"

void TabularData::reserve(size_t columns, size_t rows)
{
    _data.reserve(columns * rows);
}

size_t TabularData::index(size_t column, size_t row) const
{
    Q_ASSERT(column < numColumns());
    Q_ASSERT(row < numRows());

    auto index = !_transposed ?
        column + (row * _columns) :
        row + (column * _columns);

    return index;
}

size_t TabularData::numColumns() const
{
    return !_transposed ? _columns : _rows;
}

size_t TabularData::numRows() const
{
    return !_transposed ? _rows : _columns;
}

void TabularData::setValueAt(size_t column, size_t row, std::string&& value, int progressHint)
{
    size_t columns = column >= _columns ? column + 1 : _columns;
    size_t rows = row >= _rows ? row + 1 : _rows;

    if(columns > _columns)
    {
        // Pad any existing rows with blank values
        size_t numNewColumns = columns - _columns;
        auto it = _data.begin() + _columns;

        size_t rowsToPad = rows - 1;
        while(rowsToPad--)
        {
            _data.insert(it, numNewColumns, {});
            it += columns;
        }
    }

    _columns = columns;
    _rows = rows;

    auto newSize = _columns * _rows;
    if(newSize > _data.capacity())
    {
        size_t reserveSize = newSize;

        if(progressHint >= 10)
        {
            // If we've made it some significant way through the input, we can be
            // reasonably confident of the total memory requirement...
            reserveSize = std::max(reserveSize, (100 * newSize) / progressHint);
        }
        else
        {
            // ...otherwise just double the reservation each time we need more space
            reserveSize = newSize * 2;
        }

        _data.reserve(reserveSize);
    }

    _data.resize(newSize);
    _data.at(index(column, row)) = std::move(value);
}

void TabularData::shrinkToFit()
{
    _data.shrink_to_fit();
}

void TabularData::reset()
{
    _data.clear();
    _columns = 0;
    _rows = 0;
    _transposed = false;
}

const std::string& TabularData::valueAt(size_t column, size_t row) const
{
    return _data.at(index(column, row));
}

QString TabularData::valueAsQString(size_t column, size_t row) const
{
    // Note that the whitespace is trimmed
    return QString::fromStdString(valueAt(column, row)).trimmed();
}
