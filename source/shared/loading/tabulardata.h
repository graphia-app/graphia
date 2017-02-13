#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/graph/imutablegraph.h"
#include "shared/loading/baseparser.h"

#include "thirdparty/utfcpp/utf8.h"

#include <QUrl>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class TabularData
{
private:
    std::vector<std::string> _data;
    int _columns = 0;
    int _rows = 0;
    bool _transposed = false;

    size_t index(int column, int row) const;

public:
    void initialise(int numColumns, int numRows);

    int numColumns() const;
    int numRows() const;
    bool transposed() const { return _transposed; }
    const std::string& valueAt(int column, int row) const;
    QString valueAtQString(int column, int row) const;

    void setTransposed(bool transposed) { _transposed = transposed; }
    void setValueAt(int column, int row, std::string&& value);
};

template<const char Delimiter> class TextDelimitedTabularDataParser : public BaseParser
{
    static_assert(Delimiter != '\"', "Delimiter cannot be a quotemark");

private:
    TabularData _tabularData;
    const BaseParser* _parentParser = nullptr;

    template<typename TokenFn>
    bool parse(const QUrl& url, const ProgressFn& progress, TokenFn tokenFn)
    {
        std::ifstream file(url.toLocalFile().toStdString());
        if(!file)
            return false;

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;

        std::string line;
        std::string currentToken;
        int currentColumn = 0;
        int currentRow = 0;

        progress(-1);

        file.seekg(0, std::ios::beg);
        while(std::getline(file, line))
        {
            if(_parentParser != nullptr && _parentParser->cancelled())
                return false;

            bool inQuotes = false;

            std::string validatedLine;
            utf8::replace_invalid(line.begin(), line.end(), std::back_inserter(validatedLine));
            auto it = validatedLine.begin();
            auto end = validatedLine.end();
            while(it < end)
            {
                uint32_t codePoint = utf8::next(it, end);

                if(codePoint == '\"')
                {
                    if(inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken));
                        currentToken.clear();

                        // Quote closed, but there is text before the delimiter
                        while(it < end && codePoint != Delimiter)
                            codePoint = utf8::next(it, end);
                    }

                    inQuotes = !inQuotes;
                }
                else
                {
                    bool delimiter = (codePoint == Delimiter);

                    if(delimiter && !inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken));
                        currentToken.clear();
                    }
                    else
                        utf8::unchecked::append(codePoint, std::back_inserter(currentToken));
                }
            }

            if(!currentToken.empty())
            {
                tokenFn(currentColumn++, currentRow, std::move(currentToken));
                currentToken.clear();
            }

            currentRow++;
            currentColumn = 0;

            progress(static_cast<int>(file.tellg() * 100 / fileSize));
        }

        return true;
    }

public:
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress)
    {
        int columns = 0;
        int rows = 0;

        // First pass to determine the size of the table
        graph.setPhase(QObject::tr("Finding size"));
        bool success = parse(url, progress,
        [&columns, &rows](int column, int row, auto)
        {
            columns = std::max(columns, column + 1);
            rows = std::max(rows, row + 1);
        });

        if(!success)
            return false;

        _tabularData.initialise(columns, rows);

        graph.setPhase(QObject::tr("Parsing"));
        return parse(url, progress,
        [this](int column, int row, auto&& token)
        {
            _tabularData.setValueAt(column, row, std::move(token));
        });
    }

    void setParentParser(const BaseParser* parentParser) { _parentParser = parentParser; }

    TabularData& tabularData() { return _tabularData; }
};

using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;

#endif // TABULARDATA_H
