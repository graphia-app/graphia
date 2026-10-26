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

#include "app/graph/mutablegraph.h"
#include "app/graph/graphconsistencychecker.h"

#include "shared/graph/igraphcomponent.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QTest>

#include <algorithm>
#include <map>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// The graph of nodes a to f, with ids 0 to 5, and the given edges, e.g. "a-b"
struct NamedGraph
{
    MutableGraph _graph;
    std::map<QString, NodeId> _nodeIds;
    std::map<QString, EdgeId> _edgeIds;

    explicit NamedGraph(const QStringList& edges)
    {
        for(const auto* name : {"a", "b", "c", "d", "e", "f"})
            _nodeIds.emplace(QString::fromLatin1(name), _graph.addNode());

        for(const auto& edge : edges)
        {
            const auto names = edge.split(u'-');
            _edgeIds.emplace(edge, _graph.addEdge(_nodeIds.at(names.at(0)), _nodeIds.at(names.at(1))));
        }
    }

    QString nameOf(NodeId nodeId) const
    {
        auto it = std::ranges::find_if(_nodeIds, [nodeId](const auto& pair) { return pair.second == nodeId; });
        return it != _nodeIds.end() ? it->first : QString();
    }

    // Each set of merged nodes, as the sorted names of its nodes, separated by spaces
    QStringList mergedSets() const
    {
        QStringList sets;
        sets.reserve(static_cast<qsizetype>(_graph.nodeIds().size()));

        QStringList names;

        for(auto nodeId : _graph.nodeIds())
        {
            if(_graph.typeOf(nodeId) == MultiElementType::Tail)
                continue;

            const auto mergedNodeIds = _graph.mergedNodeIdsForNodeId(nodeId);

            names.clear();
            names.reserve(mergedNodeIds.size());
            for(auto mergedNodeId : mergedNodeIds)
                names.append(nameOf(mergedNodeId));

            names.sort();
            sets.append(names.join(u' '));
        }

        sets.sort();
        return sets;
    }
};
} // namespace

class TestMutableGraph : public QObject
{
    Q_OBJECT

private slots:
    void addNodesAndEdges()
    {
        MutableGraph graph;

        auto a = graph.addNode();
        auto b = graph.addNode();
        auto c = graph.addNode();

        auto ab = graph.addEdge(a, b);
        graph.addEdge(b, c);

        QCOMPARE(graph.numNodes(), 3);
        QCOMPARE(graph.numEdges(), 2);
        QVERIFY(graph.edgeExistsBetween(a, b));
        QVERIFY(!graph.edgeExistsBetween(a, c));
        QCOMPARE(graph.edgeById(ab).sourceId(), a);
        QCOMPARE(graph.edgeById(ab).targetId(), b);
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));
    }

    void removeNodeRemovesItsEdges()
    {
        MutableGraph graph;

        auto a = graph.addNode();
        auto b = graph.addNode();
        auto c = graph.addNode();

        graph.addEdge(a, b);
        graph.addEdge(b, c);
        auto ca = graph.addEdge(c, a);

        graph.removeNode(b);

        QCOMPARE(graph.numNodes(), 2);
        QCOMPARE(graph.numEdges(), 1);
        QVERIFY(!graph.containsNodeId(b));
        QVERIFY(graph.containsEdgeId(ca));
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));
    }

    void componentsSplitAndMerge()
    {
        MutableGraph graph;
        graph.enableComponentManagement();

        auto a = graph.addNode();
        auto b = graph.addNode();
        auto c = graph.addNode();
        auto d = graph.addNode();

        graph.addEdge(a, b);
        graph.addEdge(c, d);

        QCOMPARE(graph.numComponents(), 2);
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));

        auto bc = graph.addEdge(b, c);

        QCOMPARE(graph.numComponents(), 1);
        QCOMPARE(graph.componentIdOfNode(a), graph.componentIdOfNode(d));
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));

        graph.removeEdge(bc);

        QCOMPARE(graph.numComponents(), 2);
        QVERIFY(graph.componentIdOfNode(a) != graph.componentIdOfNode(d));
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));
    }

    // Nodes and edges that move between components that exist both before and after a change
    void nodesMoveBetweenComponents()
    {
        MutableGraph graph;
        graph.enableComponentManagement();

        std::vector<NodeId> n;
        n.reserve(6);
        for(int i = 0; i < 6; i++)
            n.push_back(graph.addNode());

        auto e02 = graph.addEdge(n[0], n[2]);
        auto e24 = graph.addEdge(n[2], n[4]);
        auto e13 = graph.addEdge(n[1], n[3]);

        // A: 0-2-4, B: 1-3, C: 5
        QCOMPARE(graph.numComponents(), 3);
        const std::map<ComponentId, QString> names
        {
            {graph.componentIdOfNode(n[0]), u"A"_s},
            {graph.componentIdOfNode(n[1]), u"B"_s},
            {graph.componentIdOfNode(n[5]), u"C"_s}
        };

        QStringList events;
        auto record = [&](const QString& event, int id, ComponentId from, ComponentId to = {})
        {
            events.append(u"%1 %2 %3%4"_s.arg(event).arg(id).arg(names.at(from),
                to.isNull() ? QString() : u"->"_s + names.at(to)));
        };

        connect(&graph, &Graph::nodeMovedBetweenComponents, this,
            [&](const Graph*, NodeId nodeId, ComponentId from, ComponentId to) { record(u"node moved"_s, static_cast<int>(nodeId), from, to); });
        connect(&graph, &Graph::edgeMovedBetweenComponents, this,
            [&](const Graph*, EdgeId edgeId, ComponentId from, ComponentId to) { record(u"edge moved"_s, static_cast<int>(edgeId), from, to); });
        connect(&graph, &Graph::nodeAddedToComponent, this,
            [&](const Graph*, NodeId nodeId, ComponentId componentId) { record(u"node added"_s, static_cast<int>(nodeId), componentId); });
        connect(&graph, &Graph::edgeAddedToComponent, this,
            [&](const Graph*, EdgeId edgeId, ComponentId componentId) { record(u"edge added"_s, static_cast<int>(edgeId), componentId); });
        connect(&graph, &Graph::nodeRemovedFromComponent, this,
            [&](const Graph*, NodeId nodeId, ComponentId componentId) { record(u"node removed"_s, static_cast<int>(nodeId), componentId); });
        connect(&graph, &Graph::edgeRemovedFromComponent, this,
            [&](const Graph*, EdgeId edgeId, ComponentId componentId) { record(u"edge removed"_s, static_cast<int>(edgeId), componentId); });
        connect(&graph, &Graph::componentAdded, this, [&](const Graph*, ComponentId, bool) { events.append(u"component added"_s); });
        connect(&graph, &Graph::componentWillBeRemoved, this, [&](const Graph*, ComponentId, bool) { events.append(u"component removed"_s); });
        connect(&graph, &Graph::componentsWillMerge, this, [&](const Graph*, const ComponentMergeSet&) { events.append(u"merge"_s); });
        connect(&graph, &Graph::componentSplit, this, [&](const Graph*, const ComponentSplitSet&) { events.append(u"split"_s); });

        // Within one change, 2 and 4 leave A for B; the edge is added first so the removed one's id isn't reused
        EdgeId e23;
        graph.performTransaction([&](IMutableGraph& mutableGraph)
        {
            e23 = mutableGraph.addEdge(n[2], n[3]);
            mutableGraph.removeEdge(e02);
        });

        QVERIFY(GraphConsistencyChecker::isConsistent(graph));
        QCOMPARE(graph.numComponents(), 3);
        QCOMPARE(names.at(graph.componentIdOfNode(n[0])), u"A"_s);
        QCOMPARE(names.at(graph.componentIdOfNode(n[2])), u"B"_s);
        QCOMPARE(names.at(graph.componentIdOfNode(n[4])), u"B"_s);
        QCOMPARE(names.at(graph.componentIdOfEdge(e24)), u"B"_s);
        QCOMPARE(names.at(graph.componentIdOfEdge(e13)), u"B"_s);
        QCOMPARE(graph.componentById(graph.componentIdOfNode(n[1]))->numNodes(), 4);

        // The removed edge no longer has a component
        QVERIFY(graph.componentIdOfEdge(e02).isNull());

        events.sort();
        QCOMPARE(events, (QStringList
        {
            u"edge added %1 B"_s.arg(static_cast<int>(e23)),
            u"edge moved %1 A->B"_s.arg(static_cast<int>(e24)),
            u"edge removed %1 A"_s.arg(static_cast<int>(e02)),
            u"node moved %1 A->B"_s.arg(static_cast<int>(n[2])),
            u"node moved %1 A->B"_s.arg(static_cast<int>(n[4]))
        }));
    }

    // Adding to a set through any of its members, rather than its head, as nothing does
    // at the moment, as the head is always the set's lowest id and is what's passed
    void addToSetThroughAnyMember()
    {
        NodeIdDistinctSetCollection collection;
        collection.resize(10);

        auto members = [&collection]
        {
            const NodeIdDistinctSet set(NodeId(1), &collection);

            QStringList names;
            names.reserve(set.size());
            for(auto nodeId : set)
                names.append(QString::number(static_cast<int>(nodeId)));

            return names;
        };

        QCOMPARE(collection.add(NodeId(1), NodeId(3)), NodeId(1));

        // Through the tail
        collection.add(NodeId(3), NodeId(5));
        QCOMPARE(members(), (QStringList{u"1"_s, u"3"_s, u"5"_s}));
        QCOMPARE(collection.typeOf(NodeId(1)), MultiElementType::Head);
        QCOMPARE(collection.typeOf(NodeId(3)), MultiElementType::Tail);
        QCOMPARE(collection.typeOf(NodeId(5)), MultiElementType::Tail);

        // Through a member higher than the one added
        collection.add(NodeId(5), NodeId(2));
        QCOMPARE(members(), (QStringList{u"1"_s, u"3"_s, u"2"_s, u"5"_s}));
        QCOMPARE(collection.typeOf(NodeId(2)), MultiElementType::Tail);

        QCOMPARE(collection.remove(NodeId(1), NodeId(2)), NodeId(1));
        QCOMPARE(members(), (QStringList{u"1"_s, u"3"_s, u"5"_s}));
        QCOMPARE(collection.typeOf(NodeId(2)), MultiElementType::Not);
    }

    void contractEdgeMergesNodes()
    {
        MutableGraph graph;

        auto a = graph.addNode();
        auto b = graph.addNode();
        auto c = graph.addNode();

        auto ab = graph.addEdge(a, b);
        graph.addEdge(b, c);

        graph.contractEdge(ab);

        // The nodes are merged rather than removed, so the
        // lower ID becomes the head of a node with multiplicity 2
        QCOMPARE(graph.multiplicityOf(a), 2);
        QCOMPARE(graph.mergedNodeIdsForNodeId(a).size(), 2);
        QCOMPARE(graph.numEdges(), 1);
        QVERIFY(!graph.containsEdgeId(ab));
        QVERIFY(graph.edgeExistsBetween(a, c));
        QVERIFY(GraphConsistencyChecker::isConsistent(graph));
    }

    void contractMergedNodes_data()
    {
        QTest::addColumn<QStringList>("edges");
        QTest::addColumn<QList<QStringList>>("contractions");
        QTest::addColumn<QStringList>("expectedSets");

        QTest::newRow("adjacent sets")
            << QStringList{u"a-b"_s, u"c-d"_s, u"b-c"_s}
            << QList<QStringList>{{u"a-b"_s, u"c-d"_s}, {u"b-c"_s}}
            << QStringList{u"a b c d"_s, u"e"_s, u"f"_s};

        QTest::newRow("interleaved sets")
            << QStringList{u"a-c"_s, u"b-d"_s, u"c-d"_s}
            << QList<QStringList>{{u"a-c"_s, u"b-d"_s}, {u"c-d"_s}}
            << QStringList{u"a b c d"_s, u"e"_s, u"f"_s};

        QTest::newRow("higher set made first")
            << QStringList{u"e-f"_s, u"a-b"_s, u"f-b"_s}
            << QList<QStringList>{{u"e-f"_s}, {u"a-b"_s}, {u"f-b"_s}}
            << QStringList{u"a b e f"_s, u"c"_s, u"d"_s};

        QTest::newRow("three sets at once")
            << QStringList{u"a-d"_s, u"b-e"_s, u"c-f"_s, u"d-e"_s, u"e-f"_s}
            << QList<QStringList>{{u"a-d"_s, u"b-e"_s, u"c-f"_s}, {u"d-e"_s, u"e-f"_s}}
            << QStringList{u"a b c d e f"_s};

        QTest::newRow("single nodes either side of a set")
            << QStringList{u"b-c"_s, u"a-b"_s, u"c-d"_s}
            << QList<QStringList>{{u"b-c"_s}, {u"a-b"_s, u"c-d"_s}}
            << QStringList{u"a b c d"_s, u"e"_s, u"f"_s};

        QTest::newRow("one edge at a time")
            << QStringList{u"a-b"_s, u"c-d"_s, u"e-f"_s, u"b-c"_s, u"d-e"_s}
            << QList<QStringList>{{u"a-b"_s}, {u"c-d"_s}, {u"b-c"_s}, {u"e-f"_s}, {u"d-e"_s}}
            << QStringList{u"a b c d e f"_s};

        QTest::newRow("two sets, one left")
            << QStringList{u"a-f"_s, u"c-d"_s, u"d-f"_s, u"b-e"_s, u"e-c"_s}
            << QList<QStringList>{{u"a-f"_s, u"c-d"_s, u"b-e"_s}, {u"d-f"_s}}
            << QStringList{u"a c d f"_s, u"b e"_s};
    }

    // Contracting edges between nodes that are already merged with others merges the sets
    void contractMergedNodes()
    {
        QFETCH(const QStringList, edges);
        QFETCH(const QList<QStringList>, contractions);
        QFETCH(const QStringList, expectedSets);

        NamedGraph namedGraph(edges);
        auto& graph = namedGraph._graph;

        for(const auto& contraction : contractions)
        {
            EdgeIdSet edgeIds;
            for(const auto& edge : contraction)
                edgeIds.insert(namedGraph._edgeIds.at(edge));

            graph.contractEdges(edgeIds);
            QVERIFY(GraphConsistencyChecker::isConsistent(graph));
        }

        QCOMPARE(namedGraph.mergedSets(), expectedSets);

        // The edges not contracted end up between the heads of the sets
        for(auto edgeId : graph.edgeIds())
        {
            const auto& edge = graph.edgeById(edgeId);
            QVERIFY(graph.typeOf(edge.sourceId()) != MultiElementType::Tail);
            QVERIFY(graph.typeOf(edge.targetId()) != MultiElementType::Tail);
        }

        // Removing the nodes one at a time, from the start, middle and end of each set
        auto remainingSets = expectedSets;
        for(const auto* name : {"a", "d", "f", "b", "c", "e"})
        {
            graph.removeNode(namedGraph._nodeIds.at(QString::fromLatin1(name)));
            QVERIFY(GraphConsistencyChecker::isConsistent(graph));

            for(auto& set : remainingSets)
            {
                auto names = set.split(u' ');
                names.removeAll(QString::fromLatin1(name));
                set = names.join(u' ');
            }

            remainingSets.removeAll(QString());
            remainingSets.sort();
            QCOMPARE(namedGraph.mergedSets(), remainingSets);
        }
    }
};

QTEST_GUILESS_MAIN(TestMutableGraph)
#include "tst_mutablegraph.moc"
