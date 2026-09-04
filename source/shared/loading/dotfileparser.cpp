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
#include <lexy/action/scan.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

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

enum class AttributeStatementType { Graph, Node, Edge };

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
    QString _id;
    StatementList _statementList;
};

namespace grammar
{
namespace dsl = lexy::dsl;

using lexyutils::anyByte;
using lexyutils::asQString;
using lexyutils::nonAsciiByte;
using lexyutils::QuotedString;

// char_("a-zA-Z\200-\377_") and char_("a-zA-Z0-9\200-\377_") respectively
constexpr auto idLead = dsl::ascii::alpha_underscore / nonAsciiByte;
constexpr auto idTrailing = dsl::ascii::alpha_digit_underscore / nonAsciiByte;
constexpr auto idPattern = dsl::identifier(idLead, idTrailing);

constexpr auto keywordStrict = LEXY_KEYWORD("strict", idPattern);
constexpr auto keywordGraph = LEXY_KEYWORD("graph", idPattern);
constexpr auto keywordDigraph = LEXY_KEYWORD("digraph", idPattern);
constexpr auto keywordSubgraph = LEXY_KEYWORD("subgraph", idPattern);

constexpr auto edgeOperator = LEXY_LIT("--") | LEXY_LIT("->");

constexpr auto comment =
    (LEXY_LIT("//") >> dsl::until(dsl::ascii::newline).or_eof()) |
    (LEXY_LIT("/*") >> dsl::until(LEXY_LIT("*/")));

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

struct HtmlString : lexy::token_production
{
    static constexpr auto nonXml = anyByte -
        (dsl::lit_c<'['> / dsl::lit_c<'<'> / dsl::lit_c<'>'>);
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
    static constexpr auto rule = dsl::p<HtmlString> | dsl::p<QuotedString> |
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

    static constexpr auto rule = dsl::p<Type> >> dsl::p<KeyValueList>;
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
                LexyDotParser::NodeStatement nodeStatement{std::move(node), {}};

                lexy::scan_result<LexyDotParser::AttributeList> attributeList;
                if(scanner.branch(attributeList, KeyValueList{}))
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
        if(scanner.branch(attributeList, KeyValueList{}))
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

struct DotGraph
{
    static constexpr auto whitespace = skipper;

    static constexpr auto rule =
        dsl::if_(keywordStrict) + (keywordGraph | keywordDigraph) + dsl::p<OptionalIdentifier> +
        dsl::curly_bracketed(dsl::p<StatementList>) + dsl::eof;

    static constexpr auto value = lexy::construct<LexyDotParser::DotGraph>;
};
} // namespace grammar

bool build(DotFileParser& parser, const DotGraph& dot, IGraphModel& graphModel,
    IUserNodeData& userNodeData, IUserEdgeData& userEdgeData)
{
    std::vector<NodeId> nodeIds;
    std::vector<EdgeId> edgeIds;
    std::map<QString, NodeId> dotNodeToNodeId;
    std::vector<AttributeStatement> attributeStatements;

    auto addNode = [&](const QString& nodeName)
    {
        if(!u::contains(dotNodeToNodeId, nodeName))
        {
            auto nodeId = graphModel.mutableGraph().addNode();

            dotNodeToNodeId[nodeName] = nodeId;
            userNodeData.setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
            graphModel.setNodeName(nodeId, nodeName);
            nodeIds.emplace_back(nodeId);
        }

        return dotNodeToNodeId.at(nodeName);
    };

    std::function<std::vector<QString>(const Statement& s)> processStatement;

    auto processStatementList = [&parser, &processStatement](const StatementList& l)
    {
        std::vector<QString> nodes;
        size_t i = 0;

        for(const auto& s : l)
        {
            for(const auto& nodeId : processStatement(s))
                nodes.emplace_back(nodeId);

            parser.setProgress(static_cast<int>((i++ * 100) / l.size()));
        }

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
                attributeStatements.emplace_back(attribute);
                return std::vector<QString>{};
            },
            [&](const EdgeStatement& edge)
            {
                std::vector<EdgeId> addedEdgeIds;

                auto sourceNodes = processEdgeEnd(edge._edgeEnd);

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
                            auto edgeId = graphModel.mutableGraph().addEdge(
                                sourceNodeId, targetNodeId);

                            addedEdgeIds.emplace_back(edgeId);
                            edgeIds.emplace_back(edgeId);
                        }
                    }

                    sourceNodes = std::move(targetNodes);
                }

                for(auto edgeId : addedEdgeIds)
                {
                    for(const auto& attribute : edge._attributeList)
                    {
                        const QString attributeName = QObject::tr("Edge ") + attribute._key;
                        userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
                    }
                }

                return std::vector<QString>{};
            },
            [&](const NodeStatement& node)
            {
                auto nodeId = addNode(node._node._text);

                for(const auto& attribute : node._attributeList)
                {
                    const QString attributeName = QObject::tr("Node ") + attribute._key;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }

                return std::vector<QString>({node._node._text});
            },
            [](auto&&) { return std::vector<QString>(); } // Ignore everything else
        }, s);
    };

    processStatementList(dot._statementList);

    parser.setProgress(-1);

    for(const auto& s : attributeStatements)
    {
        if(s._type == AttributeStatementType::Node)
        {
            for(const auto& attribute : s._attributeList)
            {
                for(auto nodeId : nodeIds)
                {
                    const QString attributeName = QObject::tr("Node ") + attribute._key;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }
            }
        }
        else if(s._type == AttributeStatementType::Edge)
        {
            for(const auto& attribute : s._attributeList)
            {
                for(auto edgeId : edgeIds)
                {
                    const QString attributeName = QObject::tr("Edge ") + attribute._key;
                    userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
                }
            }
        }
    }

    return true;
}

} // namespace LexyDotParser

DotFileParser::DotFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
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

    const auto contents = file.readAll();
    file.close();

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

    auto result = lexy::parse<LexyDotParser::grammar::DotGraph>(input, lexy::noop);

    if(cancelled() || !result.is_success())
        return false;

    setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return LexyDotParser::build(*this, result.value(), *graphModel,
        *_userNodeData, *_userEdgeData);
}
