/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/iparser.h"
#include "shared/utils/string.h"
#include "shared/utils/typeidentity.h"

#include <csv/parser.hpp>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstring>

class Progressable;

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
    void setValueAt(size_t column, size_t row, QString&& value, int progressHint = -1);

    void shrinkToFit();
    void reset();

    // First row is assumed to be a header, by default
    TypeIdentity typeIdentity(size_t columnIndex, size_t rowIndex = 1) const;
    std::vector<TypeIdentity> typeIdentities(Progressable* progressable = nullptr, size_t rowIndex = 1) const;

    int columnMatchPercentage(size_t columnIndex, const QStringList& referenceValues) const;
};

Q_DECLARE_METATYPE(std::shared_ptr<TabularData>)

template<const char Delimiter>
class TextDelimitedTabularDataParser : public IParser
{
    static_assert(Delimiter != '"', "Delimiter cannot be a quotemark");

private:
    size_t _rowLimit = 0;
    TabularData _tabularData;

public:
    explicit TextDelimitedTabularDataParser(IParser* parent = nullptr)
    {
        if(parent != nullptr)
            setProgressFn([parent](int percent) { parent->setProgress(percent); });
    }

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override
    {
        if(graphModel != nullptr)
            graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

        size_t columnIndex = 0;
        size_t rowIndex = 0;

        std::ifstream file(url.toLocalFile().toStdString());

        if(!file)
            return false;

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;
        file.seekg(0, std::ios::beg);

        if(fileSize == 0)
        {
            setFailureReason(QObject::tr("File is empty."));
            return false;
        }

        auto parser = std::make_unique<aria::csv::CsvParser>(file);
        parser->delimiter(Delimiter);
        for(const auto& row : *parser)
        {
            auto progress = file.eof() ? 100 :
                static_cast<int>(parser->position() * 100 / fileSize);
            setProgress(progress);

            for(const auto& field : row)
            {
                _tabularData.setValueAt(columnIndex++, rowIndex,
                    QString::fromStdString(field), progress);
            }

            rowIndex++;
            columnIndex = 0;

            if(_rowLimit > 0 && rowIndex > _rowLimit)
                break;

            if(cancelled())
                return false;
        }

        // Free up any over-allocation
        _tabularData.shrinkToFit();

        return true;
    }

    void setRowLimit(size_t rowLimit) { _rowLimit = rowLimit; }

    TabularData& tabularData() { return _tabularData; }

    static bool canLoad(const QUrl& url)
    {
        std::ifstream file(url.toLocalFile().toStdString());

        if(!file)
            return false;

        auto testParser = aria::csv::CsvParser(file);
        testParser.delimiter(Delimiter);

        // Count the maximum and minimum number of columns in the first few rows
        size_t linesToScan = 5;
        size_t minColumns = std::numeric_limits<size_t>::max();
        size_t maxColumns = std::numeric_limits<size_t>::min();
        for(const auto& testRow : testParser)
        {
            auto numColumns = testRow.size();

            minColumns = std::min(numColumns, minColumns);
            maxColumns = std::max(numColumns, maxColumns);

            if(--linesToScan == 0)
                break;
        }

        if(minColumns > maxColumns)
            return false;

        // Where only a single column has been found, it's highly unlikely that
        // this parser's delimiter is correct for the file in question
        if(maxColumns < 2)
            return false;

        auto delta = maxColumns - minColumns;
        const size_t maxAllowedColumnCountDelta = 3;

        // If the column counts vary too much, refuse to load the file
        if(delta > maxAllowedColumnCountDelta)
            return false;

        return true;
    }
};

using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;
using SsvFileParser = TextDelimitedTabularDataParser<';'>;

#endif // TABULARDATA_H
