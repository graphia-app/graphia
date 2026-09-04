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

#include "gmlfileparser.h"

#include "shared/graph/elementid.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include "shared/utils/container.h"
#include "shared/utils/recursivevalue.h"

#include <lexy_utils.h>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QFileInfo>
#include <QTextDocumentFragment>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <map>
#include <vector>

using namespace Qt::Literals::StringLiterals;

// https://www.google.com/search?q=gml-technical-report.pdf

namespace LexyGmlParser
{
struct KeyValue;
using List = std::vector<RecursiveValue<KeyValue>>;

using Value = std::variant<double, int, QString, List>;
struct KeyValue
{
    QString _key;
    Value _value;
};

namespace grammar
{
namespace dsl = lexy::dsl;

using lexyutils::anyByte;
using lexyutils::asQString;

struct Key
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha_digit);
    static constexpr auto value = asQString;
};

// Note there is no escaping mechanism; the string simply ends at the next quote
struct QuotedString : lexy::token_production
{
    static constexpr auto rule = dsl::lit_c<'"'> >>
        (dsl::capture(dsl::token(dsl::while_(anyByte - dsl::lit_c<'"'>))) + dsl::lit_c<'"'>);

    static constexpr auto value = asQString;
};

struct Number
{
    static constexpr auto rule = dsl::capture(lexyutils::number);
    static constexpr auto value = lexyutils::asNumber<LexyGmlParser::Value>;
};

struct List;

struct Value
{
    static constexpr auto rule = dsl::p<Number> | dsl::p<QuotedString> |
        (dsl::lit_c<'['> >> (dsl::recurse<List> + dsl::lit_c<']'>));

    static constexpr auto value = lexy::construct<LexyGmlParser::Value>;
};

struct KeyValue
{
    static constexpr auto rule = dsl::p<Key> >> dsl::p<Value>;
    static constexpr auto value = lexy::construct<LexyGmlParser::KeyValue>;
};

struct List
{
    static constexpr auto rule = dsl::opt(dsl::list(dsl::p<KeyValue>));
    static constexpr auto value = lexy::as_list<LexyGmlParser::List>;
};

struct Gml
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = dsl::p<List> + dsl::eof;
    static constexpr auto value = lexy::forward<LexyGmlParser::List>;
};
} // namespace grammar

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
                    const QString subName = _name + "." + childAttribute._name;
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
        const auto* id = findIntValue(node, u"id"_s);
        if(id == nullptr)
            return false;

        auto nodeId = graphModel.mutableGraph().addNode();
        gmlIdToNodeId[*id] = nodeId;

        auto nodeName = QString::number(*id);

        for(const auto& attributeWrapper : node)
        {
            const auto& keyValue = attributeWrapper.get();
            if(keyValue._key == u"id"_s)
                continue;

            if(keyValue._key == u"label"_s)
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
                    const QString attributeName = QObject::tr("Node ") + attribute._name;
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
        const auto* sourceId = findIntValue(edge, u"source"_s);
        const auto* targetId = findIntValue(edge, u"target"_s);

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
            if(keyValue._key == u"source"_s || keyValue._key == u"target"_s)
                continue;

            auto attributes = processAttribute(keyValue);

            for(const auto& attribute : attributes)
            {
                const QString attributeName = QObject::tr("Edge ") + attribute._name;
                userEdgeData.setValueBy(edgeId, attributeName, attribute._value);
            }
        }

        return true;
    };

    std::vector<const List*> edges;

    for(const auto& keyValue : gml)
    {
        const auto& key = keyValue.get()._key;

        if(key == u"graph"_s)
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

                if(type == u"node"_s)
                    success = processNode(*value);
                else if(type == u"edge"_s)
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

        edges.clear();
    }

    return true;
}

} // namespace LexyGmlParser

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

    auto result = lexy::parse<LexyGmlParser::grammar::Gml>(input, lexy::noop);

    if(cancelled() || !result.is_success())
        return false;

    setPhase(QObject::tr("Building Graph"));
    setProgress(-1);

    return LexyGmlParser::build(*this, result.value(), *graphModel,
        *_userNodeData, *_userEdgeData);
}
