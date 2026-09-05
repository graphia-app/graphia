/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/loading/iparser.h"
#include "shared/utils/typeidentity.h"

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRect>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Progressable;

struct MatrixTypeResult
{
    bool _isMatrix = true;
    QString _reason;

    explicit operator bool() const { return _isMatrix; }
};

class TabularData
{
private:
    std::vector<QString> _data;
    size_t _columns = 0;
    size_t _rows = 0;
    bool _transposed = false;

    size_t index(size_t column, size_t row) const;

public:
    TabularData() = default;
    TabularData(TabularData&&) noexcept;
    TabularData& operator=(TabularData&&) noexcept;

    // Make it harder to copy TabularData
    TabularData(const TabularData&) = delete;
    TabularData& operator=(const TabularData&) = delete;

    void reserve(size_t columns, size_t rows);

    bool empty() const;
    size_t numColumns() const;
    size_t numRows() const;
    bool transposed() const { return _transposed; }
    const QString& valueAt(size_t column, size_t row) const;

    void setTransposed(bool transposed) { _transposed = transposed; }
    void setValueAt(size_t column, size_t row, const QString& value, int progressHint = -1);

    void shrinkToFit();
    void reset();

    // First row is assumed to be a header, by default
    TypeIdentity columnTypeIdentity(size_t columnIndex, size_t rowIndex = 1) const;
    std::vector<TypeIdentity> columnTypeIdentities(Progressable* progressable = nullptr, size_t rowIndex = 1) const;

    bool columnHasDuplicates(size_t columnIndex, size_t rowIndex = 1) const;
    std::vector<bool> columnDuplicates(Progressable* progressable = nullptr, size_t rowIndex = 1) const;

    // First column is assumed to be a header, by default
    TypeIdentity rowTypeIdentity(size_t rowIndex, size_t columnIndex = 1) const;
    std::vector<TypeIdentity> rowTypeIdentities(Progressable* progressable = nullptr, size_t columnIndex = 1) const;

    bool rowHasDuplicates(size_t rowIndex, size_t columnIndex = 1) const;
    std::vector<bool> rowDuplicates(Progressable* progressable = nullptr, size_t columnIndex = 1) const;

    int columnMatchPercentage(size_t columnIndex, const QStringList& referenceValues) const;
    int rowMatchPercentage(size_t rowIndex, const QStringList& referenceValues) const;

    QRect findLargestNumericalDataRect(Progressable* progressable = nullptr) const;
    QRect findLargestNonNumericalDataRect(Progressable* progressable = nullptr) const;

    static QString contentIdentityOf(const QUrl& url);
};

enum class EmptyCellPolicy { Keep, Skip };

class TextDelimitedTabularDataParser : public IParser
{
private:
    size_t _rowLimit = 0;
    TabularData _tabularData;

    EmptyCellPolicy _emptyCellPolicy = EmptyCellPolicy::Keep;
    std::string _delimiters;

    bool isDelimiter(uint32_t codePoint) const;

protected:
    TextDelimitedTabularDataParser(EmptyCellPolicy emptyCellPolicy,
        std::string delimiters, IParser* parent);

    bool canLoadFrom(const QUrl& url);

public:
    bool parse(const QUrl& url, IGraphModel* = nullptr) override;

    void setRowLimit(size_t rowLimit) { _rowLimit = rowLimit; }

    TabularData& tabularData() { return _tabularData; }
};

class CsvFileParser : public TextDelimitedTabularDataParser
{
public:
    explicit CsvFileParser(IParser* parent = nullptr);
    static bool canLoad(const QUrl& url);
};

class TsvFileParser : public TextDelimitedTabularDataParser
{
public:
    explicit TsvFileParser(IParser* parent = nullptr);
    static bool canLoad(const QUrl& url);
};

class TxtFileParser : public TextDelimitedTabularDataParser
{
public:
    explicit TxtFileParser(IParser* parent = nullptr);
    static bool canLoad(const QUrl& url);
};

class SsvFileParser : public TextDelimitedTabularDataParser
{
public:
    explicit SsvFileParser(IParser* parent = nullptr);
    static bool canLoad(const QUrl& url);
};

#endif // TABULARDATA_H
