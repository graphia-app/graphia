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

#include "tests/common/testenvironment.h"
#include "tests/common/testgraphmodel.h"

#include "app/graph/elementiddistinctsetcollection.h"

#include <QTest>
#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace
{
// A triangle, a pair, and an isolated node; each edge has a distinct weight
std::unique_ptr<TestGraphModel> createComponentsGraph()
{
    auto model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"b-c"_s, u"c-a"_s, u"d-e"_s, u"f"_s});

    model->addEdgeAttribute<int>(u"Weight"_s,
    {
        {{u"a"_s, u"b"_s}, 1},
        {{u"b"_s, u"c"_s}, 2},
        {{u"c"_s, u"a"_s}, 3},
        {{u"d"_s, u"e"_s}, 4},
    });

    return model;
}

// A triangle with a three edge tail, and an isolated node
std::unique_ptr<TestGraphModel> createTailGraph()
{
    return std::make_unique<TestGraphModel>(QStringList{u"a-b"_s, u"b-c"_s, u"c-a"_s, u"c-d"_s, u"d-e"_s, u"e-g"_s, u"f"_s});
}
} // namespace

class TestStructuralTransforms : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void filters_data()
    {
        QTest::addColumn<QStringList>("transforms");
        QTest::addColumn<QStringList>("expectedNodes");
        QTest::addColumn<QStringList>("expectedEdges");

        const QStringList allNodes = {u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s};
        const QStringList allEdges = {u"a-b"_s, u"b-c"_s, u"c-a"_s, u"d-e"_s};

        QTest::newRow("no transforms") << QStringList{} << allNodes << allEdges;

        QTest::newRow("remove nodes")
            << QStringList{u"RemoveNodes where $NodeDegree < 1"_s}
            << QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s} << allEdges;

        QTest::newRow("keep nodes")
            << QStringList{u"KeepNodes where $NodeDegree == 2"_s}
            << QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s, u"b-c"_s, u"c-a"_s};

        QTest::newRow("remove edges")
            << QStringList{u"RemoveEdges where $Weight > 2"_s}
            << allNodes << QStringList{u"a-b"_s, u"b-c"_s};

        QTest::newRow("keep edges")
            << QStringList{u"KeepEdges where $Weight >= 3"_s}
            << allNodes << QStringList{u"c-a"_s, u"d-e"_s};

        QTest::newRow("remove components")
            << QStringList{u"[pinned] RemoveComponents where $ComponentSize <= 1"_s}
            << QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s} << allEdges;

        QTest::newRow("keep components")
            << QStringList{u"KeepComponents where $ComponentSize == 3"_s}
            << QStringList{u"a"_s, u"b"_s, u"c"_s} << QStringList{u"a-b"_s, u"b-c"_s, u"c-a"_s};

        // Removing b-c leaves only a with a degree of 2
        QTest::newRow("filters in sequence")
            << QStringList{u"RemoveEdges where $Weight == 2"_s,
                           u"RemoveNodes where $NodeDegree < 2"_s}
            << QStringList{u"a"_s} << QStringList{};
    }

    void filters()
    {
        QFETCH(const QStringList, transforms);
        QFETCH(const QStringList, expectedNodes);
        QFETCH(const QStringList, expectedEdges);

        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms(transforms), QStringList{});
        QCOMPARE(model->nodeNames(), expectedNodes);
        QCOMPARE(model->edgeNames(), expectedEdges);
    }

    void invalidConditionAlerts_data()
    {
        QTest::addColumn<QString>("transform");

        QTest::addColumn<QString>("alert");

        QTest::newRow("unknown attribute") << u"RemoveNodes where $Unknown == 1"_s << u"Unknown Attribute: \"Unknown\""_s;

        // A regex that isn't valid; not raw strings, which moc misparses here
        QTest::newRow("nodes") << u"RemoveNodes where $NodeDegree matches \"(\""_s << u"Invalid condition"_s; // NOLINT modernize-raw-string-literal
        QTest::newRow("edges") << u"RemoveEdges where $Weight matches \"(\""_s << u"Invalid condition"_s; // NOLINT modernize-raw-string-literal
        QTest::newRow("components") << u"RemoveComponents where $ComponentSize matches \"(\""_s << u"Invalid condition"_s; // NOLINT modernize-raw-string-literal
        QTest::newRow("keeping nodes") << u"KeepNodes where $NodeDegree matches \"(\""_s << u"Invalid condition"_s; // NOLINT modernize-raw-string-literal
    }

    // A filter whose condition can't be made removes nothing, and says why
    void invalidConditionAlerts()
    {
        QFETCH(const QString, transform);
        QFETCH(const QString, alert);

        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{alert});
        QCOMPARE(model->graph().numNodes(), 6);
        QCOMPARE(model->graph().numEdges(), 4);
    }

    void removeLeaves_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QStringList>("expectedNodes");

        QTest::newRow("one pass")
            << u"RemoveLeaves with Limit = 1"_s
            << QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s};

        QTest::newRow("two passes")
            << u"RemoveLeaves with Limit = 2"_s
            << QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s};

        QTest::newRow("branches")
            << u"RemoveBranches"_s
            << QStringList{u"a"_s, u"b"_s, u"c"_s};
    }

    void removeLeaves()
    {
        QFETCH(const QString, transform);
        QFETCH(const QStringList, expectedNodes);

        auto model = createTailGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});
        QCOMPARE(model->nodeNames(), expectedNodes);
    }

    void contractEdges()
    {
        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1"_s}), QStringList{});

        // a and b become a single merged node, which keeps the edges to c
        auto headNodeId = model->graph().typeOf(model->nodeId(u"a"_s)) == MultiElementType::Head ?
            model->nodeId(u"a"_s) : model->nodeId(u"b"_s);

        const auto mergedNodeIds = model->graph().mergedNodeIdsForNodeId(headNodeId);

        QStringList mergedNames;
        mergedNames.reserve(mergedNodeIds.size());
        for(auto nodeId : mergedNodeIds)
            mergedNames.append(model->graphModel().nodeName(nodeId));

        mergedNames.sort();
        QCOMPARE(mergedNames, (QStringList{u"a"_s, u"b"_s}));

        QVERIFY(!model->graph().containsEdgeId(model->edgeId(u"a"_s, u"b"_s)));
        QCOMPARE(model->graph().numEdges(), 3);
        QCOMPARE(model->graph().numComponents(), 3);
    }

    void removeLeavesOfMergedNodes_data()
    {
        QTest::addColumn<QString>("transform");

        QTest::newRow("leaves") << u"RemoveLeaves"_s;
        QTest::newRow("branches") << u"RemoveBranches"_s;
    }

    // The nodes merged into another have no edges of their own, but aren't leaves
    void removeLeavesOfMergedNodes()
    {
        QFETCH(const QString, transform);

        // a and b, and d and e, are merged; a and b are joined to c twice, but d and e
        // only to each other, so are a leaf as a whole
        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1 or $Weight == 4"_s, transform}),
            QStringList{});

        QCOMPARE(model->nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        auto headNodeId = model->graph().typeOf(model->nodeId(u"a"_s)) == MultiElementType::Head ?
            model->nodeId(u"a"_s) : model->nodeId(u"b"_s);
        QCOMPARE(model->graph().mergedNodeIdsForNodeId(headNodeId).size(), 2);
        QCOMPARE(model->graph().numComponents(), 1);
    }

    void filtersOfMergedNodes_data()
    {
        QTest::addColumn<QString>("transform");

        QTest::newRow("remove") << u"RemoveNodes where $NodeDegree < 1"_s;
        QTest::newRow("keep") << u"KeepNodes where $NodeDegree >= 1"_s;
    }

    // The nodes merged into another have no edges of their own, so a node filter judges
    // a merged node as a whole, by its head, and removes all of it or none of it
    void filtersOfMergedNodes()
    {
        QFETCH(const QString, transform);

        // a and b, and d and e, are merged; a and b are joined to c twice, but d and e
        // only to each other, so have no edges as a whole
        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1 or $Weight == 4"_s, transform}),
            QStringList{});

        QCOMPARE(model->nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        auto headNodeId = model->graph().typeOf(model->nodeId(u"a"_s)) == MultiElementType::Head ?
            model->nodeId(u"a"_s) : model->nodeId(u"b"_s);
        QCOMPARE(model->graph().mergedNodeIdsForNodeId(headNodeId).size(), 2);
    }

    // A condition on a node merged into another only matches it if it's the head
    void filterOnAMergedNodesValue()
    {
        auto model = createComponentsGraph();
        model->addNodeAttribute<QString>(u"Label"_s, {{u"a"_s, u"x"_s}, {u"b"_s, u"y"_s},
            {u"c"_s, u"y"_s}, {u"d"_s, u"y"_s}, {u"e"_s, u"y"_s}, {u"f"_s, u"y"_s}});
        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1"_s}), QStringList{});
        const bool aIsHead = model->graph().typeOf(model->nodeId(u"a"_s)) == MultiElementType::Head;

        QCOMPARE(model->applyTransforms({u"ContractEdges where $Weight == 1"_s,
            u"RemoveNodes where $Label == \"x\""_s}), QStringList{});
        QCOMPARE(model->nodeNames(), aIsHead ?
            (QStringList{u"c"_s, u"d"_s, u"e"_s, u"f"_s}) :
            (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s, u"f"_s}));
    }

    void spanningForest_data()
    {
        QTest::addColumn<QString>("transform");

        QTest::newRow("breadth first") << u"SpanningForest with TraversalOrder = BreadthFirst"_s;
        QTest::newRow("depth first") << u"SpanningForest with TraversalOrder = DepthFirst"_s;
    }

    void spanningForest()
    {
        QFETCH(const QString, transform);

        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});

        // Every node and component is kept, but the triangle loses an edge
        QCOMPARE(model->graph().numNodes(), 6);
        QCOMPARE(model->graph().numComponents(), 3);
        QCOMPARE(model->graph().numEdges(), 3);
        QVERIFY(model->edgeNames().contains(u"d-e"_s));
    }

    void booleanAttribute()
    {
        auto model = createComponentsGraph();
        QCOMPARE(model->applyTransforms(
            {uR"(BooleanNodeAttribute with Name = "High Degree" where $NodeDegree >= 2)"_s}),
            QStringList{});

        const std::map<QString, QString> expected =
        {
            {u"a"_s, u"True"_s}, {u"b"_s, u"True"_s}, {u"c"_s, u"True"_s},
            {u"d"_s, u"False"_s}, {u"e"_s, u"False"_s}, {u"f"_s, u"False"_s},
        };

        for(const auto& [node, value] : expected)
            QCOMPARE(model->nodeValue(u"High Degree"_s, node), value);
    }
};

QTEST_MAIN(TestStructuralTransforms)
#include "tst_structuraltransforms.moc"
