#include "tabulardata.h"

void TabularData::initialise(size_t columns, size_t rows)
{
    _columns = columns;
    _rows = rows;

    _data.resize(_columns * _rows);
}

size_t TabularData::index(size_t column, size_t row) const
{
    Q_ASSERT(column < numColumns());
    Q_ASSERT(row < numRows());

    return !_transposed ?
                column + (row * _columns) :
                row + (column * _columns);
}

size_t TabularData::numColumns() const
{
    return !_transposed ? _columns : _rows;
}

size_t TabularData::numRows() const
{
    return !_transposed ? _rows : _columns;
}

void TabularData::setValueAt(size_t column, size_t row, std::string&& value)
{
    _data.at(index(column, row)) = std::move(value);
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
