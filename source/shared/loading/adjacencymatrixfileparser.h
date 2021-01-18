/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef ADJACENCYMATRIXFILEPARSER_H
#define ADJACENCYMATRIXFILEPARSER_H

#include "shared/loading/iparser.h"

#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/matlabfileparser.h"

#include "shared/utils/is_detected.h"

#include <type_traits>

class TabularData;

template<typename> class UserElementData;
using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

class AdjacencyTabularDataMatrixParser
{
public:
    static bool isAdjacencyMatrix(const TabularData& tabularData, size_t maxRows = 5);
    static bool isEdgeList(const TabularData& tabularData, size_t maxRows = 5);

    bool parse(const TabularData& tabularData, Progressable& progressable,
        IGraphModel* graphModel, UserNodeData* userNodeData, UserEdgeData* userEdgeData);
};

template<typename TabularDataParser>
class AdjacencyMatrixParser : public IParser, public AdjacencyTabularDataMatrixParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    AdjacencyMatrixParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl& url, IGraphModel* graphModel) override
    {
        TabularDataParser parser(this);

        if(!parser.parse(url, graphModel))
            return false;

        return AdjacencyTabularDataMatrixParser::parse(parser.tabularData(), parser,
            graphModel, _userNodeData, _userEdgeData);
    }

    template<typename Parser>
    using setRowLimit_t = decltype(std::declval<Parser>().setRowLimit(0));

    static bool canLoad(const QUrl& url)
    {
        if(!TabularDataParser::canLoad(url))
            return false;

        constexpr bool TabularDataParserHasSetRowLimit =
            std::experimental::is_detected_v<setRowLimit_t, TabularDataParser>;

        // If TabularDataParser has ::setRowLimit, do some additional checks
        if constexpr(TabularDataParserHasSetRowLimit)
        {
            TabularDataParser parser;
            parser.setRowLimit(5);
            parser.parse(url);
            const auto& tabularData = parser.tabularData();

            return AdjacencyTabularDataMatrixParser::isEdgeList(tabularData) ||
                AdjacencyTabularDataMatrixParser::isAdjacencyMatrix(tabularData);
        }

        return true;
    }
};

using AdjacencyMatrixTSVFileParser =    AdjacencyMatrixParser<TsvFileParser>;
using AdjacencyMatrixSSVFileParser =    AdjacencyMatrixParser<SsvFileParser>;
using AdjacencyMatrixCSVFileParser =    AdjacencyMatrixParser<CsvFileParser>;
using AdjacencyMatrixXLSXFileParser =   AdjacencyMatrixParser<XlsxTabularDataParser>;
using AdjacencyMatrixMatLabFileParser = AdjacencyMatrixParser<MatLabFileParser>;

#endif // ADJACENCYMATRIXFILEPARSER_H
