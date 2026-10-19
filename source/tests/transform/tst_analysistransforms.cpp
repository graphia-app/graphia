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

#include <QTest>
#include <QString>
#include <QStringList>

#include <cmath>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
// A path, a star, and an isolated node
std::unique_ptr<TestGraphModel> createShapesGraph()
{
    return std::make_unique<TestGraphModel>(QStringList
    {
        u"a-b"_s, u"b-c"_s, u"c-d"_s,
        u"x-p"_s, u"x-q"_s, u"x-r"_s,
        u"z"_s
    });
}

// Two cliques of four nodes, joined by a single edge
const QStringList twoCliquesEdges =
{
    u"a1-a2"_s, u"a1-a3"_s, u"a1-a4"_s, u"a2-a3"_s, u"a2-a4"_s, u"a3-a4"_s,
    u"b1-b2"_s, u"b1-b3"_s, u"b1-b4"_s, u"b2-b3"_s, u"b2-b4"_s, u"b3-b4"_s,
    u"a1-b1"_s
};

std::unique_ptr<TestGraphModel> createTwoCliquesGraph()
{
    return std::make_unique<TestGraphModel>(twoCliquesEdges);
}

const QStringList twoCliquesPartition = {u"a1 a2 a3 a4"_s, u"b1 b2 b3 b4"_s};
} // namespace

class TestAnalysisTransforms : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void pageRank_data()
    {
        QTest::addColumn<QString>("node");
        QTest::addColumn<double>("expected");

        // Independently computed; the values in each component are scaled so that the maximum is 1
        QTest::newRow("path end")       << u"a"_s << 0.555556;
        QTest::newRow("path middle")    << u"b"_s << 1.0;
        QTest::newRow("path middle 2")  << u"c"_s << 1.0;
        QTest::newRow("path end 2")     << u"d"_s << 0.555556;
        QTest::newRow("star centre")    << u"x"_s << 1.0;
        QTest::newRow("star leaf")      << u"p"_s << 0.372549;
        QTest::newRow("isolated")       << u"z"_s << 1.0;
    }

    void pageRank()
    {
        QFETCH(const QString, node);
        QFETCH(const double, expected);

        auto model = createShapesGraph();
        QCOMPARE(model->applyTransforms({u"PageRank"_s}), QStringList{});

        auto actual = model->nodeNumericValue(u"NodePageRank"_s, node);
        QVERIFY2(std::abs(actual - expected) < 1e-4,
            qPrintable(u"%1 != %2"_s.arg(actual).arg(expected)));
    }

    void eccentricity_data()
    {
        QTest::addColumn<QString>("node");
        QTest::addColumn<int>("expected");

        QTest::newRow("path end")       << u"a"_s << 3;
        QTest::newRow("path middle")    << u"b"_s << 2;
        QTest::newRow("star centre")    << u"x"_s << 1;
        QTest::newRow("star leaf")      << u"p"_s << 2;
        QTest::newRow("isolated")       << u"z"_s << 0;
    }

    void eccentricity()
    {
        QFETCH(const QString, node);
        QFETCH(const int, expected);

        auto model = createShapesGraph();
        QCOMPARE(model->applyTransforms({u"Eccentricity"_s}), QStringList{});
        QCOMPARE(model->nodeValue(u"NodeEccentricity"_s, node), QString::number(expected));
    }

    void nodeBetweenness_data()
    {
        QTest::addColumn<QString>("node");
        QTest::addColumn<double>("expected");

        QTest::newRow("path end")       << u"a"_s << 0.0;
        QTest::newRow("path middle")    << u"b"_s << 2.0;
        QTest::newRow("path middle 2")  << u"c"_s << 2.0;
        QTest::newRow("star centre")    << u"x"_s << 3.0;
        QTest::newRow("star leaf")      << u"p"_s << 0.0;
        QTest::newRow("isolated")       << u"z"_s << 0.0;
    }

    void nodeBetweenness()
    {
        QFETCH(const QString, node);
        QFETCH(const double, expected);

        auto model = createShapesGraph();
        QCOMPARE(model->applyTransforms({u"Betweenness"_s}), QStringList{});
        QCOMPARE(model->nodeNumericValue(u"NodeBetweenness"_s, node), expected);
    }

    void edgeBetweenness_data()
    {
        QTest::addColumn<QString>("from");
        QTest::addColumn<QString>("to");
        QTest::addColumn<double>("expected");

        QTest::newRow("path end")       << u"a"_s << u"b"_s << 3.0;
        QTest::newRow("path middle")    << u"b"_s << u"c"_s << 4.0;
        QTest::newRow("star")           << u"x"_s << u"p"_s << 3.0;
    }

    void edgeBetweenness()
    {
        QFETCH(const QString, from);
        QFETCH(const QString, to);
        QFETCH(const double, expected);

        auto model = createShapesGraph();
        QCOMPARE(model->applyTransforms({u"Betweenness"_s}), QStringList{});
        QCOMPARE(model->edgeNumericValue(u"EdgeBetweenness"_s, from, to), expected);
    }

    // The a-b and b-a edges are distinct routes between a and b, so they share
    // the shortest paths that pass between them
    void reciprocalEdgeBetweenness()
    {
        TestGraphModel model({u"d-a"_s, u"a-b"_s, u"b-a"_s, u"b-c"_s});
        QCOMPARE(model.applyTransforms({u"Betweenness"_s}), QStringList{});

        QCOMPARE(model.nodeNumericValue(u"NodeBetweenness"_s, u"d"_s), 0.0);
        QCOMPARE(model.nodeNumericValue(u"NodeBetweenness"_s, u"a"_s), 2.0);
        QCOMPARE(model.nodeNumericValue(u"NodeBetweenness"_s, u"b"_s), 2.0);
        QCOMPARE(model.nodeNumericValue(u"NodeBetweenness"_s, u"c"_s), 0.0);

        QCOMPARE(model.edgeNumericValue(u"EdgeBetweenness"_s, u"d"_s, u"a"_s), 3.0);
        QCOMPARE(model.edgeNumericValue(u"EdgeBetweenness"_s, u"a"_s, u"b"_s), 2.0);
        QCOMPARE(model.edgeNumericValue(u"EdgeBetweenness"_s, u"b"_s, u"a"_s), 2.0);
        QCOMPARE(model.edgeNumericValue(u"EdgeBetweenness"_s, u"b"_s, u"c"_s), 3.0);
    }

    void clustering_data()
    {
        QTest::addColumn<QString>("transform");
        QTest::addColumn<QString>("clusterAttribute");
        QTest::addColumn<QString>("sizeAttribute");

        QTest::newRow("louvain") << u"LouvainCluster"_s
            << u"LouvainCluster"_s << u"LouvainClusterSize"_s;
        QTest::newRow("mcl") << u"MCLCluster"_s
            << u"MCLCluster"_s << u"MCLClusterSize"_s;
    }

    void clustering()
    {
        QFETCH(const QString, transform);
        QFETCH(const QString, clusterAttribute);
        QFETCH(const QString, sizeAttribute);

        auto model = createTwoCliquesGraph();
        QCOMPARE(model->applyTransforms({transform}), QStringList{});
        QCOMPARE(model->nodePartition(clusterAttribute), twoCliquesPartition);

        for(auto nodeId : model->graph().nodeIds())
        {
            auto name = model->graphModel().nodeName(nodeId);
            QCOMPARE(model->nodeValue(sizeAttribute, name), u"4"_s);
        }
    }

    void weightedLouvain()
    {
        auto model = createTwoCliquesGraph();

        // Weight the edges so that the strongest grouping no longer follows the cliques
        std::map<std::pair<QString, QString>, double> weights;
        for(const auto& edgeName : twoCliquesEdges)
        {
            auto nodes = edgeName.split(u'-');
            weights[{nodes.at(0), nodes.at(1)}] = 1.0;
        }

        weights[{u"a1"_s, u"b1"_s}] = 100.0;
        model->addEdgeAttribute(u"Weight"_s, weights);

        QCOMPARE(model->applyTransforms({u"WeightedLouvainCluster using $Weight"_s}), QStringList{});

        auto partition = model->nodePartition(u"WeightedLouvainCluster"_s);
        QVERIFY2(partition.contains(u"a1 b1"_s), qPrintable(partition.join(u", "_s)));
    }
};

QTEST_MAIN(TestAnalysisTransforms)
#include "tst_analysistransforms.moc"
