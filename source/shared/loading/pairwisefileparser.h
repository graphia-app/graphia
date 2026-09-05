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

#ifndef PAIRWISEFILEPARSER_H
#define PAIRWISEFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/iuserelementdata.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/pairwisecolumntype.h"

#include <QUrl>

#include <utility>

class PairwiseParser : public IParser
{
private:
    IUserNodeData* _userNodeData = nullptr;
    IUserEdgeData* _userEdgeData = nullptr;

    bool _firstRowIsHeader = false;
    PairwiseColumnsConfiguration _columnsConfiguration =
    {
        {0, {PairwiseColumnType::SourceNode, {}}},
        {1, {PairwiseColumnType::TargetNode, {}}},
    };

protected:
    TabularData _tabularData;

    virtual bool parseTabularData(const QUrl& url, IGraphModel* graphModel) = 0;

public:
    PairwiseParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
        TabularData* tabularData = nullptr);

    void setFirstRowIsHeader(bool firstRowIsHeader) { _firstRowIsHeader = firstRowIsHeader; }
    void setColumnsConfiguration(const PairwiseColumnsConfiguration& columnsConfiguration)
    {
        _columnsConfiguration = columnsConfiguration;
    }

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
};

template<typename TabularDataParser>
class PairwiseFileParser : public PairwiseParser
{
private:
    bool parseTabularData(const QUrl& url, IGraphModel* graphModel) override
    {
        TabularDataParser parser(this);

        if(!parser.parse(url, graphModel))
        {
            setFailureReason(parser.failureReason());
            return false;
        }

        _tabularData = std::move(parser.tabularData());

        return true;
    }

public:
    using PairwiseParser::PairwiseParser;

    static bool canLoad(const QUrl& url)
    {
        return TabularDataParser::canLoad(url);
    }
};

using PairwiseCSVFileParser =   PairwiseFileParser<CsvFileParser>;
using PairwiseSSVFileParser =   PairwiseFileParser<SsvFileParser>;
using PairwiseTSVFileParser =   PairwiseFileParser<TsvFileParser>;
using PairwiseTXTFileParser =   PairwiseFileParser<TxtFileParser>;
using PairwiseXLSXFileParser =  PairwiseFileParser<XlsxTabularDataParser>;

#endif // PAIRWISEFILEPARSER_H
