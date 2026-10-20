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

#include "shared/loading/graphsizeestimate.h"
#include "shared/graph/edgelist.h"

#include <QTest>
#include <QString>
#include <QVariantMap>
#include <QVector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// A cycle of 4 nodes, whose edges' weights are 0.9, 0.8, 0.5 and 0.1
EdgeList cycle()
{
    return
    {
        {._source = NodeId(2), ._target = NodeId(3), ._weight = 0.5},
        {._source = NodeId(0), ._target = NodeId(1), ._weight = 0.9},
        {._source = NodeId(0), ._target = NodeId(3), ._weight = -0.1},
        {._source = NodeId(1), ._target = NodeId(2), ._weight = 0.8},
    };
}

QVector<double> valuesOf(const QVariantMap& estimate, const QString& key)
{
    return estimate.value(key).value<QVector<double>>();
}
} // namespace

class TestGraphSizeEstimate : public QObject
{
    Q_OBJECT

private slots:
    // Each key is a threshold, with the size of the graph its edges would give
    void threshold()
    {
        auto estimate = graphSizeEstimateThreshold(cycle());

        QCOMPARE(valuesOf(estimate, u"keys"_s), (QVector<double>{0.1, 0.5, 0.8, 0.9}));
        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{4.0, 4.0, 3.0, 2.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{4.0, 3.0, 2.0, 1.0}));
        QCOMPARE(valuesOf(estimate, u"numUniqueEdges"_s), (QVector<double>{4.0, 3.0, 2.0, 1.0}));
    }

    // Sizes estimated from a sample are scaled up to the size of the whole data,
    // linearly for nodes, and quadratically for edges
    void thresholdScaled()
    {
        auto estimate = graphSizeEstimateThreshold(cycle(), 4, 8);

        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{8.0, 8.0, 6.0, 4.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{16.0, 12.0, 8.0, 4.0}));

        // But no more than the size of the whole data allows
        estimate = graphSizeEstimateThreshold(cycle(), 2, 3);
        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{3.0, 3.0, 3.0, 3.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{9.0, 7.0, 5.0, 3.0}));
    }

    void thresholdDuplicateEdges()
    {
        auto edgeList = cycle();
        edgeList.push_back({._source = NodeId(1), ._target = NodeId(0), ._weight = 0.95});

        auto estimate = graphSizeEstimateThreshold(edgeList);
        QCOMPARE(valuesOf(estimate, u"numEdges"_s).at(0), 5.0);
        QCOMPARE(valuesOf(estimate, u"numUniqueEdges"_s).at(0), 4.0);
    }

    // Each key is a value of k, with the size of the graph keeping each node's k best edges
    void knn()
    {
        auto estimate = graphSizeEstimateKnn(cycle(), 2, 4, 4);

        QCOMPARE(valuesOf(estimate, u"keys"_s), (QVector<double>{2.0, 1.0}));
        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{4.0, 4.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{4.0, 3.0}));
        QCOMPARE(valuesOf(estimate, u"numUniqueEdges"_s), (QVector<double>{4.0, 3.0}));
    }

    // By default, the sample is the whole data
    void knnUnscaled()
    {
        auto estimate = graphSizeEstimateKnn(cycle(), 2);

        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{4.0, 4.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{4.0, 3.0}));
    }

    // For k-NN, the number of edges grows linearly with the number of nodes
    void knnScaled()
    {
        auto estimate = graphSizeEstimateKnn(cycle(), 2, 4, 8);

        QCOMPARE(valuesOf(estimate, u"numNodes"_s), (QVector<double>{8.0, 8.0}));
        QCOMPARE(valuesOf(estimate, u"numEdges"_s), (QVector<double>{8.0, 6.0}));
    }

    // There are at most 100 samples, spread evenly
    void knnSamples()
    {
        auto estimate = graphSizeEstimateKnn(cycle(), 1000, 4, 4);
        auto keys = valuesOf(estimate, u"keys"_s);

        QCOMPARE(keys.size(), 100);
        QCOMPARE(keys.front(), 1000.0);
        QCOMPARE(keys.back(), 1.0);
    }

    void empty()
    {
        QVERIFY(graphSizeEstimateThreshold({}).isEmpty());
        QVERIFY(graphSizeEstimateKnn({}, 5).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestGraphSizeEstimate)
#include "tst_graphsizeestimate.moc"
