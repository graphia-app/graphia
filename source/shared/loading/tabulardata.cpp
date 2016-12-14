#include "tabulardata.h"

void TabularData::initialise(int columns, int rows)
{
    _columns = columns;
    _rows = rows;

    _data.resize(_columns * _rows);
}

size_t TabularData::index(int column, int row) const
{
    Q_ASSERT(column < _columns);
    Q_ASSERT(row < _rows);

    return !_transposed ?
                column + (row * _columns) :
                row + (column * _rows);
}

int TabularData::numColumns() const
{
    return !_transposed ? _columns : _rows;
}

int TabularData::numRows() const
{
    return !_transposed ? _rows : _columns;
}

void TabularData::setValueAt(int column, int row, std::string&& value)
{
    _data.at(index(column, row)) = std::move(value);
}

const std::string& TabularData::valueAt(int column, int row) const
{
    return _data.at(index(column, row));
}

QString TabularData::valueAtQString(int column, int row) const
{
    // Note that the whitespace is trimmed
    return QString::fromStdString(valueAt(column, row)).trimmed();
}
