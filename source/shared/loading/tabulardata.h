#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/iparser.h"
#include "shared/utils/string.h"
#include "thirdparty/csv/parser.hpp"

#include <QObject>
#include <QString>
#include <QUrl>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstring>

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
    TabularData(TabularData&&) = default;
    TabularData& operator=(TabularData&&) = default;

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
};

template<const char Delimiter>
class TextDelimitedTabularDataParser : public IParser
{
    static_assert(Delimiter != '\"', "Delimiter cannot be a quotemark");

private:
    TabularData _tabularData;
    const IParser* _parent = nullptr;

public:
    explicit TextDelimitedTabularDataParser(IParser* parent = nullptr) : _parent(parent)
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
        auto fileSize = file.tellg();
        file.seekg(0, std::ios::end);
        fileSize = file.tellg() - fileSize;
        file.seekg(0, std::ios::beg);

        auto parser = std::make_unique<aria::csv::CsvParser>(file);
        parser->delimiter(Delimiter);
        for(auto& row : *parser)
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
        }

        // Free up any over-allocation
        _tabularData.shrinkToFit();

        return true;
    }

    TabularData& tabularData() { return _tabularData; }

    static bool canLoad(const QUrl& url)
    {
        // Scans a few lines and identifies the delimiter based on the consistency
        // of the column count it produces on each row (within a delta tolerance)
        // with each potential delimiter.
        // Largest consistent column count within the tolerance delta wins
        const std::string POTENTIAL_DELIMITERS = ",;\t ";
        const int LINE_SCAN_COUNT = 5;
        const int ALLOWED_COLUMN_COUNT_DELTA = 1;

        std::vector<size_t> columnAppearances(POTENTIAL_DELIMITERS.length());
        std::ifstream file(url.toLocalFile().toStdString());

        // Find the appropriate delimiter from list
        for(size_t i = 0; i < POTENTIAL_DELIMITERS.length(); ++i)
        {
            auto testDelimiter = POTENTIAL_DELIMITERS.at(i);
            auto testParser = std::make_unique<aria::csv::CsvParser>(file);
            testParser->delimiter(testDelimiter);

            // Scan first few rows for matching columns
            size_t rowIndex = 0;
            size_t columnAppearancesMin = std::numeric_limits<size_t>::max();
            for(const auto& testRow : *testParser)
            {
                if(rowIndex >= LINE_SCAN_COUNT)
                    break;

                columnAppearances.at(i) = std::max(testRow.size(), columnAppearances[i]);
                columnAppearancesMin = std::min(testRow.size(), columnAppearancesMin);

                if(columnAppearances.at(i) - columnAppearancesMin > ALLOWED_COLUMN_COUNT_DELTA)
                {
                    // Inconsistent column count so not a matrix
                    columnAppearances.at(i) = 0;
                    break;
                }

                rowIndex++;
            }

            file.clear();
            file.seekg(0, std::ios::beg);
        }
        std::vector<char> likelyDelimiters;
        size_t maxColumns = *std::max_element(columnAppearances.begin(), columnAppearances.end());
        if(maxColumns > 0)
        {
            for(size_t i = 0; i < columnAppearances.size(); ++i)
            {
                if(columnAppearances.at(i) >= maxColumns)
                    likelyDelimiters.push_back(POTENTIAL_DELIMITERS.at(i));
            }
        }

        // It is possible for more than one delimiter to give the same results
        // however it is very unlikely. If it happens just use the first one we find.
        if(!likelyDelimiters.empty())
        {
            char delimiter = likelyDelimiters[0];

            if(Delimiter == delimiter)
                return true;
        }

        return false;
    }
};

using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;

#endif // TABULARDATA_H
