/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "gmlfileparser.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/support/iterators/istream_iterator.hpp>
#include <boost/boost_spirit_qstring_adapter.h>

#include "progress_iterator.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/utils/container.h"

#include <QUrl>
#include <QFileInfo>
#include <QTextDocumentFragment>

#include <fstream>
#include <variant>
#include <map>

// http://www.fim.uni-passau.de/fileadmin/files/lehrstuhl/brandenburg/projekte/gml/gml-technical-report.pdf

namespace SpiritGmlParser
{
struct KeyValue;
using List = std::vector<boost::recursive_wrapper<KeyValue>>;

using Value = std::variant<double, int, QString, List>;
struct KeyValue
{
    QString _key;
    Value _value;
};
} // namespace SpiritGmlParser

BOOST_FUSION_ADAPT_STRUCT(
    SpiritGmlParser::KeyValue,
    _key,
    _value
)

namespace SpiritGmlParser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
// Only parse strict doubles (i.e. not integers)
x3::real_parser<double, x3::strict_real_policies<double>> const double_ = {};
using x3::int_;
using x3::lexeme;
using ascii::char_;

const x3::rule<class L, List> gmlList = "list";

const x3::rule<class NoQuotesString, QString> noQuotesString = "noQuotesString";
const auto noQuotesString_def = lexeme[lit('"') >> *(~char_('"')) >> lit('"')];

const x3::rule<class V, Value> gmlValue = "value";
const auto gmlValue_def = double_ | int_ | noQuotesString | (lit('[') >> gmlList >> lit(']'));

const x3::rule<class K, QString> gmlKey = "key";
const auto gmlKey_def = lexeme[char_("a-zA-Z") >> *char_("a-zA-Z0-9")];

const x3::rule<class KV, KeyValue> gmlKeyValue = "keyValue"; // NOLINT bugprone-forward-declaration-namespace
const auto gmlKeyValue_def = gmlKey >> gmlValue;

const auto gmlList_def = *gmlKeyValue;

BOOST_SPIRIT_DEFINE(gmlList, noQuotesString, gmlKey, gmlValue, gmlKeyValue)

struct Attribute
{
    Attribute(const QString& name, const QString& value) :
        _name(name), _value(value)
    {}

    QString _name;
    QString _value;
};

using AttributeVector = std::vector<Attribute>;

AttributeVector processAttribute(const KeyValue& attribute)
{
    struct Visitor
    {
        QString _name;
        explicit Visitor(const QString& name) : _name(name) {}

        AttributeVector operator()(double v) const          { return {{_name, QString::number(v)}}; }
        AttributeVector operator()(int v) const             { return {{_name, QString::number(v)}}; }
        AttributeVector operator()(const QString& v) const
        {
            return {{_name, QTextDocumentFragment::fromHtml(v).toPlainText()}};
        }
        AttributeVector operator()(const List& v) const
        {
            AttributeVector result;

            for(const auto& attribute : v)
            {
                auto childAttributes = processAttribute(attribute.get());

                for(const auto& childAttribute : childAttributes)
                {
                    QString subName = _name + "." + childAttribute._name;
                    result.emplace_back(subName, childAttribute._value);
                }
            }

            return result;
        }
    };

    return std::visit(Visitor(attribute._key), attribute._value);
}

bool build(GmlFileParser& parser, const List& gml, IGraphModel& graphModel,
    IUserNodeData& userNodeData, IUserEdgeData& userEdgeData)
{
    auto findIntValue = [](const List& list, const QString& key) -> const int*
    {
        auto keyValue = std::find_if(list.begin(), list.end(), [&key](auto& item)
        {
           return item.get()._key == key;
        });

        if(keyValue != list.end())
            return std::get_if<int>(&keyValue->get()._value);

        return nullptr;
    };

    std::map<int, NodeId> gmlIdToNodeId;

    auto processNode = [&](const List& node)
    {
        const auto* id = findIntValue(node, QStringLiteral("id"));
        if(id == nullptr)
            return false;

        auto nodeId = graphModel.mutableGraph().addNode();
        gmlIdToNodeId[*id] = nodeId;

        auto nodeName = QString::number(*id);

        for(const auto& attributeWrapper : node)
        {
            const auto& keyValue = attributeWrapper.get();
            if(keyValue._key == QStringLiteral("id"))
                continue;

            if(keyValue._key == QStringLiteral("label"))
            {
                // If there is a label attribute, use it as the node name
                const auto* label = std::get_if<QString>(&keyValue._value);
                if(label != nullptr)
                    nodeName = *label;
            }
            else
            {
                auto attributes = processAttribute(keyValue);

                for(const auto& attribute : attributes)
                {
                    QString attributeName = QObject::tr("Node ") + attribute._name;
                    userNodeData.setValueBy(nodeId, attributeName, attribute._value);
                }
            }
        }

        userNodeData.setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
        graphModel.setNodeName(nodeId, nodeName);

        return true;
    };

    auto processEdge = [&](const List& edge)
    {
        const auto* sourceId = findIntValue(edge, QStringLiteral("source"));
        const auto* targetId = findIntValue(edge, QStringLiteral("target"));

        if(sourceId == nullptr || targetId == nullptr)
            return false;

        if(!u::contains(gmlIdToNodeId, *sourceId) || !u::contains(gmlIdToNodeId, *targetId))
            return false;

        auto sourceNodeId = gmlIdToNodeId[*sourceId];
        auto targetNodeId = gmlIdToNodeId[*targetId];
        auto edgeId = graphModel.mutableGraph().addEdge(sourceNodeId, targetNodeId);

        for(const auto& attributeWrapper : edge)
        {
            const auto& keyValue = attributeWrapper.get();
            if(keyValue._key == QStringLiteral("source") || keyValue._key == QStringLiteral("target"))
                continue;

            auto attributes = processAttribute(keyValue);

            for(const auto& attribute : attributes)
            {
                QString attributeName = QObject::tr("Edge ") + attribute._name;
                userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
            }
        }

        return true;
    };

    std::vector<const List*> edges;

    for(const auto& keyValue : gml)
    {
        const auto& key = keyValue.get()._key;

        if(key == QStringLiteral("graph"))
        {
            const auto* graph = std::get_if<List>(&keyValue.get()._value);

            if(graph == nullptr)
                return false;

            uint64_t i = 0;
            for(const auto& element : *graph)
            {
                parser.setProgress(static_cast<int>((i++ * 100) / graph->size()));

                const auto& type = element.get()._key;
                const auto* value = std::get_if<List>(&element.get()._value);

                if(value == nullptr)
                    continue;

                bool success = true;

                if(type == QStringLiteral("node"))
                    success = processNode(*value);
                else if(type == QStringLiteral("edge"))
                    edges.push_back(value);

                if(!success || parser.cancelled())
                    return false;
            }
        }

        // It's possible to define edges before nodes, so we save
        // processing edges until all the nodes have been visited
        for(const auto* edge : edges)
        {
            if(!processEdge(*edge) || parser.cancelled())
                return false;
        }
    }

    return true;
}

} // namespace SpiritGmlParser

GmlFileParser::GmlFileParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool GmlFileParser::parse(const QUrl& url, IGraphModel* graphModel)
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

    if(!stream)
        return false;

    stream.unsetf(std::ios::skipws);

    boost::spirit::istream_iterator istreamIt(stream);
    using GmlIterator = progress_iterator<decltype(istreamIt)>;
    GmlIterator it(istreamIt);
    GmlIterator end;

    it.onPositionChanged(
    [this, &fileSize](size_t position)
    {
        setProgress(static_cast<int>((position * 100) / fileSize));
    });

    auto cancelledFn = [this] { return cancelled(); };
    it.setCancelledFn(cancelledFn);

    graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

    SpiritGmlParser::List gml;
    bool success = false;

    try
    {
        success = SpiritGmlParser::x3::phrase_parse(it, end,
            SpiritGmlParser::gmlList, SpiritGmlParser::ascii::space, gml);
    }
    catch(GmlIterator::cancelled_exception&) {}

    if(cancelled() || !success || it != end)
        return false;

    graphModel->mutableGraph().setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return SpiritGmlParser::build(*this, gml, *graphModel,
        *_userNodeData, *_userEdgeData);
}
