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

#ifndef ADJACENCYMATRIXFILEPARSER_H
#define ADJACENCYMATRIXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/graph/elementid.h"

#include "shared/loading/adjacencymatrixutils.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/xlsxtabulardataparser.h"
#include "shared/loading/matlabfileparser.h"

#include "shared/utils/is_detected.h"

#include <QString>
#include <QUrl>

#include <utility>

template<typename> class IUserElementData;
using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

class AdjacencyMatrixParser : public IParser
{
private:
    IUserNodeData* _userNodeData = nullptr;
    IUserEdgeData* _userEdgeData = nullptr;

    double _minimumAbsEdgeWeight = 0.0;
    bool _skipDuplicates = false;

    void addEdge(IGraphModel* graphModel,
        NodeId sourceNodeId, NodeId targetNodeId,
        double edgeWeight, double absEdgeWeight);

    bool parseAdjacencyMatrix(const TabularData& tabularData, IGraphModel* graphModel);
    bool parseEdgeList(const TabularData& tabularData, IGraphModel* graphModel);

protected:
    TabularData _tabularData;

    virtual bool parseTabularData(const QUrl& url, IGraphModel* graphModel) = 0;

public:
    AdjacencyMatrixParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData,
        TabularData* tabularData = nullptr);

    void setMinimumAbsEdgeWeight(double minimumAbsEdgeWeight) { _minimumAbsEdgeWeight = minimumAbsEdgeWeight; }
    void setSkipDuplicates(bool skipDuplicates) { _skipDuplicates = skipDuplicates; }

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    QString log() const override;
};

template<typename TabularDataParserType>
class AdjacencyMatrixFileParser : public AdjacencyMatrixParser
{
private:
    bool parseTabularData(const QUrl& url, IGraphModel* graphModel) override
    {
        TabularDataParserType parser(this);

        if(!parser.parse(url, graphModel))
        {
            setFailureReason(parser.failureReason());
            return false;
        }

        _tabularData = std::move(parser.tabularData());

        return true;
    }

public:
    using AdjacencyMatrixParser::AdjacencyMatrixParser;

    template<typename Parser>
    using setRowLimit_t = decltype(std::declval<Parser>().setRowLimit(0));

    static bool canLoad(const QUrl& url)
    {
        if(!TabularDataParserType::canLoad(url))
            return false;

        constexpr bool TabularDataParserHasSetRowLimit =
            std::experimental::is_detected_v<setRowLimit_t, TabularDataParserType>;

        // If TabularDataParserType has ::setRowLimit, do some additional checks
        if constexpr(TabularDataParserHasSetRowLimit)
        {
            TabularDataParserType parser;
            parser.setRowLimit(5);
            parser.parse(url);
            const auto& tabularData = parser.tabularData();

            return AdjacencyMatrixUtils::isEdgeList(tabularData) ||
                AdjacencyMatrixUtils::isAdjacencyMatrix(tabularData);
        }

        return true;
    }
};

using AdjacencyMatrixTSVFileParser =    AdjacencyMatrixFileParser<TsvFileParser>;
using AdjacencyMatrixSSVFileParser =    AdjacencyMatrixFileParser<SsvFileParser>;
using AdjacencyMatrixCSVFileParser =    AdjacencyMatrixFileParser<CsvFileParser>;
using AdjacencyMatrixTXTFileParser =    AdjacencyMatrixFileParser<TxtFileParser>;
using AdjacencyMatrixXLSXFileParser =   AdjacencyMatrixFileParser<XlsxTabularDataParser>;
using AdjacencyMatrixMatLabFileParser = AdjacencyMatrixFileParser<MatLabFileParser>;

#endif // ADJACENCYMATRIXFILEPARSER_H
