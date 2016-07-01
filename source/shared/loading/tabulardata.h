#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/loading/baseparser.h"

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

    template<typename TokenFn>
    bool parse(const QUrl& url, const ProgressFn& progress, TokenFn tokenFn)
    {
        std::ifstream file(url.toLocalFile().toStdString());
        if(!file)
            return false;

        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;

        int percentComplete = 0;
        std::string line;
        std::string currentToken;
        int currentColumn = 0;
        int currentRow = 0;

        file.seekg(0, std::ios::beg);
        while(std::getline(file, line))
        {
            if(cancelled())
                return false;

            bool inQuotes = false;

            for(size_t i = 0; i < line.length(); i++)
            {
                if(line[i] == '\"')
                {
                    if(inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken));
                        currentToken.clear();

                        // Quote closed, but there is text before the delimiter
                        while(i < line.length() && line[i] != Delimiter)
                            i++;
                    }

                    inQuotes = !inQuotes;
                }
                else
                {
                    bool delimiter = (line[i] == Delimiter);

                    if(delimiter && !inQuotes)
                    {
                        tokenFn(currentColumn++, currentRow, std::move(currentToken));
                        currentToken.clear();
                    }
                    else
                        currentToken += line[i];
                }
            }

            if(!currentToken.empty())
            {
                tokenFn(currentColumn++, currentRow, std::move(currentToken));
                currentToken.clear();
            }

            currentRow++;
            currentColumn = 0;

            int newPercentComplete = static_cast<int>(file.tellg() * 100 / fileSize);

            if(newPercentComplete > percentComplete)
            {
                percentComplete = newPercentComplete;
                progress(newPercentComplete);
            }
        }

        return true;
    }

public:
    bool parse(const QUrl& url, IMutableGraph&, const ProgressFn& progress)
    {
        int columns = 0;
        int rows = 0;

        // First pass to determine the size of the table
        bool success = parse(url, progress,
        [&columns, &rows](int column, int row, auto)
        {
            columns = std::max(columns, column + 1);
            rows = std::max(rows, row + 1);
        });

        if(!success)
            return false;

        _tabularData.initialise(columns, rows);

        return parse(url, progress,
        [this](int column, int row, auto&& token)
        {
            _tabularData.setValueAt(column, row, std::move(token));
        });
    }

    const TabularData& tabularData() const { return _tabularData; }
};

using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;

#endif // TABULARDATA_H
