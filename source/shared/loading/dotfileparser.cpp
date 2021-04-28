#include "dotfileparser.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/support/iterators/istream_iterator.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include "progress_iterator.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/utils/container.h"
#include "shared/utils/visitor.h"

#include <QUrl>
#include <QFileInfo>

#include <fstream>
#include <variant>
#include <type_traits>
#include <functional>
#include <map>

// https://www.graphviz.org/doc/info/lang.html

namespace SpiritDotParser
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

using EdgeEnd = std::variant<DotNode, boost::recursive_wrapper<DotSubGraph>>;

struct EdgeStatement
{
    EdgeEnd _edgeEnd;
    std::vector<EdgeEnd> _edgeEnds;
    AttributeList _attributeList;
};

struct AttributeStatement
{
    std::string _type;
    AttributeList _attributeList;
};

using Statement = std::variant<boost::recursive_wrapper<DotSubGraph>, KeyValue,
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

} // namespace SpiritDotParser

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::KeyValue,
    _key,
    _value
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::DotNode,
    _text,
    _ignore1,
    _ignore2
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::NodeStatement,
    _node,
    _attributeList
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::EdgeStatement,
    _edgeEnd,
    _edgeEnds,
    _attributeList
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::AttributeStatement,
    _type,
    _attributeList
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::DotSubGraph,
    _id,
    _statementList
)

BOOST_FUSION_ADAPT_STRUCT(
    SpiritDotParser::DotGraph,
    _id,
    _statementList
)

namespace SpiritDotParser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
using x3::string;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using ascii::char_;

const x3::rule<class DSG, DotSubGraph> dotSubGraph = "dotSubGraph";

const x3::rule<class ANI, QString> alphaNumericId = "alphaNumericId";
const auto alphaNumericId_def = lexeme[char_("a-zA-Z\200-\377_") >> *char_("a-zA-Z\200-\3770-9_")];

const x3::rule<class N, QString> numeral = "numeral";
const auto numeral_def = lexeme[(-char_('-') >> char_('.') >> +char_("0-9")) |
    (-char_('-') >> +char_("0-9") >> -(char_('.') >> *char_("0-9")))];

const x3::rule<class QS, QString> quotedString = "quotedString";
const auto escapedQuote = lit('\\') >> char_('"');
const auto quotedString_def = lit('"') >> lexeme[*(escapedQuote | ~char_('"'))] >> lit('"');

const x3::rule<class NX, QString> nonXml = "nonXml";
const auto nonXml_def = *(~char_("[<>]"));

const x3::rule<class XT, QString> xmlTag = "xmlTag";
const auto xmlTag_def = char_('<') >> +(~char_('>')) >> char_('>');

const x3::rule<class HS, QString> htmlString = "htmlString";
const auto htmlString_def = lit('<') >> lexeme[*(nonXml >> xmlTag >> nonXml)] >> lit('>');

const x3::rule<class ID, QString> identifier = "identifier";
const auto identifier_def = htmlString | quotedString | numeral | alphaNumericId;

const x3::rule<class KV, KeyValue> keyValue = "keyValue";
const auto keyValue_def = identifier >> lit('=') >> identifier;

const x3::rule<class AL, AttributeList> keyValueList = "keyValueList";
const auto keyValueList_def = lit('[') >> *(keyValue >> -(lit(';') | lit(','))) >> lit(']');

const x3::rule<class DNI, DotNode> dotNode = "Node";
const auto dotNode_def = identifier >>
    // This isn't strictly the correct grammar, but we're never going to
    // be using the result anyway so it doesn't really matter
    -(lit(':') >> identifier) >> -(lit(':') >> identifier);

const x3::rule<class NS, NodeStatement> nodeStatement = "nodeStatement";
const auto nodeStatement_def = dotNode >> -keyValueList;

const x3::rule<class DE, EdgeEnd> edgeEnd = "edgeEnd";
const auto edgeEnd_def = dotNode | dotSubGraph;

const x3::rule<class ES, EdgeStatement> edgeStatement = "edgeStatement";
const auto edgeStatement_def = edgeEnd >> +((lit("--") | lit("->")) >> edgeEnd) >> -keyValueList;

const x3::rule<class AS, AttributeStatement> attributeStatement = "attributeStatement";
const auto attributeStatement_def = (string("graph") | string("node") | string("edge")) >> keyValueList;

const x3::rule<class S, Statement> statement = "statement";
const auto statement_def = dotSubGraph | keyValue | attributeStatement | edgeStatement | nodeStatement;

const x3::rule<class SL, StatementList> statementList = "list";
const auto statementList_def = *(statement >> -lit(";"));

const auto dotSubGraph_def = -(lit("subgraph") >> -identifier) >>
    lit("{") >> statementList >> lit("}");

const x3::rule<class DG, DotGraph> dotGraph = "dotGraph";
const auto dotGraph_def = -lit("strict") >> (lit("graph") | lit("digraph")) >> -identifier >>
    lit("{") >> statementList >> lit("}");

BOOST_SPIRIT_DEFINE(dotGraph, dotSubGraph, keyValue, keyValueList,
    dotNode, edgeEnd,
    nodeStatement, edgeStatement, attributeStatement,
    statement, statementList,
    alphaNumericId, numeral, quotedString,
    xmlTag, nonXml, htmlString,
    identifier)

const auto comment = lexeme[
    "/*" >> *(char_ - "*/") >> "*/" |
    "//" >> *~char_("\r\n") >> x3::eol];

const auto skipper = comment | ascii::space;

bool build(DotFileParser& parser, const DotGraph& dot, IGraphModel& graphModel,
    UserNodeData& userNodeData, UserEdgeData& userEdgeData)
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

    auto processStatementList = [&](const StatementList& l)
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

    auto processEdgeEnd = [&](const EdgeEnd& e)
    {
        return std::visit(Visitor
        {
            [&](const boost::recursive_wrapper<DotSubGraph>& subGraph)
                { return processStatementList(subGraph.get()._statementList); },
            [&](const DotNode& node)
                { return std::vector<QString>({node._text}); }
        }, e);
    };

    processStatement = [&](const Statement& s)
    {
        return std::visit(Visitor
        {
            [&](const boost::recursive_wrapper<DotSubGraph>& subGraph)
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

                    for(const auto& node : sourceNodes)
                        sourceNodeIds.emplace_back(addNode(node));

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
                        QString attributeName = QObject::tr("Edge ") + attribute._key;
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
                    QString attributeName = QObject::tr("Node ") + attribute._key;
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
        if(s._type == "node")
        {
            for(const auto& attribute : s._attributeList)
            {
                for(auto nodeId : nodeIds)
                {
                    QString attributeName = QObject::tr("Node ") + attribute._key;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }
            }
        }
        else if(s._type == "edge")
        {
            for(const auto& attribute : s._attributeList)
            {
                for(auto edgeId : edgeIds)
                {
                    QString attributeName = QObject::tr("Edge ") + attribute._key;
                    userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
                }
            }
        }
    }

    return true;
}

} // namespace SpiritDotParser

DotFileParser::DotFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
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

    QString localFile = url.toLocalFile();
    QFileInfo fileInfo(localFile);

    if(!fileInfo.exists())
        return false;

    auto fileSize = fileInfo.size();
    if(fileSize == 0)
    {
        setFailureReason(QObject::tr("File is empty."));
        return false;
    }

    setProgress(-1);

    std::ifstream stream(localFile.toStdString());
    stream.unsetf(std::ios::skipws);

    boost::spirit::istream_iterator istreamIt(stream);
    using DotIterator = progress_iterator<decltype(istreamIt)>;
    DotIterator it(istreamIt);
    DotIterator end;

    it.onPositionChanged(
    [this, &fileSize](size_t position)
    {
        setProgress(static_cast<int>((position * 100) / fileSize));
    });

    auto cancelledFn = [this] { return cancelled(); };
    it.setCancelledFn(cancelledFn);

    graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

    SpiritDotParser::DotGraph dot;
    bool success = false;

    try
    {
        success = SpiritDotParser::x3::phrase_parse(it, end,
            SpiritDotParser::dotGraph, SpiritDotParser::skipper, dot);
    }
    catch(DotIterator::cancelled_exception&) {}

    if(cancelled() || !success || it != end)
        return false;

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return SpiritDotParser::build(*this, dot, *graphModel,
        *_userNodeData, *_userEdgeData);
}
