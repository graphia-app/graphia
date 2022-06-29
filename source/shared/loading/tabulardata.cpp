/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "tabulardata.h"
#include "xlsxtabulardataparser.h"

#include "shared/utils/progressable.h"

#include <set>
#include <algorithm>

TabularData::TabularData(TabularData&& other) noexcept :
    _data(std::move(other._data)),
    _columns(other._columns),
    _rows(other._rows),
    _transposed(other._transposed)

{
    other.reset();
}

TabularData& TabularData::operator=(TabularData&& other) noexcept
{
    if(this != &other)
    {
        _data = std::move(other._data);
        _columns = other._columns;
        _rows = other._rows;
        _transposed = other._transposed;

        other.reset();
    }

    return *this;
}

void TabularData::reserve(size_t columns, size_t rows)
{
    _data.reserve(columns * rows);
}

bool TabularData::empty() const
{
    return _data.empty();
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

void TabularData::setValueAt(size_t column, size_t row, QString&& value, int progressHint)
{
    size_t columns = column >= _columns ? column + 1 : _columns;
    size_t rows = row >= _rows ? row + 1 : _rows;
    auto newSize = columns * rows;

    // If the column count is increasing, jiggle all the existing rows around,
    // taking into account the new row width
    if(_rows > 0 && rows > 1 && columns > _columns)
    {
        _data.resize(newSize);

        for(size_t offset = _rows - 1; offset > 0; offset--)
        {
            auto oldPosition = _data.begin() + static_cast<std::ptrdiff_t>(offset * _columns);
            auto newPosition = _data.begin() + static_cast<std::ptrdiff_t>(offset * columns);

            std::move_backward(oldPosition,
                oldPosition + static_cast<std::ptrdiff_t>(_columns),
                newPosition + static_cast<std::ptrdiff_t>(_columns));
        }
    }

    _columns = columns;
    _rows = rows;

    if(newSize > _data.capacity())
    {
        size_t reserveSize = newSize;

        if(progressHint >= 10)
        {
            // If we've made it some significant way through the input, we can be
            // reasonably confident of the total memory requirement...

            // We over-allocate ever so slightly to avoid the case where our estimate
            // is on the small side. Otherwise, when we hit 100, we would default to
            // reallocating for each new element -- exactly what we're trying to avoid
            const auto extraFudgeFactor = 2;
            auto estimate = ((100 + extraFudgeFactor) * newSize) / progressHint;

            reserveSize = std::max(reserveSize, estimate);
        }
        else
        {
            // ...otherwise just double the reservation each time we need more space
            reserveSize = newSize * 2;
        }

        _data.reserve(reserveSize);
    }

    _data.resize(newSize);
    _data.at(index(column, row)) = value.trimmed();
}

void TabularData::shrinkToFit()
{
    auto lastRowIsEmpty = [this]
    {
        size_t column = 0;
        while(column < _columns && valueAt(column, _rows - 1).isEmpty())
            column++;

        return column >= _columns;
    };

    // Truncate any trailing empty rows
    while(_rows > 0 && lastRowIsEmpty())
    {
        _data.resize(_data.size() - _columns);
        _rows--;
    }

    _data.shrink_to_fit();
}

void TabularData::reset()
{
    _data.clear();
    _columns = 0;
    _rows = 0;
    _transposed = false;
}

const QString& TabularData::valueAt(size_t column, size_t row) const
{
    return _data.at(index(column, row));
}

TypeIdentity TabularData::typeIdentity(size_t columnIndex, size_t rowIndex) const
{
    TypeIdentity identity;

    for(; rowIndex < numRows(); rowIndex++)
    {
        const auto& value = valueAt(columnIndex, rowIndex);
        identity.updateType(value);
    }

    return identity;
}

std::vector<TypeIdentity> TabularData::typeIdentities(Progressable* progressable, size_t rowIndex) const
{
    std::vector<TypeIdentity> t;

    t.resize(numColumns());

    if(progressable != nullptr)
        progressable->setProgress(-1);

    for(size_t columnIndex = 0; columnIndex < numColumns(); columnIndex++)
    {
        if(progressable != nullptr)
            progressable->setProgress(static_cast<int>((columnIndex * 100) / numColumns()));

        t.at(columnIndex) = typeIdentity(columnIndex, rowIndex);
    }

    if(progressable != nullptr)
        progressable->setProgress(-1);

    return t;
}

int TabularData::columnMatchPercentage(size_t columnIndex, const QStringList& referenceValues) const
{
    std::set<QString> referenceSet(referenceValues.begin(), referenceValues.end());

    std::set<QString> columnValues;
    std::set<QString> intersection;

    for(size_t row = 1; row < numRows(); row++)
        columnValues.insert(valueAt(columnIndex, row));

    std::set_intersection(referenceSet.begin(), referenceSet.end(),
        columnValues.begin(), columnValues.end(),
        std::inserter(intersection, intersection.begin()));

    auto percent = static_cast<int>((intersection.size() * 100) / referenceValues.size());

    // In the case where the intersection is very small, but non-zero,
    // don't report a 0% match
    if(percent == 0 && !intersection.empty())
        percent = 1;

    return percent;
}

QString TabularData::contentIdentityOf(const QUrl& url)
{
    if(XlsxTabularDataParser::canLoad(url))
        return QStringLiteral("XLSX");

    QString identity;

    std::ifstream file(url.toLocalFile().toStdString());

    if(!file)
        return identity;

    const int maxLines = 50;
    int numLinesScanned = 0;

    std::istream* is = nullptr;
    do
    {
        std::string line;

        is = &u::getline(file, line);

        std::map<char, size_t> counts;
        bool inQuotes = false;

        for(auto character : line)
        {
            switch(character)
            {
            case '"':
                inQuotes = !inQuotes;
                break;

            case ',':
            case ';':
            case '\t':
                if(!inQuotes)
                    counts[character]++;
                break;

            default: break;
            }
        }

        if(!counts.empty())
        {
            auto maxCount = std::max_element(counts.begin(), counts.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });

            auto character = maxCount->first;

            switch(character)
            {
            case ',':  identity = QStringLiteral("CSV"); break;
            case ';':  identity = QStringLiteral("SSV"); break;
            case '\t': identity = QStringLiteral("TSV"); break;
            }
        }

        numLinesScanned++;
    } while(identity.isEmpty() &&
        !is->fail() && !is->eof() &&
        numLinesScanned < maxLines);

    return identity;
}
