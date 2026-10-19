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

#include <QTest>

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
};

QTEST_GUILESS_MAIN(TestMutableGraph)
#include "tst_mutablegraph.moc"
