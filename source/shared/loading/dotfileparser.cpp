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

#include "dotfileparser.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/utils/container.h"
#include "shared/utils/recursivevalue.h"
#include "shared/utils/visitor.h"

#include <lexy_utils.h>
#include <lexy/action/parse.hpp>
#include <lexy/dsl/case_folding.hpp>

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QFileInfo>

#include <cstddef>
#include <utility>
#include <variant>
#include <functional>
#include <map>
#include <set>
#include <vector>

// https://www.graphviz.org/doc/info/lang.html

namespace LexyDotParser
{
struct DotSubGraph;

struct KeyValue
{
    QString _key;
    QString _value;
};

using AttributeList = std::vector<KeyValue>;

struct DotNode
{
    QString _text;

    // For our purposes, port and compass point are
    // not useful and we're aren't parsing them
    // correctly anyway, but they'll end up here
    QString _ignore1;
    QString _ignore2;
};

struct NodeStatement
{
    DotNode _node;
    AttributeList _attributeList;
};

using EdgeEnd = std::variant<DotNode, RecursiveValue<DotSubGraph>>;

struct EdgeStatement
{
    EdgeEnd _edgeEnd;
    std::vector<EdgeEnd> _edgeEnds;
    AttributeList _attributeList;
};

enum class AttributeStatementType : unsigned char { Graph, Node, Edge };

struct AttributeStatement
{
    AttributeStatementType _type;
    AttributeList _attributeList;
};

using Statement = std::variant<RecursiveValue<DotSubGraph>, KeyValue,
    AttributeStatement, EdgeStatement, NodeStatement>;
using StatementList = std::vector<Statement>;

struct DotSubGraph
{
    QString _id;
    StatementList _statementList;
};

struct DotGraph
{
    bool _strict = false;
    bool _directed = false;
    QString _id;
    StatementList _statementList;
};

namespace grammar
{
namespace dsl = lexy::dsl;

using lexyutils::anyByte;
using lexyutils::asQString;
using lexyutils::nonAsciiByte;

// char_("a-zA-Z\200-\377_") and char_("a-zA-Z0-9\200-\377_") respectively
constexpr auto idLead = dsl::ascii::alpha_underscore / nonAsciiByte;
constexpr auto idTrailing = dsl::ascii::alpha_digit_underscore / nonAsciiByte;
constexpr auto idPattern = dsl::identifier(idLead, idTrailing);

constexpr auto keywordStrict = dsl::ascii::case_folding(LEXY_KEYWORD("strict", idPattern));
constexpr auto keywordSubgraph = dsl::ascii::case_folding(LEXY_KEYWORD("subgraph", idPattern));

constexpr auto edgeOperator = LEXY_LIT("--") | LEXY_LIT("->");

// Lines beginning with # are output by the C preprocessor, and are ignored
constexpr auto comment =
    (LEXY_LIT("//") >> dsl::until(dsl::ascii::newline).or_eof()) |
    (LEXY_LIT("/*") >> dsl::until(LEXY_LIT("*/"))) |
    (dsl::lit_c<'#'> >> dsl::until(dsl::ascii::newline).or_eof());

constexpr auto skipper = dsl::ascii::space | comment;

struct AlphaNumericId : lexy::token_production
{
    static constexpr auto rule = idPattern;
    static constexpr auto value = asQString;
};

struct Numeral : lexy::token_production
{
    static constexpr auto rule = dsl::capture(dsl::token(
        dsl::if_(dsl::lit_c<'-'>) +
        ((dsl::lit_c<'.'> >> dsl::digits<>) |
        (dsl::digits<> >> dsl::if_(dsl::lit_c<'.'> >> dsl::if_(dsl::digits<>))))));

    static constexpr auto value = asQString;
};

// Within a quoted string, \" is a quote, and \\ is a pair of backslashes that doesn't
// escape what follows; any other backslash is kept as it is, as Graphviz does
struct QuotedString : lexy::token_production
{
    static constexpr auto rule = dsl::lit_c<'"'> >>
        (dsl::capture(dsl::token(dsl::while_(LEXY_LIT("\\\\") | LEXY_LIT("\\\"") |
            (anyByte - dsl::lit_c<'"'>)))) + dsl::lit_c<'"'>);

    static constexpr auto value = lexy::callback<QString>([](const auto& lexeme)
    {
        auto text = lexyutils::qStringFrom(lexeme);
        QString result;
        result.reserve(text.size());

        for(qsizetype i = 0; i < text.size(); i++)
        {
            if(text.at(i) == '\\' && i + 1 < text.size())
            {
                if(text.at(i + 1) == '"')
                    i++;
                else if(text.at(i + 1) == '\\')
                    result.append(text.at(i++));
            }

            result.append(text.at(i));
        }

        return result;
    });
};

struct ConcatenatedString
{
    static constexpr auto rule = dsl::list(dsl::p<QuotedString>, dsl::sep(dsl::lit_c<'+'>));
    static constexpr auto value = lexy::as_list<std::vector<QString>> >>
        lexy::callback<QString>([](const std::vector<QString>& strings)
        {
            QString result;
            for(const auto& string : strings)
                result.append(string);

            return result;
        });
};

struct HtmlString : lexy::token_production
{
    static constexpr auto nonXml = anyByte - (dsl::lit_c<'<'> / dsl::lit_c<'>'>);
    static constexpr auto xmlTag = dsl::lit_c<'<'> >>
        (dsl::while_one(anyByte - dsl::lit_c<'>'>) + dsl::lit_c<'>'>);

    // Note the whitespace skip; the content of the string itself is verbatim,
    // but any whitespace immediately following the opening < is not part of it
    static constexpr auto rule = dsl::lit_c<'<'> >>
        (dsl::whitespace(skipper) +
        dsl::capture(dsl::token(dsl::while_(nonXml | xmlTag))) + dsl::lit_c<'>'>);

    static constexpr auto value = asQString;
};

struct Identifier
{
    static constexpr auto rule = dsl::p<HtmlString> | dsl::p<ConcatenatedString> |
        dsl::p<Numeral> | dsl::p<AlphaNumericId>;

    static constexpr auto value = lexy::forward<QString>;
};

// An identifier that need not be present, in which case it yields an empty string
struct OptionalIdentifier
{
    static constexpr auto rule = dsl::opt(dsl::p<Identifier>);
    static constexpr auto value = lexy::bind(lexy::forward<QString>, lexy::_1.or_default());
};

struct NodePort
{
    static constexpr auto rule = dsl::opt(dsl::lit_c<':'> >> dsl::p<Identifier>);
    static constexpr auto value = lexy::bind(lexy::forward<QString>, lexy::_1.or_default());
};

struct KeyValue
{
    static constexpr auto rule = dsl::p<Identifier> >>
        (dsl::lit_c<'='> + dsl::p<Identifier>);

    static constexpr auto value = lexy::construct<LexyDotParser::KeyValue>;
};

struct KeyValueList
{
    static constexpr auto rule = dsl::lit_c<'['> >>
        (dsl::opt(dsl::list(dsl::p<KeyValue> >>
            dsl::if_(dsl::lit_c<';'> / dsl::lit_c<','>))) + dsl::lit_c<']'>);

    static constexpr auto value = lexy::as_list<LexyDotParser::AttributeList>;
};

struct AttributeLists
{
    static constexpr auto rule = dsl::list(dsl::p<KeyValueList>);
    static constexpr auto value = lexy::concat<LexyDotParser::AttributeList>;
};

struct DotNode
{
    // This isn't strictly the correct grammar, but we're never going to
    // be using the port or compass point anyway so it doesn't really matter
    static constexpr auto rule = dsl::p<Identifier> >>
        (dsl::p<NodePort> + dsl::p<NodePort>);

    static constexpr auto value = lexy::construct<LexyDotParser::DotNode>;
};

struct StatementList;

struct SubGraphBody
{
    static constexpr auto rule = dsl::curly_bracketed(dsl::recurse<StatementList>);
    static constexpr auto value = lexy::forward<LexyDotParser::StatementList>;
};

struct SubGraph
{
    static constexpr auto rule = dsl::peek(keywordSubgraph | dsl::lit_c<'{'>) >>
        (dsl::if_(keywordSubgraph) + dsl::p<OptionalIdentifier> + dsl::p<SubGraphBody>);

    static constexpr auto value = lexy::construct<LexyDotParser::DotSubGraph>;
};

struct EdgeEnd
{
    static constexpr auto rule = dsl::p<SubGraph> | dsl::p<DotNode>;

    static constexpr auto value = lexy::construct<LexyDotParser::EdgeEnd>;
};

constexpr auto attributeTypes = lexy::symbol_table<LexyDotParser::AttributeStatementType>
    .case_folding(dsl::ascii::case_folding)
    .map<LEXY_SYMBOL("graph")>(LexyDotParser::AttributeStatementType::Graph)
    .map<LEXY_SYMBOL("node")>(LexyDotParser::AttributeStatementType::Node)
    .map<LEXY_SYMBOL("edge")>(LexyDotParser::AttributeStatementType::Edge);

struct AttributeStatement
{
    struct Type
    {
        static constexpr auto rule = dsl::symbol<attributeTypes>(idPattern);
        static constexpr auto value = lexy::forward<LexyDotParser::AttributeStatementType>;
    };

    static constexpr auto rule = dsl::p<Type> >> dsl::p<AttributeLists>;
    static constexpr auto value = lexy::construct<LexyDotParser::AttributeStatement>;
};

// The various kinds of statement can't be distinguished by their leading
// tokens alone, so the statement is scanned manually instead
struct Statement
{
    using scan_result = lexy::scan_result<LexyDotParser::Statement>;

    static constexpr auto lead = dsl::peek(idLead / dsl::ascii::digit /
        dsl::lit_c<'-'> / dsl::lit_c<'.'> / dsl::lit_c<'"'> /
        dsl::lit_c<'<'> / dsl::lit_c<'{'>);

    static constexpr auto rule = lead >> dsl::scan;
    static constexpr auto value = lexy::forward<LexyDotParser::Statement>;

    template<typename Context, typename Reader>
    static scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        LexyDotParser::EdgeEnd edgeEnd;

        // graph/node/edge, followed by an attribute list
        lexy::scan_result<LexyDotParser::AttributeStatement> attributeStatement;
        if(scanner.branch(attributeStatement, AttributeStatement{}))
        {
            if(!scanner)
                return lexy::scan_failed;

            return LexyDotParser::Statement(std::move(attributeStatement).value());
        }

        lexy::scan_result<LexyDotParser::DotSubGraph> subGraph;
        if(scanner.branch(subGraph, SubGraph{}))
        {
            if(!scanner)
                return lexy::scan_failed;

            RecursiveValue<LexyDotParser::DotSubGraph> subGraphValue(std::move(subGraph).value());

            if(!scanner.peek(edgeOperator))
                return LexyDotParser::Statement(std::move(subGraphValue));

            edgeEnd = LexyDotParser::EdgeEnd(std::move(subGraphValue));
        }
        else
        {
            auto id = scanner.parse(Identifier{});
            if(!scanner)
                return lexy::scan_failed;

            // A bare identifier followed by = is an attribute assignment
            if(scanner.branch(dsl::lit_c<'='>))
            {
                auto assignment = scanner.parse(Identifier{});
                if(!scanner)
                    return lexy::scan_failed;

                return LexyDotParser::Statement(LexyDotParser::KeyValue{
                    std::move(id).value(), std::move(assignment).value()});
            }

            LexyDotParser::DotNode node{std::move(id).value(), {}, {}};

            if(scanner.branch(dsl::lit_c<':'>))
            {
                auto port = scanner.parse(Identifier{});
                if(!scanner)
                    return lexy::scan_failed;

                node._ignore1 = std::move(port).value();

                if(scanner.branch(dsl::lit_c<':'>))
                {
                    auto compassPoint = scanner.parse(Identifier{});
                    if(!scanner)
                        return lexy::scan_failed;

                    node._ignore2 = std::move(compassPoint).value();
                }
            }

            if(!scanner.peek(edgeOperator))
            {
                // Not an edge statement, so it must be a node statement
                LexyDotParser::NodeStatement nodeStatement{._node = std::move(node), ._attributeList = {}};

                lexy::scan_result<LexyDotParser::AttributeList> attributeList;
                if(scanner.branch(attributeList, AttributeLists{}))
                {
                    if(!scanner)
                        return lexy::scan_failed;

                    nodeStatement._attributeList = std::move(attributeList).value();
                }

                return LexyDotParser::Statement(std::move(nodeStatement));
            }

            edgeEnd = LexyDotParser::EdgeEnd(std::move(node));
        }

        LexyDotParser::EdgeStatement edgeStatement;
        edgeStatement._edgeEnd = std::move(edgeEnd);

        while(scanner.branch(edgeOperator))
        {
            auto end = scanner.parse(EdgeEnd{});
            if(!scanner)
                return lexy::scan_failed;

            edgeStatement._edgeEnds.emplace_back(std::move(end).value());
        }

        lexy::scan_result<LexyDotParser::AttributeList> attributeList;
        if(scanner.branch(attributeList, AttributeLists{}))
        {
            if(!scanner)
                return lexy::scan_failed;

            edgeStatement._attributeList = std::move(attributeList).value();
        }

        return LexyDotParser::Statement(std::move(edgeStatement));
    }
};

struct StatementList
{
    static constexpr auto rule =
        dsl::opt(dsl::list(dsl::p<Statement> >> dsl::if_(dsl::lit_c<';'>)));

    static constexpr auto value = lexy::as_list<LexyDotParser::StatementList>;
};

// Whether the graph is directed
constexpr auto graphTypes = lexy::symbol_table<bool>
    .case_folding(dsl::ascii::case_folding)
    .map<LEXY_SYMBOL("graph")>(false)
    .map<LEXY_SYMBOL("digraph")>(true);

struct DotGraph
{
    static constexpr auto whitespace = skipper;

    static constexpr auto rule =
        dsl::flag(keywordStrict) +
        dsl::symbol<graphTypes>(idPattern) +
        dsl::p<OptionalIdentifier> + dsl::curly_bracketed(dsl::p<StatementList>) + dsl::eof;

    static constexpr auto value = lexy::construct<LexyDotParser::DotGraph>;
};
} // namespace grammar

static bool build(DotFileParser& parser, const DotGraph& dot, IGraphModel& graphModel,
    IUserNodeData& userNodeData, IUserEdgeData& userEdgeData)
{
    std::map<QString, NodeId> dotNodeToNodeId;

    // Default attributes apply to elements created after they're set,
    // within the graph or subgraph that sets them
    struct Defaults
    {
        std::map<QString, QString> _node;
        std::map<QString, QString> _edge;
    };

    std::vector<Defaults> defaults(1);

    // In a strict graph, there is at most one edge between a pair of nodes, in each
    // direction if it's directed, and repeating an edge gives the existing one
    std::map<std::pair<NodeId, NodeId>, EdgeId> strictEdges;

    auto addEdge = [&](NodeId sourceNodeId, NodeId targetNodeId)
    {
        if(!dot._strict)
            return graphModel.mutableGraph().addEdge(sourceNodeId, targetNodeId);

        auto key = std::make_pair(sourceNodeId, targetNodeId);
        if(!dot._directed && key.second < key.first)
            std::swap(key.first, key.second);

        if(!u::contains(strictEdges, key))
            strictEdges.emplace(key, graphModel.mutableGraph().addEdge(sourceNodeId, targetNodeId));

        return strictEdges.at(key);
    };

    auto addNode = [&](const QString& nodeName)
    {
        if(!u::contains(dotNodeToNodeId, nodeName))
        {
            auto nodeId = graphModel.mutableGraph().addNode();

            dotNodeToNodeId[nodeName] = nodeId;
            userNodeData.setValueBy(nodeId, IParser::nodeNameAttribute(), nodeName);
            graphModel.setNodeName(nodeId, nodeName);

            for(const auto& [key, value] : defaults.back()._node)
                userNodeData.setValueBy(nodeId, key, value);
        }

        return dotNodeToNodeId.at(nodeName);
    };

    std::function<std::vector<QString>(const Statement& s)> processStatement;

    auto processStatementList = [&parser, &processStatement, &defaults](const StatementList& l)
    {
        std::vector<QString> nodes;
        std::set<QString> nodesSeen;
        size_t i = 0;

        defaults.push_back(defaults.back());

        for(const auto& s : l)
        {
            for(const auto& node : processStatement(s))
            {
                if(nodesSeen.insert(node).second)
                    nodes.emplace_back(node);
            }

            parser.setProgress(static_cast<int>((i++ * 100) / l.size()));
        }

        defaults.pop_back();

        return nodes;
    };

    auto processEdgeEnd = [&processStatementList](const EdgeEnd& e)
    {
        return std::visit(Visitor
        {
            [&processStatementList](const RecursiveValue<DotSubGraph>& subGraph)
                { return processStatementList(subGraph.get()._statementList); },
            [](const DotNode& node)
                { return std::vector<QString>({node._text}); }
        }, e);
    };

    processStatement = [&](const Statement& s)
    {
        return std::visit(Visitor
        {
            [&](const RecursiveValue<DotSubGraph>& subGraph)
                { return processStatementList(subGraph.get()._statementList); },
            [&](const AttributeStatement& attribute)
            {
                for(const auto& [key, value] : attribute._attributeList)
                {
                    if(attribute._type == AttributeStatementType::Node)
                        defaults.back()._node[key] = value;
                    else if(attribute._type == AttributeStatementType::Edge)
                        defaults.back()._edge[key] = value;
                }

                return std::vector<QString>{};
            },
            [&](const EdgeStatement& edge)
            {
                std::vector<EdgeId> addedEdgeIds;

                auto sourceNodes = processEdgeEnd(edge._edgeEnd);

                // Every node in the statement, so that an edge to or from a subgraph
                // containing it is also an edge to or from it
                auto statementNodes = sourceNodes;

                for(const auto& target : edge._edgeEnds)
                {
                    std::vector<NodeId> sourceNodeIds;
                    std::vector<NodeId> targetNodeIds;

                    auto targetNodes = processEdgeEnd(target);

                    sourceNodeIds.reserve(sourceNodes.size());
                    for(const auto& node : sourceNodes)
                        sourceNodeIds.emplace_back(addNode(node));

                    targetNodeIds.reserve(targetNodes.size());
                    for(const auto& node : targetNodes)
                        targetNodeIds.emplace_back(addNode(node));

                    for(auto sourceNodeId : sourceNodeIds)
                    {
                        for(auto targetNodeId : targetNodeIds)
                        {
                            addedEdgeIds.emplace_back(addEdge(sourceNodeId, targetNodeId));
                        }
                    }

                    statementNodes.insert(statementNodes.end(), targetNodes.begin(), targetNodes.end());
                    sourceNodes = std::move(targetNodes);
                }

                for(auto edgeId : addedEdgeIds)
                {
                    for(const auto& [key, value] : defaults.back()._edge)
                        userEdgeData.setValueBy(edgeId, key, value);

                    for(const auto& attribute : edge._attributeList)
                    {
                        const QString attributeName = attribute._key;
                        userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
                    }
                }

                return statementNodes;
            },
            [&](const NodeStatement& node)
            {
                auto nodeId = addNode(node._node._text);

                for(const auto& attribute : node._attributeList)
                {
                    const QString attributeName = attribute._key;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }

                return std::vector<QString>({node._node._text});
            },
            [](auto&&) { return std::vector<QString>(); } // Ignore everything else
        }, s);
    };

    processStatementList(dot._statementList);

    parser.setProgress(-1);

    return true;
}

} // namespace LexyDotParser

DotFileParser::DotFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(nodeNameAttribute());
}

bool DotFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    const QString localFile = url.toLocalFile();
    const QFileInfo fileInfo(localFile);

    if(!fileInfo.exists())
        return false;

    if(fileInfo.size() == 0)
    {
        setFailureReason(QObject::tr("File is empty."));
        return false;
    }

    setProgress(-1);

    QFile file(localFile);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    auto contents = file.readAll();
    file.close();

    // A UTF-8 byte order mark
    if(contents.startsWith("\xEF\xBB\xBF"))
        contents.remove(0, 3);

    if(contents.isEmpty())
        return false;

    const auto size = static_cast<size_t>(contents.size());
    lexyutils::progress_input input(contents.constData(), size);

    input.onPositionChanged([this, size](size_t position)
    {
        setProgress(static_cast<int>((position * 100) / size));
    });

    auto cancelledFn = [this] { return cancelled(); };
    input.setCancelledFn(cancelledFn);

    setPhase(QObject::tr("Parsing"));

    QString errorDescription;
    auto result = lexy::parse<LexyDotParser::grammar::DotGraph>(input,
        lexyutils::describeFirstError(contents.constData(), errorDescription));

    if(cancelled())
        return false;

    if(!result.is_success())
    {
        setFailureReason(errorDescription);
        return false;
    }

    setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return LexyDotParser::build(*this, result.value(), *graphModel,
        *_userNodeData, *_userEdgeData);
}
