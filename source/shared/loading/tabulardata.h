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

#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/iparser.h"
#include "shared/utils/string.h"
#include "shared/utils/typeidentity.h"
#include "shared/utils/source_location.h"

#include <utf8.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

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

    static QString contentIdentityOf(const QUrl& url);
};

Q_DECLARE_METATYPE(std::shared_ptr<TabularData>) // NOLINT performance-no-int-to-ptr

enum class EmptyCellPolicy { Keep, Skip };

template<EmptyCellPolicy ECP, const char... Delimiters>
class TextDelimitedTabularDataParser : public IParser
{
    static_assert(((Delimiters != '"') && ...), "Delimiter cannot be a quotemark");

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
        {
            setGenericFailureReason(CURRENT_SOURCE_LOCATION);
            return false;
        }

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;
        file.seekg(0, std::ios::beg);

        if(fileSize == 0)
        {
            setFailureReason(QObject::tr("File is empty."));
            return false;
        }

        int progress = 0;
        std::string line;
        std::string token;

        auto setCurrentCellToToken = [&]
        {
            _tabularData.setValueAt(columnIndex, rowIndex,
                QString::fromStdString(token), progress);

            token.clear();
            columnIndex++;
        };

        file.seekg(0, std::ios::beg);
        while(!u::getline(file, line).eof())
        {
            bool inQuotes = false;
            bool delimiter = false;

            std::string validatedLine;
            utf8::replace_invalid(line.begin(), line.end(), std::back_inserter(validatedLine));
            auto it = validatedLine.begin();
            auto end = validatedLine.end();
            while(it < end)
            {
                const uint32_t codePoint = utf8::next(it, end);

                if(codePoint == '"')
                {
                    if(inQuotes)
                        setCurrentCellToToken();

                    inQuotes = !inQuotes;
                    delimiter = false;
                }
                else
                {
                    auto previousTokenWasDelimiter = delimiter || columnIndex == 0;
                    delimiter = ((Delimiters == codePoint) || ...);

                    if(!delimiter || inQuotes)
                        utf8::unchecked::append(codePoint, std::back_inserter(token));
                    else if(!token.empty() || (previousTokenWasDelimiter && ECP == EmptyCellPolicy::Keep))
                        setCurrentCellToToken();
                }
            }

            if(!token.empty())
                setCurrentCellToToken();

            progress = file.eof() ? 100 :
                static_cast<int>(file.tellg() * 100 / fileSize);
            setProgress(progress);

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
        TextDelimitedTabularDataParser<ECP, Delimiters...> testParser;

        testParser.setRowLimit(5);
        if(!testParser.parse(url))
            return false;

        const auto& tabularData = testParser.tabularData();
        size_t minColumns = std::numeric_limits<size_t>::max();
        size_t maxColumns = std::numeric_limits<size_t>::min();

        for(size_t row = 0; row < tabularData.numRows(); row++)
        {
            size_t numColumns = 0;

            for(size_t column = 0; column < tabularData.numColumns(); column++)
            {
                if(!tabularData.valueAt(column, row).isEmpty())
                    numColumns = column + 1;
            }

            minColumns = std::min(numColumns, minColumns);
            maxColumns = std::max(numColumns, maxColumns);
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

using CsvFileParser = TextDelimitedTabularDataParser<EmptyCellPolicy::Keep, ','>;
using TsvFileParser = TextDelimitedTabularDataParser<EmptyCellPolicy::Keep, '\t'>;
using TxtFileParser = TextDelimitedTabularDataParser<EmptyCellPolicy::Skip, ' ', '\t'>;
using SsvFileParser = TextDelimitedTabularDataParser<EmptyCellPolicy::Keep, ';'>;

#endif // TABULARDATA_H
