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

#ifndef TESTGRAPHMODEL_H
#define TESTGRAPHMODEL_H

#include "tests/common/testplugin.h"

#include "app/application.h"
#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"
#include "app/transform/transforminfo.h"

#include "shared/graph/elementid.h"
#include "shared/loading/iparser.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

// A GraphModel built from a compact description of a graph, for testing transforms,
// or parsed from a file, for testing parsers
class TestGraphModel
{
private:
    TestPlugin _plugin;
    GraphModel _graphModel{application(), &_plugin};

    // Declared after the GraphModel, as parsers may refer to its graph
    std::unique_ptr<IParser> _parser;

    std::map<QString, NodeId> _nodeIds;
    std::map<std::pair<QString, QString>, EdgeId> _edgeIds;

    TestGraphModel() = default;

    void indexElementsByName()
    {
        const auto& mutableGraph = _graphModel.mutableGraph();

        for(auto nodeId : mutableGraph.nodeIds())
            _nodeIds.emplace(_graphModel.nodeName(nodeId), nodeId);

        for(auto edgeId : mutableGraph.edgeIds())
        {
            const auto& edge = mutableGraph.edgeById(edgeId);
            _edgeIds.emplace(std::make_pair(_graphModel.nodeName(edge.sourceId()),
                _graphModel.nodeName(edge.targetId())), edgeId);
        }
    }

public:
    // The Application that provides GraphModels with their transforms, visualisation channels
    // and attribute declarations; one for the whole test, created when it's first needed, once
    // the TestEnvironment is initialised. Creating it queries OpenGL, so the test must have a
    // QGuiApplication, i.e. use QTEST_MAIN, not QTEST_GUILESS_MAIN
    static Application* application()
    {
        // Never destroyed, as that would be after the QGuiApplication
        static auto* application = new Application(nullptr, false); // NOLINT cppcoreguidelines-owning-memory
        return application;
    }

    // Each element is either a node name, or two node names joined by '-', in which
    // case an edge is created from the first to the second; nodes are created as needed
    explicit TestGraphModel(const QStringList& elements)
    {
        using namespace Qt::Literals::StringLiterals;

        _graphModel.mutableGraph().performTransaction([this, &elements](IMutableGraph& graph)
        {
            auto addNode = [this, &graph](const QString& name)
            {
                if(!_nodeIds.contains(name))
                {
                    auto nodeId = graph.addNode();
                    _nodeIds.emplace(name, nodeId);
                    _graphModel.setNodeName(nodeId, name);
                }

                return _nodeIds.at(name);
            };

            for(const auto& element : elements)
            {
                auto names = element.split(u'-');

                if(names.size() == 1)
                    addNode(names.at(0));
                else
                {
                    auto edgeId = graph.addEdge(addNode(names.at(0)), addNode(names.at(1)));
                    _edgeIds.emplace(std::make_pair(names.at(0), names.at(1)), edgeId);
                }
            }
        });

        _graphModel.initialiseAttributeRanges();
    }

    // Parses content, written to a file, in the same way as ParserThread and Document; createParser
    // is passed the model's user node and edge data, and returns the parser to use. If parsing
    // fails, nullptr is returned and failureReason is set
    template<typename CreateParserFn>
    static std::unique_ptr<TestGraphModel> parse(const QByteArray& content,
        const CreateParserFn& createParser, QString* failureReason = nullptr)
    {
        using namespace Qt::Literals::StringLiterals;

        const QTemporaryDir dir;
        auto fileName = dir.filePath(u"graph"_s);

        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            return nullptr;

        file.close();

        // Not std::make_unique, as the constructor is private
        std::unique_ptr<TestGraphModel> model(new TestGraphModel);
        auto& graphModel = model->_graphModel;
        auto parser = createParser(&graphModel.userNodeData(), &graphModel.userEdgeData());

        bool success = false;
        graphModel.mutableGraph().performTransaction([&](IMutableGraph& graph)
        {
            success = parser->parse(QUrl::fromLocalFile(fileName), &graphModel);

            if(!success)
                graph.clear();
        });

        if(!success)
        {
            if(failureReason != nullptr)
                *failureReason = parser->failureReason();

            return nullptr;
        }

        model->_parser = std::move(parser);

        graphModel.userNodeData().exposeAsAttributes(graphModel);
        graphModel.userEdgeData().exposeAsAttributes(graphModel);
        graphModel.initialiseAttributeRanges();

        model->indexElementsByName();
        model->applyTransforms({});

        return model;
    }

    GraphModel& graphModel() { return _graphModel; }

    // The parser that parse used, for querying anything else it loaded
    template<typename Parser>
    const Parser* parser() const { return dynamic_cast<const Parser*>(_parser.get()); }

    const Graph& graph() const { return _graphModel.graph(); }

    NodeId nodeId(const QString& name) const { return _nodeIds.at(name); }
    EdgeId edgeId(const QString& from, const QString& to) const { return _edgeIds.at({from, to}); }

    template<typename T>
    void addNodeAttribute(const QString& name, const std::map<QString, T>& values)
    {
        std::map<NodeId, T> valuesById;
        for(const auto& [nodeName, value] : values)
            valuesById.emplace(nodeId(nodeName), value);

        auto valueFn = [valuesById](NodeId nodeId) { return valuesById.at(nodeId); };
        auto& attribute = _graphModel.createAttribute(name);

        if constexpr(std::is_same_v<T, int>)
            attribute.setIntValueFn(valueFn);
        else if constexpr(std::is_same_v<T, double>)
            attribute.setFloatValueFn(valueFn);
        else
            attribute.setStringValueFn(valueFn);

        _graphModel.calculateAttributeRange(attribute);
    }

    template<typename T>
    void addEdgeAttribute(const QString& name, const std::map<std::pair<QString, QString>, T>& values)
    {
        std::map<EdgeId, T> valuesById;
        for(const auto& [nodeNames, value] : values)
            valuesById.emplace(edgeId(nodeNames.first, nodeNames.second), value);

        auto valueFn = [valuesById](EdgeId edgeId) { return valuesById.at(edgeId); };
        auto& attribute = _graphModel.createAttribute(name);

        if constexpr(std::is_same_v<T, int>)
            attribute.setIntValueFn(valueFn);
        else if constexpr(std::is_same_v<T, double>)
            attribute.setFloatValueFn(valueFn);
        else
            attribute.setStringValueFn(valueFn);

        _graphModel.calculateAttributeRange(attribute);
    }

    // Applies transforms in the same way as Document, i.e. with any missing parameters set to
    // their defaults, then returns the alerts generated, if any
    QStringList applyTransforms(const QStringList& transforms)
    {
        _graphModel.buildTransforms(_graphModel.transformsWithMissingParametersSetToDefault(transforms), nullptr);

        QStringList alerts;
        if(!_graphModel.hasTransformInfo())
            return alerts;

        for(int index = 0; index < transforms.size(); index++)
        {
            for(const auto& alert : _graphModel.transformInfoAtIndex(index).alerts())
                alerts.append(alert._text);
        }

        return alerts;
    }

    // The names of the nodes in the transformed graph, sorted
    QStringList nodeNames() const
    {
        QStringList names;
        names.reserve(static_cast<qsizetype>(graph().nodeIds().size()));
        for(auto nodeId : graph().nodeIds())
            names.append(_graphModel.nodeName(nodeId));

        std::ranges::sort(names);
        return names;
    }

    // The edges in the transformed graph as "from-to" node names, sorted
    QStringList edgeNames() const
    {
        using namespace Qt::Literals::StringLiterals;

        QStringList names;
        for(auto edgeId : graph().edgeIds())
        {
            const auto& edge = graph().edgeById(edgeId);
            names.append(u"%1-%2"_s.arg(_graphModel.nodeName(edge.sourceId()),
                _graphModel.nodeName(edge.targetId())));
        }

        std::ranges::sort(names);
        return names;
    }

    QString nodeValue(const QString& attributeName, const QString& nodeName) const
    {
        auto attribute = _graphModel.attributeValueByName(attributeName);
        return attribute.stringValueOf(nodeId(nodeName));
    }

    QString edgeValue(const QString& attributeName, const QString& from, const QString& to) const
    {
        auto attribute = _graphModel.attributeValueByName(attributeName);
        return attribute.stringValueOf(edgeId(from, to));
    }

    bool nodeHasValue(const QString& attributeName, const QString& nodeName) const
    {
        auto attribute = _graphModel.attributeValueByName(attributeName);
        return !attribute.valueMissingOf(nodeId(nodeName));
    }

    double nodeNumericValue(const QString& attributeName, const QString& nodeName) const
    {
        auto attribute = _graphModel.attributeValueByName(attributeName);
        return attribute.numericValueOf(nodeId(nodeName));
    }

    double edgeNumericValue(const QString& attributeName, const QString& from, const QString& to) const
    {
        auto attribute = _graphModel.attributeValueByName(attributeName);
        return attribute.numericValueOf(edgeId(from, to));
    }

    // The nodes grouped by their value of the given attribute, ignoring what the values
    // actually are, e.g. for comparing clusterings where the cluster names are arbitrary
    QStringList nodePartition(const QString& attributeName) const
    {
        using namespace Qt::Literals::StringLiterals;

        auto attribute = _graphModel.attributeValueByName(attributeName);

        std::map<QString, QStringList> groups;
        for(auto nodeId : graph().nodeIds())
            groups[attribute.stringValueOf(nodeId)].append(_graphModel.nodeName(nodeId));

        QStringList partition;
        for(auto& [value, names] : groups)
        {
            names.sort();
            partition.append(names.join(u' '));
        }

        partition.sort();
        return partition;
    }
};

#endif // TESTGRAPHMODEL_H
