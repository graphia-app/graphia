#ifndef TABULARDATA_H
#define TABULARDATA_H

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/loading/iparser.h"
#include "shared/utils/string.h"
#include "thirdparty/csv/parser.hpp"

#include <QObject>
#include <QUrl>
#include <QString>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

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

template<const char Delimiter> class TextDelimitedTabularDataParser : public IParser
{
    static_assert(Delimiter != '\"', "Delimiter cannot be a quotemark");

private:
    TabularData _tabularData;
    const IParser* _parent = nullptr;

    template<typename TokenFn>
    bool tokenise(const QUrl& url, TokenFn tokenFn)
    {
        size_t currentColumn = 0;
        size_t currentRow = 0;

        std::ifstream matrixFile(url.toLocalFile().toStdString());
        auto matrixfileSize = matrixFile.tellg();
        matrixFile.seekg(0, std::ios::end);
        matrixfileSize = matrixFile.tellg() - matrixfileSize;
        matrixFile.seekg(0, std::ios::beg);

        aria::csv::CsvParser testParser(matrixFile);
        testParser.delimiter(Delimiter);
        for(auto& row : testParser)
        {
            auto progress = static_cast<int>(matrixFile.tellg() * 100 / matrixfileSize);
            setProgress(progress);
            for (auto field : row)
            {
                tokenFn(currentColumn++, currentRow,
                        QString::fromStdString(field), progress);
            }

            currentRow++;
            currentColumn = 0;
        }
        return true;
    }

public:
    explicit TextDelimitedTabularDataParser(IParser* parent = nullptr) :
        _parent(parent)
    {
        if(parent != nullptr)
        {
            setProgressFn(
            [parent](int percent)
            {
                parent->setProgress(percent);
            });
        }
    }

    bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) override
    {
        if(graphModel != nullptr)
            graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

        auto result = tokenise(url,
        [this](size_t column, size_t row, auto&& token, int progress)
        {
            _tabularData.setValueAt(column, row,
                std::forward<decltype(token)>(token), progress);
        });

        // Free up any over-allocation
        _tabularData.shrinkToFit();

        return result;
    }

    TabularData& tabularData() { return _tabularData; }

    static bool isType(const QUrl &url)
    {
        //Matrix Scanning
        std::string potentialDelimiters = ",;\t ";
        std::vector<size_t> columnAppearances(potentialDelimiters.size());

        std::ifstream file(url.toLocalFile().toStdString());
        char delimiter = '\0';

        const int LINE_SCAN_COUNT = 5;
        const int ALLOWED_COLUMN_COUNT_DELTA = 1;

        // Find the appropriate delimiter from list
        for(size_t i = 0; i < potentialDelimiters.size(); ++i)
        {
            auto testDelimiter = potentialDelimiters[i];
            aria::csv::CsvParser testParser(file);
            testParser.delimiter(testDelimiter);

            // Scan first few rows for matching columns
            size_t rowIndex = 0;
            size_t columnAppearancesMin = std::numeric_limits<size_t>::max();
            for(auto testRow : testParser)
            {
                if(rowIndex >= LINE_SCAN_COUNT)
                    break;

                columnAppearances[i] = std::max(testRow.size(), columnAppearances[i]);
                columnAppearancesMin = std::min(testRow.size(), columnAppearancesMin);

                if(columnAppearances[i] - columnAppearancesMin > ALLOWED_COLUMN_COUNT_DELTA)
                {
                    // Inconsistant column count so not a matrix
                    columnAppearances[i] = 0;
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
                if(columnAppearances[i] >= maxColumns)
                    likelyDelimiters.push_back(potentialDelimiters[i]);
            }
        }

        if(likelyDelimiters.size() > 0)
        {
            delimiter = likelyDelimiters[0];

            if(Delimiter == delimiter)
                return true;
        }

        return false;
    }
};


using CsvFileParser = TextDelimitedTabularDataParser<','>;
using TsvFileParser = TextDelimitedTabularDataParser<'\t'>;

#endif // TABULARDATA_H
